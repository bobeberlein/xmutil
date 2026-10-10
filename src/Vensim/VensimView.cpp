#include "VensimView.h"

#include <algorithm>
#include <cmath>

#include "../Symbol/Variable.h"
#include "VensimParse.h"

namespace {

// Advance widths of Times-Roman in 1/1000 em (the Adobe AFM metrics, which
// Times New Roman matches closely), for printable ASCII from ' ' to '~'.
const short kTimesWidths[95] = {
    250, 333, 408, 500, 500, 833, 778, 180, 333, 333, 500, 564, 250, 333, 250, 278,  // ' ' .. '/'
    500, 500, 500, 500, 500, 500, 500, 500, 500, 500,                                // '0' .. '9'
    278, 278, 564, 564, 564, 444, 921,                                               // ':' .. '@'
    722, 667, 667, 722, 611, 556, 722, 722, 333, 389, 722, 611, 889,                 // 'A' .. 'M'
    722, 722, 556, 722, 667, 556, 611, 722, 722, 944, 722, 722, 611,                 // 'N' .. 'Z'
    333, 278, 333, 469, 500, 333,                                                    // '[' .. '`'
    444, 500, 444, 500, 444, 333, 500, 500, 278, 278, 500, 278, 778,                 // 'a' .. 'm'
    500, 500, 500, 500, 333, 389, 278, 500, 500, 722, 500, 500, 444,                 // 'n' .. 'z'
    480, 200, 480, 541,                                                              // '{' .. '~'
};

// Pixel width of text in Vensim's default font, Times New Roman 12pt -- 16px
// at the 96 ppi of the default font line. Anything outside printable ASCII is
// counted as one average-width character per code point.
double DefaultFontWidth(const std::string &text) {
  double units = 0;
  for (unsigned char c : text) {
    if (c >= 32 && c <= 126)
      units += kTimesWidths[c - 32];
    else if ((c & 0xC0) != 0x80)
      units += 500;
  }
  return units * 16.0 / 1000.0;
}

}  // namespace

void VensimDefaultNameSize(const std::string &rawName, bool ghost, int &halfWidth, int &halfHeight) {
  // Fitted to names Vensim laid out itself (test_models/NameSizing.mdl and the
  // other test_models sketches), which this reproduces to within 2px:
  //  - text is measured in the default font;
  //  - it wraps at spaces to a width of 120px, or sqrt(81 * text width) for
  //    long text (Vensim keeps long names from becoming one wide line);
  //  - the box is as wide as the widest line, at least 12px;
  //  - one line is 22px high, and each further line adds 56/3px.
  const std::string name = ghost ? "<" + rawName + ">" : rawName;
  const double total = DefaultFontWidth(name);
  const double wrapWidth = std::max(120.0, std::sqrt(81.0 * total));
  const double space = DefaultFontWidth(" ");
  int lines = 1;
  double line = 0, widest = 0;
  size_t start = 0;
  while (start <= name.size()) {
    size_t end = name.find(' ', start);
    if (end == std::string::npos)
      end = name.size();
    double word = DefaultFontWidth(name.substr(start, end - start));
    if (line > 0 && line + space + word > wrapWidth) {
      lines++;
      line = word;
    } else {
      line = line > 0 ? line + space + word : word;
    }
    widest = std::max(widest, line);
    start = end + 1;
  }
  halfWidth = std::max(6, static_cast<int>(std::lround(widest / 2)));
  halfHeight = std::max(11, static_cast<int>(std::lround((56.0 * lines + 2) / 6)));
}

VensimVariableElement::VensimVariableElement(VensimView *view, char *curpos, char *buf, VensimParse *parser) {
  std::string name;
  curpos = parser->GetString(curpos, name);  // this might be an index number

  curpos = parser->GetInt(curpos, _x);
  curpos = parser->GetInt(curpos, _y);
  curpos = parser->GetInt(curpos, _width);
  curpos = parser->GetInt(curpos, _height);

  int shape, bits;
  curpos = parser->GetInt(curpos, shape);
  if (shape & (1 << 5))
    _attached = true;
  else
    _attached = false;
  curpos = parser->GetInt(curpos, bits);
  if ((bits & 1))
    _ghost = false;
  else
    _ghost = true;
  _cross_level = false;
  // hid, hasf, then tpos. Older or hand-written records may stop short; keep
  // the default rather than reading a missing field as 0.
  int ignore;
  if (*curpos)
    curpos = parser->GetInt(curpos, ignore);
  if (*curpos)
    curpos = parser->GetInt(curpos, ignore);
  if (*curpos)
    curpos = parser->GetInt(curpos, _textPos);

#ifndef NDEBUG
  if (name == "P100")
    curpos = curpos;
#endif

  // try to find the variable
  _variable = parser->FindVariable(name);
  if (_variable) {
    if (_variable->GetView())
      _ghost = true;  // only allow 1 definition
    else if (!_ghost) {
      _variable->SetView(view);
      if (_attached)
        _variable->MarkAsFlow();
    }
  } else {
    std::string *nname = SymbolNameSpace::ToLowerSpace(name);
    if (*nname != "time")  // any others?
      log("Can't find - %s\n", name.c_str());
    delete nname;
  }
}
VensimVariableElement::VensimVariableElement(VensimView *view, Variable *var, int x, int y) {
  _x = x;
  _y = y;
  _width = _height = 0;
  _ghost = var->GetView() != NULL;
  _cross_level = false;
  // _attached is set later by MarkAttachedToFlow / equivalent caller logic when
  // a valve adopts this variable as its flow label; default it to false so the
  // comparator's attached-state check is deterministic across the round trip.
  _attached = false;
  _variable = var;
  _variable->SetView(view);
#ifndef NDEBUG
  if (var->GetName() == "P100")
    x = x;
#endif
}

VensimCommentElement::VensimCommentElement(char *curpos, char *buf, VensimParse *parser) {
  std::string name;
  curpos = parser->GetString(curpos, name);  // this might be an index number

  curpos = parser->GetInt(curpos, _x);
  curpos = parser->GetInt(curpos, _y);
  curpos = parser->GetInt(curpos, _width);
  curpos = parser->GetInt(curpos, _height);

  int shape, bits;
  curpos = parser->GetInt(curpos, shape);
  curpos = parser->GetInt(curpos, bits);

  // The name field of a comment is an icon index; 48 is the cloud.
  _cloud = name == "48";

  if (bits & (1 << 2))  // scratch name - it is the next line
  {
    parser->Lexer().ReadLine(buf, BUFLEN);
    name = buf;
  }
}

VensimValveElement::VensimValveElement(char *curpos, char *buf, VensimParse *parser) {
  std::string name;
  curpos = parser->GetString(curpos, name);  // this might be an index number

  curpos = parser->GetInt(curpos, _x);
  curpos = parser->GetInt(curpos, _y);
  curpos = parser->GetInt(curpos, _width);
  curpos = parser->GetInt(curpos, _height);
  int shape;
  curpos = parser->GetInt(curpos, shape);
  if (shape & (1 << 5))
    _attached = true;
  else
    _attached = false;
  // bits, hid, hasf, then tpos (see VensimVariableElement).
  int ignore;
  for (int i = 0; i < 3 && *curpos; i++)
    curpos = parser->GetInt(curpos, ignore);
  if (*curpos)
    curpos = parser->GetInt(curpos, _textPos);
}

bool VensimVariableElement::Ghost(std::set<Variable *, SymbolNameLess> *adds, bool update) {
  if (_ghost && !_cross_level) {
    if (adds) {
      std::set<Variable *, SymbolNameLess>::iterator it = adds->find(this->GetVariable());
      if (it != adds->end()) {
        if (update) {
          adds->erase(it);
          _cross_level = true;  // so it will continue to return false
        }
        return false;
      }
    }
    return true;
  }
  return false;
}

VensimConnectorElement::VensimConnectorElement(char *curpos, char *buf, VensimParse *parser) {
  curpos = parser->GetInt(curpos, _from);
  curpos = parser->GetInt(curpos, _to);
  std::string ignore;
  curpos = parser->GetInt(curpos, _shape);
  curpos = parser->GetString(curpos, ignore);
  int polarity_ascii;
  curpos = parser->GetInt(curpos, polarity_ascii);
  if (polarity_ascii == 'S' || polarity_ascii == 's') {
    parser->SetLetterPolarity(true);
    _polarity = '+';
  } else if (polarity_ascii == 'O' || polarity_ascii == '0') {
    parser->SetLetterPolarity(true);
    _polarity = '-';
  } else
    _polarity = polarity_ascii;  // might be invalid
  curpos = parser->GetString(curpos, ignore);
  curpos = parser->GetString(curpos, ignore);
  curpos = parser->GetString(curpos, ignore);
  curpos = parser->GetString(curpos, ignore);
  curpos = parser->GetString(curpos, ignore);
  curpos = parser->GetString(curpos, ignore);

  int npoints;
  sscanf(curpos, "%d|(%d,%d)", &npoints, &_x, &_y);
  _npoints = 1;  // todo get all of them

  std::string name;
  curpos = parser->GetString(curpos, name);  // this might be an index number
}

VensimConnectorElement::VensimConnectorElement(int from, int to, int x, int y, char polarity) {
  _from = from;
  _to = to;
  _npoints = 1;
  _x = x;
  _y = y;
  _polarity = polarity;
}

VensimValveElement::VensimValveElement(int x, int y) {
  _x = x;
  _y = y;
  // Vensim's own valve size (half-width, half-height).
  _width = 6;
  _height = 8;
  // The MDL sketch records a "shape bits" word whose bit-5 means "attached to
  // a flow variable". XMILE-synthesized valves are always paired with a
  // following VensimVariableElement, so they are attached.
  _attached = true;
}

VensimCommentElement::VensimCommentElement(int x, int y) {
  _x = x;
  _y = y;
  // The only comment the XMILE reader synthesizes is a cloud, at Vensim's own
  // cloud size.
  _width = 10;
  _height = 8;
  _cloud = true;
}

bool VensimConnectorElement::ScalePoints(double xs, double ys, int xo, int yo) {
  if (_x != 0 || _y != 0)  // invalide leave it alone
  {
    _x = _x * xs + xo;
    _y = _y * ys + yo;
  }
  return true;
}

void VensimView::ReadView(VensimParse *parser, char *buf) {
  VensimLex &lexer = parser->Lexer();
  while (true) {
    lexer.ReadLine(buf, BUFLEN);  // version line
    if (buf[0] < '0' || buf[0] > '9')
      break;
    int len = 0;
    int type = -1;
    int uid = -1;
    char *curpos = parser->GetInt(buf, type);
    curpos = parser->GetInt(curpos, uid);
    if (type >= 0 && uid >= 0)  // otherwise ignore
    {
      if (uid > len) {
        len = uid + 25;
        vElements.resize(len + 1, NULL);
      }
      assert(vElements[uid] == NULL);
      switch (type) {
      case 10:  // a variable
        vElements[uid] = new VensimVariableElement(this, curpos, buf, parser);
        break;
      case 11:  // a valve if connected to a variable always just after it in the lest (??)
        vElements[uid] = new VensimValveElement(curpos, buf, parser);
        break;
      case 12:  // a comment including clouds
        vElements[uid] = new VensimCommentElement(curpos, buf, parser);
        break;
      case 1:  // a connector
        vElements[uid] = new VensimConnectorElement(curpos, buf, parser);
        break;
      case 30:  // a ??????
        break;
      default:
        assert(false);
        break;
      }
    }
  }
}

int VensimView::GetNextUID() {
  // Walk from the top of the vector down looking for a NULL slot. The Vensim
  // sketch convention reserves UID 0 (the Vensim parser populates from index 1
  // upward, then connectors reference those indices), so the scan stops at
  // index 1. When the vector is empty or fully packed, grow by 25 NULL slots
  // and re-scan; the recursion terminates because the resize guarantees at
  // least one NULL slot above index 0.
  //
  // Pre-Phase-7 callers (the Vensim sketch parser) always pre-sized the vector
  // to at least the highest UID + 25 before invoking GetNextUID, so an empty
  // vector never reached the for-loop. The XMILE reader path can hit this with
  // an empty vector, so the size-0 guard is now mandatory: the original
  // `--i > 0` underflows size_t when vElements.size() == 0 and would deref
  // garbage at the first iteration.
  if (!vElements.empty()) {
    for (size_t i = vElements.size() - 1; i > 0; --i) {
      if (!vElements[i])
        return static_cast<int>(i);
    }
  }
  vElements.resize(vElements.size() + 25, NULL);
  return GetNextUID();
}

int VensimView::SetViewStart(int startx, int starty, double xratio, double yratio, int uid_start) {
  _uid_offset = uid_start;
  if (this->vElements.empty())
    return _uid_offset;
  int min_x = INT32_MAX;
  int min_y = INT32_MAX;
  for (VensimViewElement *ele : vElements) {
    if (ele) {
      // A connector is an edge, not a positioned box: its _x/_y is a routing
      // waypoint (and a straight connector carries a degenerate (0,0) point).
      // Letting one define the view's minimum would peg the origin at (0,0) and
      // translate the whole diagram on every round trip, since SetViewStart then
      // shifts min to (startx,starty) and the connector's points move with it.
      // Bound the view by the actual boxed elements; connectors are translated
      // by the same offset below via ScalePoints.
      if (ele->Type() == VensimViewElement::ElementTypeCONNECTOR)
        continue;
      if (ele->X() < min_x)
        min_x = ele->X();
      if (ele->Y() < min_y)
        min_y = ele->Y();
    }
  }
  // A view with no boxed elements at all (only connectors) has no meaningful
  // origin to normalize against; leave coordinates untranslated rather than
  // shifting by the INT32_MAX sentinel.
  if (min_x == INT32_MAX || min_y == INT32_MAX)
    return _uid_offset + vElements.size();
  int off_x = std::round(startx - min_x * xratio);
  int off_y = std::round(starty - min_y * yratio);
  for (VensimViewElement *ele : vElements) {
    if (ele) {
      if (!ele->ScalePoints(xratio, yratio, off_x, off_y)) {
        ele->SetX(std::round(ele->X() * xratio + off_x));
        ele->SetY(std::round(ele->Y() * yratio + off_y));
        ele->SetWidth(std::round(ele->Width() * xratio));
        ele->SetHeight(std::round(ele->Height() * yratio));
      }
    }
  }
  return _uid_offset + vElements.size();
}

int VensimView::KeepViewInPlace(int uid_start) {
  _uid_offset = uid_start;
  // Bound by each element's box (center minus half size), not just its center:
  // the XMILE writer emits a sized stock by its top-left corner. Connectors are
  // edges, not boxes, and are translated along with everything else below.
  int min_x = 0;
  int min_y = 0;
  for (VensimViewElement *ele : vElements) {
    if (!ele || ele->Type() == VensimViewElement::ElementTypeCONNECTOR)
      continue;
    min_x = std::min(min_x, ele->X() - ele->Width());
    min_y = std::min(min_y, ele->Y() - ele->Height());
  }
  if (min_x < 0 || min_y < 0)
    Translate(-min_x, -min_y);
  return _uid_offset + vElements.size();
}

void VensimView::Translate(int dx, int dy) {
  if (dx == 0 && dy == 0)
    return;
  for (VensimViewElement *ele : vElements) {
    if (ele && !ele->ScalePoints(1.0, 1.0, dx, dy)) {
      ele->SetX(ele->X() + dx);
      ele->SetY(ele->Y() + dy);
    }
  }
}

void VensimView::GetViewExtent(int &right, int &bottom) {
  right = bottom = 0;
  for (VensimViewElement *ele : vElements) {
    if (!ele || ele->Type() == VensimViewElement::ElementTypeCONNECTOR)
      continue;
    right = std::max(right, ele->X() + ele->Width());
    bottom = std::max(bottom, ele->Y() + ele->Height());
  }
}

int VensimView::GetViewMaxX(int defval) {
  if (this->vElements.empty())
    return defval;
  int max_x = -INT32_MAX;
  for (VensimViewElement *ele : vElements) {
    if (ele) {
      if (ele->X() > max_x)
        max_x = ele->X();
    }
  }
  return max_x;
}
int VensimView::GetViewMaxY(int defval) {
  if (this->vElements.empty())
    return defval;
  int max_y = -INT32_MAX;
  for (VensimViewElement *ele : vElements) {
    if (ele) {
      if (ele->Y() > max_y)
        max_y = ele->Y();
    }
  }
  return max_y;
}

bool VensimView::UpgradeGhost(Variable *var) {
  for (VensimViewElement *ele : vElements) {
    if (ele && ele->Type() == VensimViewElement::ElementTypeVARIABLE) {
      VensimVariableElement *vele = static_cast<VensimVariableElement *>(ele);
      if (vele->GetVariable() == var) {
        assert(vele->Ghost(NULL, false));
        vele->SetGhost(false);
        var->SetView(this);  // now done
        return true;
      }
    }
  }
  return false;
}

bool VensimView::empty() const {
  int i = 5;
  if (this->sTitle == "Energy sensitivity")
    i++;
  for (VensimViewElement *vve : vElements) {
    if (vve) {
      Variable *var = vve->GetVariable();
      if (var && var->GetView() == this && !var->Unwanted())
        return false;
    }
  }
  return true;
}

bool VensimView::AddFlowDefinition(Variable *var, Variable *upstream, Variable *downstream) {
  // for flows we are looking for any stocks that use the flow
  // for
  int xstart, ystart, xend, yend;
  xstart = ystart = xend = yend = 0;
  bool startfound = false;
  bool endfound = false;
  for (VensimViewElement *ele : vElements) {
    if (ele && ele->Type() == VensimViewElement::ElementTypeVARIABLE) {
      VensimVariableElement *vele = static_cast<VensimVariableElement *>(ele);
      if (vele->GetVariable() == upstream) {
        xstart = vele->X();
        ystart = vele->Y();
        startfound = true;
        if (endfound)
          break;
      } else if (vele->GetVariable() == downstream) {
        xend = vele->X();
        yend = vele->Y();
        endfound = true;
        if (startfound)
          break;
      }
    }
  }
  if (!startfound && !endfound)
    return false;              // can't find anything - should not happen
  if (startfound && endfound)  // put in the middle
  {
    xstart = (xstart + xend) / 2;
    ystart = (ystart + yend) / 2;
  } else if (startfound) {
    xstart += 60;
  } else if (endfound) {
    xstart = xend - 60;
    ystart = yend;
  }
  // add the var to this view
  int uid = this->GetNextUID();
  vElements[uid] = new VensimVariableElement(this, var, xstart, ystart);
  return true;
}

bool VensimView::AddVarDefinition(Variable *var, int x, int y) {
  // add the var to this view
  int uid = this->GetNextUID();
  vElements[uid] = new VensimVariableElement(this, var, x, y);
  return true;
}

// add if msising - if extra just ignore
void VensimView::CheckGhostOwners() {
  int uid;
  int n = vElements.size();
  for (uid = 0; uid < n; uid++) {
    VensimViewElement *ele = vElements[uid];
    if (ele && ele->Type() == VensimViewElement::ElementTypeVARIABLE) {
      VensimVariableElement *vele = static_cast<VensimVariableElement *>(ele);
      Variable *var = vele->GetVariable();
      if (var && var->GetView() == NULL) {
        var->SetView(this);
        vele->SetGhost(false);
        // A ghost laid out by its name was sized as "<name>"; promoted, it is
        // drawn as the plain name.
        if (vele->SizedByName()) {
          int hw, hh;
          VensimDefaultNameSize(var->GetName(), false, hw, hh);
          vele->SetWidth(hw);
          vele->SetHeight(hh);
        }
      }
    }
  }
}

// add if msising - if extra just ignore
void VensimView::CheckLinksIn() {
  int uid;
  int n = vElements.size();
  for (uid = 0; uid < n; uid++) {
    VensimViewElement *ele = vElements[uid];
    if (ele && ele->Type() == VensimViewElement::ElementTypeVARIABLE) {
      VensimVariableElement *vele = static_cast<VensimVariableElement *>(ele);
      Variable *var = vele->GetVariable();
      if (var && var->VariableType() != XMILE_Type_STOCK && !vele->Ghost(NULL, false)) {
        std::vector<Variable *> ins = var->GetInputVars();
        for (Variable *in : ins) {
          if (!this->FindInArrow(in, uid) && in->VariableType() != XMILE_Type_ARRAY &&
              in->VariableType() != XMILE_Type_ARRAY_ELM && in->VariableType() != XMILE_Type_UNKNOWN) {
            int fromuid = this->FindVariable(in, vele->X(), vele->Y() + 30);
            int x = (vElements[fromuid]->X() + vele->X()) / 2;
            int y = (vElements[fromuid]->Y() + vele->Y()) / 2;
            int nuid = this->GetNextUID();
            vElements[nuid] = new VensimConnectorElement(fromuid, uid, x, y);
          }
        }
        this->RemoveExtraArrowsIn(ins, uid);  // sometimes there are anomolous arrows that show up
      }
    }
  }
}

bool VensimView::FindInArrow(Variable *in, int target) {
  for (VensimViewElement *ele : this->vElements) {
    if (ele && ele->Type() == VensimViewElement::ElementTypeCONNECTOR) {
      VensimConnectorElement *cele = static_cast<VensimConnectorElement *>(ele);
      int to = cele->To();
      VensimValveElement *tele = static_cast<VensimValveElement *>(vElements[to]);
      if (tele && tele->Type() == VensimViewElement::ElementTypeVALVE && tele->Attached())
        to++;
      if (to == target) {
        VensimVariableElement *from = static_cast<VensimVariableElement *>(vElements[cele->From()]);
        if (from && from->Type() == VensimViewElement::ElementTypeVALVE && tele->Attached() &&
            static_cast<VensimValveElement *>(vElements[cele->From()])->Attached())
          from = static_cast<VensimVariableElement *>(vElements[cele->From() + 1]);
        if (from->Type() == VensimViewElement::ElementTypeVARIABLE && from->GetVariable() == in)
          return true;
      }
    }
  }
  return false;
}

void VensimView::RemoveExtraArrowsIn(std::vector<Variable *> ins, int target) {
  for (VensimViewElement *ele : this->vElements) {
    if (ele && ele->Type() == VensimViewElement::ElementTypeCONNECTOR) {
      VensimConnectorElement *cele = static_cast<VensimConnectorElement *>(ele);
      int to = cele->To();
      VensimValveElement *tele = static_cast<VensimValveElement *>(vElements[to]);
      if (tele && tele->Type() == VensimViewElement::ElementTypeVALVE && tele->Attached())
        to++;
      if (to == target) {
        VensimVariableElement *from = static_cast<VensimVariableElement *>(vElements[cele->From()]);
        if (from && from->Type() == VensimViewElement::ElementTypeVALVE && tele->Attached() &&
            static_cast<VensimValveElement *>(vElements[cele->From()])->Attached())
          from = static_cast<VensimVariableElement *>(vElements[cele->From() + 1]);
        bool found = false;
        for (Variable *in : ins) {
          if (from->GetVariable() == in) {
            found = true;
            break;
          }
        }
        if (!found)
          cele->Invalidate();
      }
    }
  }
}

int VensimView::FindVariable(Variable *in, int x, int y) {
  int uid = 0;
  for (VensimViewElement *ele : this->vElements) {
    if (ele && ele->Type() == VensimViewElement::ElementTypeVARIABLE) {
      VensimVariableElement *vele = static_cast<VensimVariableElement *>(ele);
      if (vele->GetVariable() == in)
        return uid;
    }
    uid++;
  }
  uid = GetNextUID();
  vElements[uid] = new VensimVariableElement(this, in, x, y);
  return uid;
}
