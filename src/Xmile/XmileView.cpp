#include "XmileView.h"

#include <tinyxml2.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <set>
#include <utility>
#include <vector>

#include "../Model.h"
#include "../Symbol/Symbol.h"
#include "../Symbol/SymbolNameSpace.h"
#include "../Symbol/Variable.h"
#include "../Vensim/VensimView.h"
#include "XmileReader.h"

namespace {

// Case-insensitive prefix test using only standard <cctype>. POSIX strncasecmp
// is unavailable on MSVC, and _strnicmp is unavailable on POSIX; this helper
// avoids both platform-specific names.
static bool StartsWithIgnoreCase(const char *s, const char *prefix) {
  while (*prefix) {
    if (!*s)
      return false;
    if (std::tolower(static_cast<unsigned char>(*s)) != std::tolower(static_cast<unsigned char>(*prefix)))
      return false;
    ++s;
    ++prefix;
  }
  return true;
}

// True when a folded name key belongs to a simulation control variable (Time,
// DT, SAVEPER, INITIAL/FINAL TIME, ...). Control variables are never drawn in a
// sketch -- Vensim keeps them in the .Control section and both xmutil writers
// skip them -- so the reader treats a control name specially in two places:
// AllocateElements does not create a sketch element for one (matching the
// writers), and ResolveConnectors drops a connector that names one instead of
// reporting an unresolved-reference diagnostic (some producers emit dependency
// arrows from Time/DT into the variables that use them). Both the XMILE keyword
// spellings (dt, time_step, starttime, ...) and the Vensim control-variable
// names fold to the entries below.
static bool IsControlVariableKey(const std::string &foldedKey) {
  static const char *const kControlKeys[] = {
      "time",       "dt",        "time step", "saveper",  "save step", "initial time", "final time",
      "start time", "starttime", "stop time", "stoptime", "end time",  "endtime",
  };
  for (const char *k : kControlKeys) {
    if (foldedKey == k)
      return true;
  }
  return false;
}

// A connector endpoint carries a variable name as element text. Some producers
// wrap a name containing special characters in double quotes (Vensim's quoting
// convention leaking into the XMILE), while the matching <var>/<aux>/... name
// attribute is unquoted. Strip a single matched surrounding pair so the two
// spellings fold to the same key.
static std::string StripSurroundingQuotes(const std::string &s) {
  if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
    return s.substr(1, s.size() - 2);
  return s;
}

// The folded name key for a text endpoint, or empty for an alias/empty endpoint.
// The per-view name map keys through XmileReader::FoldNameKey, which mirrors
// the fold ToLowerSpace applies to namespace hash keys -- so a connector
// endpoint written in Stella's canonical form ("MY_VAR") finds a view element
// declared as "My Var" exactly the way the namespace itself would resolve the
// variable.
static std::string EndpointFoldedKey(tinyxml2::XMLElement *endpoint) {
  const char *text = endpoint->GetText();
  if (!text)
    return std::string();
  return XmileReader::FoldNameKey(XmileReader::NormalizeName(StripSurroundingQuotes(text).c_str()));
}

static bool EndpointIsControlVariable(tinyxml2::XMLElement *endpoint) {
  return IsControlVariableKey(EndpointFoldedKey(endpoint));
}

// A <from>/<to> whose only child is <alias uid="N"/> references a ghost element
// declared elsewhere in the view. When that uid was never declared (a
// malformed/incomplete view), the reference is dangling: there is no element to
// connect to, so the connector is dropped without diagnostic, the same way an
// endpoint naming a control variable is.
static bool EndpointIsAliasReference(tinyxml2::XMLElement *endpoint) {
  return endpoint->FirstChildElement("alias") != nullptr;
}

}  // namespace

XmileView::XmileView(XmileReader *reader, Model *model, VensimView *view)
    : _reader(reader),
      _model(model),
      _sns(reader ? reader->GetSymbolNameSpace() : nullptr),
      _view(view),
      // The Vensim sketch wire format leaves slot 0 unused (UIDs start at 1);
      // allocate the first usable slot at 1 so the MDL writer's slot-order
      // iteration produces the same on-wire UIDs that this reader recorded.
      _nextUid(1) {
}

bool XmileView::ProcessView(tinyxml2::XMLElement *viewEl, std::vector<std::string> &errs) {
  if (!viewEl || !_view)
    return false;
  AllocateElements(viewEl, errs);
  ResolveConnectors(viewEl, errs);
  ProcessViewGroups(viewEl, errs);
  return true;
}

void XmileView::AllocateElements(tinyxml2::XMLElement *viewEl, std::vector<std::string> &errs) {
  // Shared prologue for the named, positioned element kinds (<stock>, <aux>,
  // <flow>): read the name attribute, normalize it, resolve the Variable, and
  // read the center coordinates. Elements with no name attribute are skipped.
  struct Decl {
    std::string norm;
    Variable *var;
    int x, y;
  };
  auto readDecl = [this, &errs](tinyxml2::XMLElement *child, Decl &d) {
    const char *name = child->Attribute("name");
    if (!name)
      return false;
    d.norm = XmileReader::NormalizeName(name);
    d.var = _reader->InsertVariable(d.norm);
    if (!d.var) {
      // The name is held by a non-Variable symbol -- one of the builtins the
      // XmileReader ctor registers into this same namespace (STEP, SUM,
      // TREND, ...). There is no Variable to place, and every allocator below
      // dereferences the pointer it is handed, so the element is dropped with
      // the same wording DeclareVariable uses at the declaration sites.
      errs.push_back(std::string("<") + child->Name() + " name=\"" + name +
                     "\">: name collides with a non-variable symbol; the sketch element is dropped");
      return false;
    }
    d.x = static_cast<int>(child->DoubleAttribute("x", 0.0));
    d.y = static_cast<int>(child->DoubleAttribute("y", 0.0));
    return true;
  };
  // Record an allocated element under both lookup channels: the folded-name
  // map (connector endpoints by name) and, when the producer stamped a
  // per-file uid attribute, the XMILE-uid map (alias/group references).
  auto recordUid = [this](tinyxml2::XMLElement *child, const std::string &norm, int uid) {
    _nameToUid[XmileReader::FoldNameKey(norm)] = uid;
    if (const char *xmileUid = child->Attribute("uid"))
      _xmileUidToOurUid[std::atoi(xmileUid)] = uid;
  };

  for (tinyxml2::XMLElement *child = viewEl->FirstChildElement(); child; child = child->NextSiblingElement()) {
    if (XmileReader::IsForeignNamespace(child->Name()))
      continue;
    const char *tagC = child->Name();
    if (!tagC)
      continue;
    std::string tag(tagC);
    if (tag == "stock" || tag == "aux") {
      Decl d;
      if (!readDecl(child, d))
        continue;
      // Control variables (Time, DT, SAVEPER, INITIAL/FINAL TIME) are never
      // drawn in the sketch: Vensim keeps them in the .Control section and both
      // xmutil writers skip them (XMILEGenerator's Unwanted() filter). A few
      // producers nonetheless place them as <aux> elements; skip those here so
      // the reader's view matches what the writers round-trip to.
      if (IsControlVariableKey(XmileReader::FoldNameKey(d.norm)))
        continue;
      int uid = AllocateVariableElement(d.var, d.x, d.y);
      if (uid < 0)
        continue;
      recordUid(child, d.norm, uid);
    } else if (tag == "flow") {
      Decl d;
      if (!readDecl(child, d))
        continue;
      int varUid = AllocateValveAndVariable(d.var, d.x, d.y);
      if (varUid < 0) {
        errs.push_back(std::string("<flow name=\"") + d.norm + "\">: could not allocate adjacent valve/variable slots");
        continue;
      }
      recordUid(child, d.norm, varUid);
      // Pipe endpoints: <pts><pt x= y=/><pt x= y=/></pts>. Resolve each
      // endpoint to either an existing stock/aux UID (positional match) or a
      // freshly synthesized cloud (VensimCommentElement). Then drop two
      // connector records: source -> valve and valve -> sink.
      tinyxml2::XMLElement *pts = child->FirstChildElement("pts");
      if (!pts)
        continue;
      std::vector<std::pair<int, int>> endpoints;
      for (tinyxml2::XMLElement *pt = pts->FirstChildElement("pt"); pt; pt = pt->NextSiblingElement("pt")) {
        endpoints.emplace_back(static_cast<int>(pt->DoubleAttribute("x", 0.0)),
                               static_cast<int>(pt->DoubleAttribute("y", 0.0)));
      }
      if (endpoints.size() != 2) {
        errs.push_back(std::string("<flow name=\"") + d.norm + "\">: <pts> does not have exactly 2 <pt> children");
        continue;
      }
      // Only stocks structurally connected to this flow (its name appears in
      // their <inflow>/<outflow> lists) are pipe-anchor candidates; geometry
      // then picks WHICH of those (up to two) each endpoint touches, and
      // detects the cloud case. Without the structural filter, any
      // geometrically nearby element -- including an unrelated aux -- could
      // capture the endpoint.
      const std::vector<std::string> *anchorStocks = _reader->StocksForFlow(XmileReader::FoldNameKey(d.norm));
      int srcUid = ResolveFlowEndpoint(endpoints[0].first, endpoints[0].second, anchorStocks);
      int dstUid = ResolveFlowEndpoint(endpoints[1].first, endpoints[1].second, anchorStocks);
      int valveUid = varUid - 1;
      // For the midpoint use the RESOLVED endpoint's actual stored position,
      // not the input <pt> coordinates. When the endpoint resolved to a stock
      // or aux the recorded position is the variable's center (a stock's
      // border is several pixels off its center, so the input <pt> and the
      // resolved variable's X/Y differ); using the variable's center matches
      // what the XMILE writer emits on re-emit ("xanchor[count] = stock->X()"
      // in XMILEGenerator.cpp), so the round-trip midpoint stays stable.
      int srcX = endpoints[0].first;
      int srcY = endpoints[0].second;
      int dstX = endpoints[1].first;
      int dstY = endpoints[1].second;
      VensimViewElements &els = _view->Elements();
      if (srcUid >= 0 && static_cast<size_t>(srcUid) < els.size() && els[srcUid]) {
        srcX = els[srcUid]->X();
        srcY = els[srcUid]->Y();
      }
      if (dstUid >= 0 && static_cast<size_t>(dstUid) < els.size() && els[dstUid]) {
        dstX = els[dstUid]->X();
        dstY = els[dstUid]->Y();
      }
      // Both pipe records originate at the valve (From = valve) so the XMILE
      // writer's <pts> reconstruction loop (XMILEGenerator.cpp around the
      // `From() == local_uid - 1` predicate) can find BOTH endpoints during
      // emission. Without this convention the writer only sees the outflow
      // connector and falls back to a synthetic pipe-pt heuristic, losing the
      // original endpoint positions on the next round trip.
      AllocateConnector(valveUid, srcUid, (srcX + d.x) / 2, (srcY + d.y) / 2, 0);
      AllocateConnector(valveUid, dstUid, (d.x + dstX) / 2, (d.y + dstY) / 2, 0);
    } else if (tag == "alias") {
      tinyxml2::XMLElement *ofEl = child->FirstChildElement("of");
      if (!ofEl || !ofEl->GetText())
        continue;
      std::string ofName = XmileReader::NormalizeName(ofEl->GetText());
      // XMILE encodes alias targets in canonical form (spaces replaced with
      // underscores) so the name is a single token; xmutil stores variable
      // names with literal spaces. Convert back.
      std::replace(ofName.begin(), ofName.end(), '_', ' ');
      Variable *v = _reader->InsertVariable(ofName);
      if (!v) {
        errs.push_back(std::string("<alias><of>") + ofName +
                       "</of></alias>: name collides with a non-variable symbol; the sketch element is dropped");
        continue;
      }
      int x = static_cast<int>(child->DoubleAttribute("x", 0.0));
      int y = static_cast<int>(child->DoubleAttribute("y", 0.0));
      int uid = AllocateGhost(v, x, y);
      if (uid < 0)
        continue;
      if (const char *xmileUid = child->Attribute("uid"))
        _xmileUidToOurUid[std::atoi(xmileUid)] = uid;
    }
    // <connector> and <group> are handled by passes 2 and 3. isee:* and
    // unknown elements (graphs, sliders, ...) drop through silently because
    // they have no sketch counterpart.
  }
}

int XmileView::ReserveNextSlot() {
  int uid = _nextUid++;
  VensimViewElements &els = _view->Elements();
  if (static_cast<size_t>(uid) >= els.size())
    els.resize(uid + 1, nullptr);
  return uid;
}

int XmileView::AllocateVariableElement(Variable *var, int x, int y) {
  // VensimVariableElement's programmatic ctor dereferences var immediately
  // (unlike the parse-driven ctor, which tolerates an unresolved name). The
  // callers above already refuse a NULL with a diagnostic naming the element;
  // this guard is what makes the crash unreachable rather than merely unmet.
  if (!var)
    return -1;
  int uid = ReserveNextSlot();
  _view->Elements()[uid] = new VensimVariableElement(_view, var, x, y);
  return uid;
}

int XmileView::AllocateValveAndVariable(Variable *flowVar, int x, int y) {
  // The MDL sketch convention pairs a flow's variable record at UID N+1 with
  // the preceding valve at UID N; the MDL writer reads elements[var_uid - 1]
  // to find the valve. With the bottom-up sequential allocator below, the
  // valve goes at the lower index (allocated first) and the variable at the
  // index immediately after (allocated second), so the var_uid - 1 lookup
  // always lands on the right valve.
  if (!flowVar)
    return -1;
  int valveUid = ReserveNextSlot();
  _view->Elements()[valveUid] = new VensimValveElement(x, y);
  int varUid = ReserveNextSlot();
  VensimVariableElement *ve = new VensimVariableElement(_view, flowVar, x, y);
  // Flow-paired variables carry shape bit5 ("attached to a valve") in the MDL
  // sketch encoding. The Vensim parser reads this back when re-parsing; the
  // programmatic ctor cannot infer it (there is no shape word), so set it
  // explicitly here so the round trip preserves the bit.
  ve->SetAttached(true);
  _view->Elements()[varUid] = ve;
  return varUid;
}

int XmileView::AllocateCloud(int x, int y) {
  int uid = ReserveNextSlot();
  _view->Elements()[uid] = new VensimCommentElement(x, y);
  return uid;
}

int XmileView::AllocateGhost(Variable *var, int x, int y) {
  if (!var)
    return -1;
  int uid = ReserveNextSlot();
  VensimVariableElement *ve = new VensimVariableElement(_view, var, x, y);
  // The base ctor sets _ghost = (var->GetView() != NULL). For aliases declared
  // before any non-ghost reference to the same variable, that derivation is
  // wrong: the alias must always be a ghost regardless of declaration order.
  // Override explicitly.
  ve->SetGhost(true);
  _view->Elements()[uid] = ve;
  return uid;
}

int XmileView::AllocateConnector(int fromUid, int toUid, int midX, int midY, char polarity) {
  if (fromUid < 0 || toUid < 0)
    return -1;
  int uid = ReserveNextSlot();
  _view->Elements()[uid] = new VensimConnectorElement(fromUid, toUid, midX, midY, polarity);
  return uid;
}

int XmileView::ResolveFlowEndpoint(double x, double y, const std::vector<std::string> *anchorStocks) {
  // A flow's pipe <pt> is the position of the stock that anchors that end of
  // the pipe, but it is drawn at the element's BOUNDARY rather than its
  // center. Vensim's default stock box is 60x40 pixels, so a pipe endpoint
  // can sit ~30 pixels horizontally or ~20 pixels vertically off the stock's
  // centered position. The matching tolerance therefore has to be wide enough
  // to capture that gap without false-matching the flow's OTHER anchor stock
  // (the only other candidate, typically several stock-widths away). Pick the
  // axis-aligned bound at 35 pixels: covers a default-sized stock plus a few
  // pixels of slack, while staying well under the inter-element spacing seen
  // in the corpora.
  // Only the structural anchorStocks (the stocks whose <inflow>/<outflow>
  // lists name this flow) are candidates, so a flow's own center can never
  // capture one of its own pipe endpoints.
  const double kTolerance = 35.0;
  int bestUid = -1;
  double bestDist = kTolerance + 1.0;
  if (anchorStocks) {
    for (const std::string &key : *anchorStocks) {
      auto it = _nameToUid.find(key);
      if (it == _nameToUid.end())
        continue;  // structurally connected, but not placed in this view
      VensimViewElement *el = _view->Elements()[it->second];
      double vx = static_cast<double>(el->X());
      double vy = static_cast<double>(el->Y());
      if (std::fabs(vx - x) <= kTolerance && std::fabs(vy - y) <= kTolerance) {
        double dist = std::hypot(vx - x, vy - y);
        if (dist < bestDist) {
          bestDist = dist;
          bestUid = it->second;
        }
      }
    }
  }
  if (bestUid >= 0)
    return bestUid;
  // No structural anchor in range -- the endpoint is a cloud. Synthesize a
  // VensimCommentElement at the cloud position; the MDL writer treats type-12
  // records at pipe endpoints as cloud sources/sinks.
  return AllocateCloud(static_cast<int>(x), static_cast<int>(y));
}

void XmileView::ResolveConnectors(tinyxml2::XMLElement *viewEl, std::vector<std::string> &errs) {
  for (tinyxml2::XMLElement *child = viewEl->FirstChildElement("connector"); child;
       child = child->NextSiblingElement("connector")) {
    // Polarity attribute. XMILE 1.0 uses the literal "+"/"-" character, but
    // some Stella exports emit "positive"/"negative". Accept either via a
    // case-insensitive prefix check on the first three letters of the word
    // form. Anything that fails both shapes is silently treated as no
    // polarity (the MDL writer simply omits the polarity glyph).
    char polarity = 0;
    if (const char *p = child->Attribute("polarity")) {
      if (p[0] == '+') {
        polarity = '+';
      } else if (p[0] == '-') {
        polarity = '-';
      } else if (StartsWithIgnoreCase(p, "pos")) {
        polarity = '+';
      } else if (StartsWithIgnoreCase(p, "neg")) {
        polarity = '-';
      }
    }
    tinyxml2::XMLElement *fromEl = child->FirstChildElement("from");
    tinyxml2::XMLElement *toEl = child->FirstChildElement("to");
    if (!fromEl || !toEl) {
      errs.push_back("<connector>: missing <from> or <to> child");
      continue;
    }
    int fromUid = ResolveEndpoint(fromEl);
    int toUid = ResolveEndpoint(toEl);
    if (fromUid < 0 || toUid < 0) {
      // An endpoint that names a control variable has no sketch element and is
      // dropped without diagnostic (see IsControlVariableKey). Any other
      // unresolved endpoint is a genuine dangling reference worth surfacing.
      bool fromDroppable = fromUid < 0 && (EndpointIsControlVariable(fromEl) || EndpointIsAliasReference(fromEl));
      bool toDroppable = toUid < 0 && (EndpointIsControlVariable(toEl) || EndpointIsAliasReference(toEl));
      bool fromOk = fromUid >= 0 || fromDroppable;
      bool toOk = toUid >= 0 || toDroppable;
      if (!fromOk || !toOk)
        errs.push_back("<connector>: unresolved <from> or <to> reference");
      continue;
    }
    // XMILE carries an "angle" attribute (geometric direction); the MDL
    // sketch format expects a midpoint (x, y). Most XMILE writers omit
    // numeric x/y on connectors -- when absent, midpoint defaults to (0, 0)
    // and the MDL writer renders an auto-routed connector. The pre-existing
    // VensimConnectorElement(int, int, int, int, char) ctor accepts the
    // values directly.
    int midX = static_cast<int>(child->DoubleAttribute("x", 0.0));
    int midY = static_cast<int>(child->DoubleAttribute("y", 0.0));
    AllocateConnector(fromUid, toUid, midX, midY, polarity);
  }
}

int XmileView::ResolveEndpoint(tinyxml2::XMLElement *endpoint) {
  // <from><alias uid="N"/></from> -- the nested alias references an <alias>
  // declared elsewhere in this view by its XMILE-per-file uid. The reader
  // recorded that mapping during pass 1.
  if (tinyxml2::XMLElement *alias = endpoint->FirstChildElement("alias")) {
    if (const char *uidStr = alias->Attribute("uid")) {
      auto it = _xmileUidToOurUid.find(std::atoi(uidStr));
      if (it != _xmileUidToOurUid.end())
        return it->second;
    }
    return -1;
  }
  // <from>name</from> -- text content is the referenced variable's name.
  // Stella's connector emitter uses the canonical (underscore-quoted) form
  // even when the variable's display name has spaces ("marginal_productivity
  // _per_developer" referencing the aux "marginal productivity per developer"),
  // while xmutil's writer emits display names verbatim. The map key folds
  // case and '_'<->' ' (XmileReader::FoldNameKey), so both spellings -- and any case
  // variation a producer introduces -- resolve to the same element.
  std::string key = EndpointFoldedKey(endpoint);
  if (key.empty())
    return -1;
  auto it = _nameToUid.find(key);
  if (it != _nameToUid.end())
    return it->second;
  return -1;
}

void XmileView::ProcessViewGroups(tinyxml2::XMLElement *viewEl, std::vector<std::string> &errs) {
  // The uids of this view's <alias> elements. An <item uid="N"/> naming one
  // places a GHOST inside the group's rectangle, which says where a REFERENCE
  // to the variable is drawn -- the variable itself lives wherever its real
  // element sits, possibly in another group entirely. A variable belongs to one
  // group (XmileReader::AddGroupMember, first claim wins), so letting a ghost
  // claim it ahead of its real placement decides the answer by document order:
  // in test/fixtures/simlin-test/land_model/land_model.stmx every <item> in
  // every group resolves to a ghost, and the losing claims were reported as
  // membership conflicts even though nothing contradicted anything. Ghost
  // claims are therefore held back until every real placement in this view has
  // been settled, and a held-back claim for an already-placed variable is
  // dropped quietly -- it never asserted a membership to begin with.
  std::set<int> ghostUids;
  for (tinyxml2::XMLElement *alias = viewEl->FirstChildElement("alias"); alias;
       alias = alias->NextSiblingElement("alias")) {
    if (const char *uidStr = alias->Attribute("uid"))
      ghostUids.insert(std::atoi(uidStr));
  }

  std::vector<std::pair<ModelGroup *, Variable *>> ghostClaims;
  for (tinyxml2::XMLElement *child = viewEl->FirstChildElement("group"); child;
       child = child->NextSiblingElement("group")) {
    // Two distinct child shapes appear in the wild:
    //   <item uid="N"/>   - Stella's view-layout-group form (members named
    //                       by the per-view XMILE uid attribute)
    //   <var>name</var>   - the xmutil writer's group-emission form (members
    //                       named by display name, no uid attribute)
    // Either may appear; the writer mainly emits the <var> form, while
    // hand-authored Stella exports use <item>. Handle <var> by delegating to
    // the reader's existing ProcessGroup helper so the member-by-name
    // resolution path stays single-sourced. Then walk the <item> children
    // separately so a mixed group (rare but legal) still attaches both kinds
    // of members.
    ModelGroup *group = nullptr;
    if (child->FirstChildElement("var")) {
      group = _reader->ProcessGroup(child, errs);
    } else {
      const char *name = child->Attribute("name");
      if (!name) {
        errs.push_back("<group> with no name attribute");
        continue;
      }
      // Registered even when the element has no children at all: a childless
      // <group name="X" owner="Y"/> still states a nesting, and the identical
      // element directly under <views> does register (ProcessViews delegates
      // it to ProcessGroup). Where the element sits should not change what it
      // means. Find-or-create, because a same-named group from <views> or from
      // a sibling <view> may already live in the Model.
      group = _reader->RegisterGroup(XmileReader::NormalizeName(name), child->Attribute("owner"), errs);
    }
    if (!group)
      continue;
    for (tinyxml2::XMLElement *item = child->FirstChildElement("item"); item; item = item->NextSiblingElement("item")) {
      const char *uidStr = item->Attribute("uid");
      if (!uidStr)
        continue;
      int xmileUid = std::atoi(uidStr);
      auto it = _xmileUidToOurUid.find(xmileUid);
      if (it == _xmileUidToOurUid.end())
        continue;
      VensimViewElement *e = _view->Elements()[it->second];
      VensimVariableElement *ve = dynamic_cast<VensimVariableElement *>(e);
      if (!ve)
        continue;
      Variable *v = ve->GetVariable();
      if (!v)
        continue;
      if (ghostUids.find(xmileUid) != ghostUids.end())
        ghostClaims.emplace_back(group, v);
      else
        _reader->AddGroupMember(group, v, errs);
    }
  }

  // A ghost claim is worth honoring only for a variable no real placement
  // spoke for -- typically one drawn only as ghosts in this view because its
  // definition lives in a view this reader does not process.
  for (const std::pair<ModelGroup *, Variable *> &claim : ghostClaims) {
    if (claim.second->GetGroup())
      continue;
    _reader->AddGroupMember(claim.first, claim.second, errs);
  }
}
