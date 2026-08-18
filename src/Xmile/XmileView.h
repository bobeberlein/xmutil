#ifndef _XMUTIL_XMILE_XMILEVIEW_H
#define _XMUTIL_XMILE_XMILEVIEW_H
#include <string>
#include <unordered_map>
#include <vector>

namespace tinyxml2 {
class XMLElement;
}

class Model;
class SymbolNameSpace;
class Variable;
class VensimView;
class XmileReader;

// XmileView walks one XMILE <view> element and populates a caller-supplied
// VensimView with the corresponding sketch geometry. The XMILE reader owns the
// VensimView (it has already been registered with the Model); XmileView only
// mutates it.
//
// Walk order is three passes over the same <view> element:
//   1. AllocateElements - <stock>/<aux>/<flow>/<alias> become VensimViewElements
//      and the per-view name and XMILE-uid lookup maps are built.
//   2. ResolveConnectors - <connector> children resolve <from>/<to> against
//      those maps and become VensimConnectorElement records.
//   3. ProcessViewGroups - <group> children become ModelGroup membership.
//
// Pass 1 handles the cloud-synthesis path: a flow endpoint that does not
// match a stock or aux position becomes a VensimCommentElement at the cloud
// position, which is what the MDL writer recognizes as a cloud.
class XmileView {
public:
  XmileView(XmileReader *reader, Model *model, VensimView *view);

  // ProcessView orchestrates the three passes. Returns true on success; any
  // diagnostics (bad <pts> shape, unresolved connector endpoint, multi-view
  // notice) are pushed onto errs but only structural failures return false.
  bool ProcessView(tinyxml2::XMLElement *viewEl, std::vector<std::string> &errs);

private:
  // Pass 1: walk children, allocate elements, build name -> UID maps.
  void AllocateElements(tinyxml2::XMLElement *viewEl, std::vector<std::string> &errs);
  // Pass 2: walk <connector> children, resolve from/to names to UIDs.
  void ResolveConnectors(tinyxml2::XMLElement *viewEl, std::vector<std::string> &errs);
  // Pass 3: walk <group> children with view-level UID maps available.
  void ProcessViewGroups(tinyxml2::XMLElement *viewEl, std::vector<std::string> &errs);

  // Element allocation helpers. Each returns the UID (array index in
  // VensimView::Elements()) of the newly allocated element, or -1 on failure.

  // Allocate a plain VensimVariableElement at (x, y) for var.
  int AllocateVariableElement(Variable *var, int x, int y);
  // Allocate a paired VensimValveElement + VensimVariableElement for a flow.
  // The valve goes at UID N and the variable at UID N+1 (the MDL writer's
  // convention: elements[var_uid - 1] is the preceding valve). Returns the
  // VARIABLE's UID.
  int AllocateValveAndVariable(Variable *flowVar, int x, int y);
  // Allocate a VensimCommentElement at (x, y) -- represents a cloud.
  int AllocateCloud(int x, int y);
  // Allocate a ghost VensimVariableElement (an alias / second reference) at
  // (x, y). Sets _ghost=true regardless of whether the Variable already has a
  // home view, because aliases declared before their non-ghost reference
  // would otherwise default to the wrong polarity from the ctor's auto-derived
  // _ghost = (var->GetView() != NULL).
  int AllocateGhost(Variable *var, int x, int y);
  // Allocate a VensimConnectorElement with explicit polarity.
  int AllocateConnector(int fromUid, int toUid, int midX, int midY, char polarity);

  // For a flow endpoint position, find the nearest structurally-connected
  // stock (anchorStocks: folded names from XmileReader::StocksForFlow) within
  // a small pixel tolerance and return its UID. If none matches -- no
  // structural candidate, none placed in this view, or none in range --
  // synthesize a cloud (VensimCommentElement) at the position and return its
  // UID.
  int ResolveFlowEndpoint(double x, double y, const std::vector<std::string> *anchorStocks);

  // Resolve a <from> or <to> child of a <connector>: either text content (the
  // referenced variable's name) or a nested <alias uid="N"/> (the XMILE per-
  // file uid of an <alias> declared elsewhere in this view). Returns the
  // sequential UID of the referenced element, or -1 if no resolution succeeds.
  int ResolveEndpoint(tinyxml2::XMLElement *endpoint);

  // Reserve the next sequential UID slot in _view->Elements() (growing the
  // vector when needed). Allocates bottom-up (slot 1, 2, 3, ...) skipping
  // slot 0 -- the Vensim sketch wire format starts UIDs at 1, and bottom-up
  // matches the slot ordering the MDL/XMILE writers emit (they iterate from
  // index 0 upward and emit non-NULL slots in that order), so the
  // reader-writer-reader round trip preserves the slot -> element mapping.
  int ReserveNextSlot();

  XmileReader *_reader;
  Model *_model;
  SymbolNameSpace *_sns;
  VensimView *_view;
  // Bottom-up slot allocator. Starts at 1 (slot 0 is left empty to match the
  // Vensim sketch convention) and increments by one per allocation.
  int _nextUid;
  // Folded variable name (XmileReader::FoldNameKey: ASCII-lowered, '_'/' '
  // runs collapsed to one space) -> sequential UID (array index in
  // Elements()). The fold mirrors the namespace's ToLowerSpace hash key so
  // name references resolve here whenever they resolve in the namespace.
  std::unordered_map<std::string, int> _nameToUid;
  // XMILE per-file uid attribute -> our sequential UID. Built when an
  // <alias uid="N">, <stock uid="N">, <flow uid="N">, or <aux uid="N"> is
  // seen. Used by ResolveConnectors when <from><alias uid="N"/></from> refers
  // to an alias by UID, and by ProcessViewGroups when <item uid="N"/> refers
  // to a group member.
  std::unordered_map<int, int> _xmileUidToOurUid;
};

#endif
