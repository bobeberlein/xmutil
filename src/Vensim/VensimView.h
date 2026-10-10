#ifndef _XMUTIL_VENSIM_VENSIMVIEW_H
#define _XMUTIL_VENSIM_VENSIMVIEW_H
#include <string>

#include "../Model.h"
#include "../Symbol/Parse.h"
#include "../Symbol/Symbol.h"
#include "VensimLex.h"
class VensimParse;
class VensimView;
class Variable;

// Sketch record tpos <-> XMILE label_side. tpos: 0 inside, 1 below, 2 left,
// 3 above, 4 right. Returns nullptr for a tpos with no label_side.
inline const char *TextPosToLabelSide(int tpos) {
  static const char *const kSides[] = {"center", "bottom", "left", "top", "right"};
  if (tpos < 0 || tpos > 4)
    return nullptr;
  return kSides[tpos];
}
// The size Vensim gives a name it lays out itself (a new variable, a flow's
// name, a shadow variable), as sketch half-extents. A ghost is drawn as
// "<name>". See VensimView.cpp for how the rule was fitted.
void VensimDefaultNameSize(const std::string &name, bool ghost, int &halfWidth, int &halfHeight);

// def is returned when side is null or not a recognized label_side.
inline int LabelSideToTextPos(const char *side, int def) {
  if (!side)
    return def;
  std::string s(side);
  for (int tpos = 0; tpos <= 4; tpos++) {
    if (s == TextPosToLabelSide(tpos))
      return tpos;
  }
  return def;
}

class VensimViewElement {
public:
  enum ElementType { ElementTypeVARIABLE, ElementTypeVALVE, ElementTypeCOMMENT, ElementTypeCONNECTOR };
  virtual ElementType Type() = 0;
  virtual bool ScalePoints(double xratio, double yratio, int offx, int offy) {
    return false;
  }
  virtual Variable *GetVariable() const {
    return NULL;
  }
  int X() {
    return _x;
  }
  void SetX(int x) {
    _x = x;
  }
  int Y() {
    return _y;
  }
  void SetY(int y) {
    _y = y;
  }
  int Width() const {
    return _width;
  }
  void SetWidth(int w) {
    _width = w;
  }
  int Height() const {
    return _height;
  }
  void SetHeight(int h) {
    _height = h;
  }

protected:
  // Default-initialized to 0 because not every element kind reads all four from
  // the sketch line: a connector record (`1,...`) carries only a point, so the
  // connector constructors set _x/_y but leave _width/_height untouched. Without
  // these initializers those fields would hold indeterminate stack values, which
  // is undefined behavior and made connector width/height non-deterministic
  // across a parse -> emit -> re-parse round trip (the comparator reads them).
  int _x = 0;
  int _y = 0;
  int _width = 0;
  int _height = 0;
};
typedef std::vector<VensimViewElement *> VensimViewElements;
class VensimVariableElement : public VensimViewElement {
public:
  VensimVariableElement(VensimView *view, char *curpos, char *buf, VensimParse *parser);
  VensimVariableElement(VensimView *view, Variable *var, int x, int y);
  ElementType Type() {
    return ElementTypeVARIABLE;
  }
  virtual Variable *GetVariable() const override {
    return _variable;
  }
  bool Ghost(std::set<Variable *, SymbolNameLess> *adds, bool update);
  bool CrossLevel() {
    return _cross_level;
  }
  void SetGhost(bool set) {
    _ghost = set;
  }
  bool Attached() {
    return _attached;
  }
  // Set the "attached to a valve" flag explicitly. The Vensim parser derives
  // this from sketch bit5 of the record's shape word; the XMILE reader does
  // not see that bit and must mark flow-paired variables programmatically.
  void SetAttached(bool set) {
    _attached = set;
  }
  // The record's tpos field: where the name sits relative to the shape
  // (0 inside, 1 below, 2 left, 3 above, 4 right; -1 on a flow's label, whose
  // placement the valve's own tpos describes).
  int TextPos() const {
    return _textPos;
  }
  void SetTextPos(int tpos) {
    _textPos = tpos;
  }
  // True when the record's size is its name's extent (a name, a flow's name, a
  // ghost) rather than a box of its own (a stock), so that a rename has to
  // re-size it.
  bool SizedByName() const {
    return _sizedByName;
  }
  void SetSizedByName(bool set) {
    _sizedByName = set;
  }

protected:
  Variable *_variable;
  bool _ghost;
  bool _cross_level;
  bool _attached;  // to a valve for flows
  int _textPos = 0;
  bool _sizedByName = false;
};
class VensimValveElement : public VensimViewElement {
public:
  ElementType Type() {
    return ElementTypeVALVE;
  }
  VensimValveElement(char *curpos, char *buf, VensimParse *parser);
  // Programmatic ctor for the XMILE reader path: the XMILE <flow> shape carries
  // no separate valve record, so we synthesize one at the flow's screen
  // coordinates and pair it with the flow's VensimVariableElement.
  VensimValveElement(int x, int y);
  bool Attached() {
    return _attached;
  }
  // tpos of the valve record: which side of the valve the flow's name sits on,
  // with the same encoding as VensimVariableElement::TextPos.
  int TextPos() const {
    return _textPos;
  }
  void SetTextPos(int tpos) {
    _textPos = tpos;
  }

private:
  bool _attached;
  int _textPos = 1;
};
class VensimCommentElement : public VensimViewElement {
public:
  ElementType Type() {
    return ElementTypeCOMMENT;
  }
  VensimCommentElement(char *curpos, char *buf, VensimParse *parser);
  // Programmatic ctor for the XMILE reader path: clouds (unmatched flow
  // endpoints) are emitted as type-12 comment records at the cloud position.
  VensimCommentElement(int x, int y);
  // A cloud is a comment record whose name field is icon index 48; anything
  // else is a free-text or picture comment.
  bool IsCloud() const {
    return _cloud;
  }

private:
  bool _cloud = false;
};
class VensimConnectorElement : public VensimViewElement {
public:
  ElementType Type() {
    return ElementTypeCONNECTOR;
  }
  VensimConnectorElement(char *curpos, char *buf, VensimParse *parser);
  // Programmatic ctor. polarity is '+' / '-' / 0 for none; the XMILE reader
  // passes it to preserve the polarity attribute on <connector> elements.
  // It defaults to none because the MDL writer reads the field
  // (EmitConnectorRecord), so leaving it indeterminate would make the emitted
  // polarity byte non-deterministic across a round trip.
  VensimConnectorElement(int from, int to, int x, int y, char polarity = 0);
  virtual bool ScalePoints(double xratio, double yratio, int offx, int offy) override;
  int From() {
    return _from;
  }
  int To() {
    return _to;
  }
  void Invalidate() {
    _to = _from = 0;
  }
  bool FromAsAlias();  // in this case From will send back a number
  char Polarity() const {
    return _polarity;
  }
  // The record's shape field: 1 for an arc (an information arrow), and for the
  // two pipe segments of a flow 4 on the downstream end (the arrowhead) and 100
  // on the upstream end.
  int Shape() const {
    return _shape;
  }
  void SetShape(int shape) {
    _shape = shape;
  }
  bool IsPipe() const {
    return _shape == kPipeDownstream || _shape == kPipeUpstream;
  }
  // The XMILE takeoff angle this connector was read with, if any. The point
  // (X/Y) rebuilt from it is rounded to whole pixels, so recomputing the angle
  // from the point would drift on every XMILE -> XMILE pass; the writer reuses
  // the original instead.
  bool HasAngle() const {
    return _hasAngle;
  }
  double Angle() const {
    return _angle;
  }
  void SetAngle(double angle) {
    _hasAngle = true;
    _angle = angle;
  }
  static const int kArc = 1;
  static const int kPipeDownstream = 4;
  static const int kPipeUpstream = 100;

private:
  int _from;
  int _to;
  int _npoints;
  char _polarity;
  int _shape = kArc;
  bool _hasAngle = false;
  double _angle = 0;
};

class VensimView : public View {
public:
  const std::string &Title() {
    return sTitle;
  }
  void SetTitle(const std::string &title) {
    sTitle = title;
  }
  void ReadView(VensimParse *parser, char *buf);
  int GetNextUID();
  VensimViewElements &Elements() {
    return vElements;
  }
  virtual bool empty() const override;

  virtual bool UpgradeGhost(Variable *var) override;
  virtual bool AddFlowDefinition(Variable *var, Variable *upstream, Variable *downstream) override;
  virtual bool AddVarDefinition(Variable *var, int x, int y) override;
  virtual void CheckGhostOwners() override;
  virtual void CheckLinksIn() override;
  bool FindInArrow(Variable *source, int target);
  void RemoveExtraArrowsIn(std::vector<Variable *> ins, int target);
  int FindVariable(Variable *in, int x, int y);  // add if necessary - returns UID

  int SetViewStart(int x, int y, double xratio, double yratio, int uid);  // returns last uid val + 1
  // Keep the view's own coordinates, translating only as far as needed to keep
  // every element's box at non-negative coordinates (XMILE does not allow
  // negative positions; Vensim does). Returns last uid val + 1, as SetViewStart.
  int KeepViewInPlace(int uid);
  // Move every element (connector points included) by (dx, dy).
  void Translate(int dx, int dy);
  // The right and bottom edges of the boxes of all non-connector elements, or
  // (0, 0) for a view with none.
  void GetViewExtent(int &right, int &bottom);
  int GetViewMaxX(int defval);
  int GetViewMaxY(int defval);
  int UIDOffset() {
    return _uid_offset;
  }

private:
  VensimViewElements vElements;
  std::string sTitle;
  int _uid_offset;
};

#endif
