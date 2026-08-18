#include "XmileReader.h"

#include <tinyxml2.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <set>
#include <stdexcept>

#include "../Mdl/MDLFormat.h"
#include "../Model.h"
#include "../Symbol/Expression.h"
#include "../Symbol/ExpressionList.h"
#include "../Symbol/LeftHandSide.h"
#include "../Symbol/Symbol.h"
#include "../Symbol/SymbolList.h"
#include "../Symbol/SymbolNameSpace.h"
#include "../Symbol/UnitExpression.h"
#include "../Symbol/Units.h"
#include "../Symbol/Variable.h"
#include "../Vensim/VensimParse.h"
#include "../Vensim/VensimView.h"
#include "../XMUtil.h"
#include "XmileEqLex.h"
#include "XmileView.h"

// The bison-generated parser entry. The `-p xpyy` rename produces this name
// in XmileEqYacc.tab.cpp; declaring it here lets us avoid including the
// generated header (which would also export YYSTYPE / xpyylval globals that
// callers of XmileReader.h shouldn't see). The generated parser is compiled
// as C++, so no extern "C" wrapping.
int xpyyparse(void);

XmileReader *XPObject = nullptr;

namespace {

// The canonical Vensim spellings of the four control variables, indexed by
// XmileReader::ControlIndex. These are the names <sim_specs> registers, the
// names MDLGenerator::GenerateControl looks up, and the names the .Control
// group is written from.
const char *const kControlNames[] = {"INITIAL TIME", "FINAL TIME", "TIME STEP", "SAVEPER"};

// The numeric constant a Variable's defining equation holds, or false when it
// has no equation or the equation is anything else. Mirrors what
// Model::GetConstanValue sees through: a control variable's value only reaches
// the engine's own fields, and the XMILE writer's <start>/<stop>/<dt>, if it is
// a bare number.
bool ConstantValueOf(Variable *v, double *out) {
  if (!v || v->GetAllEquations().empty())
    return false;
  Equation *eq = v->GetEquation(0);
  Expression *e = eq ? eq->GetExpression() : nullptr;
  if (!e || e->GetType() != EXPTYPE_Number)
    return false;
  *out = e->Eval(NULL);
  return true;
}

// Read text as one complete numeric literal, writing it to *out. strtod skips
// leading whitespace and trailing whitespace is tolerated, but anything else
// left over -- an operator, a second token, an identifier -- means the body is
// an expression rather than a constant. "nan"/"inf" parse but are rejected:
// they are not usable as a control value.
bool ParseNumericLiteral(const char *text, double *out) {
  if (!text)
    return false;
  char *endp = nullptr;
  const double v = std::strtod(text, &endp);
  if (endp == text)
    return false;
  while (*endp && std::isspace(static_cast<unsigned char>(*endp)))
    ++endp;
  if (*endp || !std::isfinite(v))
    return false;
  *out = v;
  return true;
}

// Build the "<tag name="X">: " prefix that attributes an equation-level
// diagnostic to the variable element it came from.
std::string ElementContext(tinyxml2::XMLElement *el) {
  const char *tag = el ? el->Name() : nullptr;
  const char *nm = el ? el->Attribute("name") : nullptr;
  return std::string("<") + (tag ? tag : "?") + " name=\"" + (nm ? nm : "(?)") + "\">: ";
}

// Forward every diagnostic ParseEquation collected up to the document errs
// channel, tagged with the offending element, and report whether any was a hard
// error. Advisory diagnostics carry a "warning: " prefix (see the arity note in
// XmileParseFunctions and the non_negative advisory); anything else -- an
// unknown function lowered to a 0 placeholder, an unresolvable identifier -- is
// a wrongness that must fail the conversion even though ParseEquation returned a
// (placeholder) Expression. Without this classification those diagnostics were
// discarded whenever the placeholder-returning shims let the bison parse
// succeed, silently shipping a semantically wrong model.
bool ForwardEqnDiagnostics(const std::string &context, const std::vector<std::string> &eqnErrs,
                           std::vector<std::string> &errs) {
  bool hardError = false;
  for (const std::string &e : eqnErrs) {
    errs.push_back(context + e);
    if (e.compare(0, 9, "warning: ") != 0)
      hardError = true;
  }
  return hardError;
}

// Walk an expression tree, recording the names of ExpressionLookup targets --
// the `table` in `table(x)` -- whose Variable ended the parse with no defining
// equation. The tree shape mirrors Model::ResolveWildcardsInExpr. WITH LOOKUP
// nodes (an inline <gf> table, GetLookupVariable() == NULL) are always defined
// and skipped. Names accumulate into a set so a phantom referenced from several
// equations is reported once.
void CollectPhantomLookups(Expression *e, std::set<std::string> &phantoms) {
  if (!e)
    return;
  switch (e->GetType()) {
  case EXPTYPE_Lookup: {
    ExpressionLookup *lk = static_cast<ExpressionLookup *>(e);
    if (ExpressionVariable *ev = lk->GetLookupVariable()) {
      Variable *target = ev->GetVariable();
      if (target && target->GetAllEquations().empty() && target->GetAllInitEquations().empty())
        phantoms.insert(target->GetName());
    }
    CollectPhantomLookups(lk->GetInput(), phantoms);
    break;
  }
  case EXPTYPE_Operator:
    CollectPhantomLookups(e->GetArg(0), phantoms);
    CollectPhantomLookups(e->GetArg(1), phantoms);
    break;
  case EXPTYPE_Logical: {
    ExpressionLogical *lg = static_cast<ExpressionLogical *>(e);
    CollectPhantomLookups(lg->GetLeft(), phantoms);
    CollectPhantomLookups(lg->GetRight(), phantoms);
    break;
  }
  case EXPTYPE_Function:
  case EXPTYPE_FunctionMemory: {
    ExpressionList *args = static_cast<ExpressionFunction *>(e)->GetArgs();
    if (args)
      for (int i = 0; i < args->Length(); i++)
        CollectPhantomLookups(args->GetExp(i), phantoms);
    break;
  }
  default:
    // Number, literal, table, symlist, bare variable: no nested lookup target.
    break;
  }
}

}  // namespace

XmileReader::XmileReader(Model *model)
    : _model(model),
      pSymbolNameSpace(model->GetNameSpace()),
      _haveTimeUnits(false),
      _lastParsedExpr(nullptr),
      _currentLex(nullptr),
      _currentErrs(nullptr),
      _declaresPi(false) {
  // The bison action shims see the active reader only through this global, so
  // exactly one reader may be live. An `assert` alone compiled out under
  // NDEBUG -- which is the build an embedder parsing two documents from nested
  // scopes would be running -- and there the second reader silently took the
  // global over. Refuse at runtime instead: the extern-C entries catch this and
  // return NULL, which is a diagnosable failure rather than a wrong model.
  if (XPObject)
    throw std::runtime_error("XmileReader: another XMILE parse is already active in this process");
  XPObject = this;
  // ParseEquation and ProcessStock both look up xmutil Functions by name
  // (INTEG, IF THEN ELSE, ZIDZ, ...). Register the shared table here so the
  // reader is self-contained -- no need for the caller to construct a
  // transient VensimParse to seed the namespace. Guard against
  // double-registration in case the same namespace already has the table
  // (e.g. an in-process pipeline that ran VensimParse first).
  if (!pSymbolNameSpace->Find("INTEG"))
    RegisterXmutilFunctions(pSymbolNameSpace);

  // Seed the control values from the Model's own fields so ApplyControlValues
  // has something coherent to fall back on even for a document that carries no
  // <sim_specs> at all. ProcessSimSpecs overwrites all four the moment it runs.
  const double seeds[kControlCount] = {model->initial_time(), model->final_time(), model->dt(), model->dt()};
  for (int i = 0; i < kControlCount; i++) {
    _controls[i].var = nullptr;
    _controls[i].value = seeds[i];
    _controls[i].stated = false;
    _controls[i].pendingUnitsAndDoc = nullptr;
  }
}

XmileReader::~XmileReader() {
  // Only the reader that claimed the global may clear it. Clearing it
  // unconditionally is what turned a stolen global into silent corruption: the
  // thief's destructor left the still-live owner pointing at nothing, and every
  // shim it called afterwards took its defensive null branch and returned a
  // 0-placeholder without a diagnostic.
  if (XPObject == this)
    XPObject = nullptr;
}

bool XmileReader::ProcessFile(const std::string &filename, const char *contents, size_t len,
                              std::vector<std::string> &errs) {
  tinyxml2::XMLDocument doc;
  // tinyxml2::XMLDocument::Parse accepts a length, so the buffer need not be
  // NUL-terminated -- the extern-C path passes a uint32_t length.
  tinyxml2::XMLError parseErr = doc.Parse(contents, len);
  if (parseErr != tinyxml2::XML_SUCCESS) {
    const char *detail = doc.ErrorStr();
    errs.push_back(filename + ": XML parse error: " + (detail ? detail : "(no detail)"));
    return false;
  }
  tinyxml2::XMLElement *root = doc.RootElement();
  if (!root) {
    errs.push_back(filename + ": empty XML document");
    return false;
  }
  // The bare local name "xmile" is the only accepted root. A vendor-prefixed
  // root like "isee:xmile" is not the XMILE envelope.
  const char *rootName = root->Name();
  if (!rootName || std::string(rootName) != "xmile") {
    errs.push_back(filename + ": root element is not <xmile> (got <" + (rootName ? rootName : "(null)") + ">)");
    return false;
  }
  // Envelope-level pre-pass: reject <macro> outright, refuse documents that
  // carry more than one <model> sibling, and collect the <sim_specs> elements.
  // Counting <model> siblings here -- rather than inside the dispatch loop
  // below -- means a multi-model document fails before any partial state is
  // built on the reader's Model, which keeps the error path symmetric with the
  // empty-input and malformed-XML returns above.
  int modelCount = 0;
  std::vector<tinyxml2::XMLElement *> simSpecsEls;
  for (tinyxml2::XMLElement *child = root->FirstChildElement(); child; child = child->NextSiblingElement()) {
    const char *name = child->Name();
    if (!name)
      continue;
    if (IsForeignNamespace(name))
      continue;
    std::string tag(name);
    if (tag == "macro") {
      errs.push_back(filename + ": <macro> elements are not supported");
      return false;
    }
    if (tag == "model")
      ++modelCount;
    else if (tag == "sim_specs")
      simSpecsEls.push_back(child);
  }
  if (modelCount > 1) {
    errs.push_back(filename + ": multiple <model> elements are not supported");
    return false;
  }
  // <sim_specs> is dispatched ahead of the main loop rather than in document
  // order. XMILE fixes no order among the envelope's children, and <variables>
  // may declare any of the four control names as an ordinary <aux> -- so unless
  // the sim specs are known first, whether the declaration or <sim_specs> wins
  // depends on which element the document happens to put first. Hoisting makes
  // the control values settled before any declaration is walked, which is what
  // lets ProcessControlDeclaration state one rule for both orders.
  bool ok = true;
  for (tinyxml2::XMLElement *simSpecs : simSpecsEls) {
    if (!ProcessSimSpecs(simSpecs, errs)) {
      ok = false;
      break;
    }
  }
  for (tinyxml2::XMLElement *child = root->FirstChildElement(); ok && child; child = child->NextSiblingElement()) {
    const char *name = child->Name();
    if (!name)
      continue;
    if (IsForeignNamespace(name))
      continue;
    std::string tag(name);
    if (tag == "header") {
      // The Model has no field that stores XMILE header metadata (name,
      // vendor, product); acknowledge the element so it doesn't fall into the
      // unknown bucket below.
      continue;
    } else if (tag == "sim_specs") {
      continue;  // already dispatched by the pre-pass above
    } else if (tag == "model_units") {
      ok = ProcessModelUnits(child, errs);
    } else if (tag == "dimensions") {
      // XMILEGenerator emits the model's <dimensions> at root scope (sibling
      // to <model>), not inside <model>. The reader also accepts a
      // <dimensions> child of <model> (handled in ProcessModel) for fixtures
      // and writers that place it there; both forms feed the same
      // ProcessDimensions handler.
      ok = ProcessDimensions(child, errs);
    } else if (tag == "model") {
      ok = ProcessModel(child, errs);
    } else if (tag == "style") {
      // Styling sections carry no semantic content.
      continue;
    } else if (IsStellaUIWidget(name)) {
      continue;
    } else {
      // Unknown default-namespace envelope element. Drop silently to leave the
      // door open for future XMILE additions; tighten to a hard error only if
      // the corpus demands it.
      continue;
    }
  }
  // Whole-document epilogue. Every step needs the full document in hand: a
  // group's owner="..." may name a group declared later, a control value
  // <sim_specs> left unstated may have been supplied by a <variables>
  // declaration, the document-level time_units only defaults the control
  // variables that did not spell out their own <units>, and a graphical-function
  // target may be forward-referenced, so a lookup application can only be judged
  // phantom once every <gf> has been walked.
  if (ok)
    ResolveGroupOwners(errs);
  if (ok)
    ApplyControlValues();
  if (ok)
    ApplyDeferredTimeUnits();
  if (ok && !ValidateLookupTargets(errs))
    ok = false;
  return ok;
}

bool XmileReader::IsForeignNamespace(const char *qualifiedName) {
  if (!qualifiedName)
    return false;
  return std::strchr(qualifiedName, ':') != nullptr;
}

bool XmileReader::IsStellaUIWidget(const char *unprefixedName) {
  if (!unprefixedName)
    return false;
  // Documented Stella UI widget tags plus the corpus-observed loop indicator.
  // Kept as an alphabetized static array for readability; the list is short
  // enough that a linear scan is faster than any hashed structure.
  static const char *const kWidgets[] = {
      "animation_object", "button",          "gauge",         "graph",  "knob",
      "loop_indicator",   "numeric_display", "numeric_input", "slider", "spatial_map",
  };
  for (const char *w : kWidgets) {
    if (std::strcmp(unprefixedName, w) == 0)
      return true;
  }
  return false;
}

std::string XmileReader::NormalizeName(const char *raw) {
  if (!raw)
    return std::string();
  std::string out;
  out.reserve(std::strlen(raw));
  bool inSpace = false;
  bool seenNonSpace = false;
  for (const char *p = raw; *p; ++p) {
    unsigned char c = static_cast<unsigned char>(*p);
    // Stella encodes visual line-breaks in display names as the literal 2-byte
    // sequence "\n" (backslash + n) -- and occasionally "\r" -- to instruct the
    // diagram renderer to wrap the label across multiple lines while keeping
    // the underlying identifier single-line. Vensim identifiers carry no such
    // marker (and the namespace's ToLowerSpace canonicalizer treats `_` and
    // real whitespace as equivalent but does NOT recognize literal `\n`), so a
    // model that declared an aux as `maximum_fishery_size` in <variables> and
    // then re-referenced it as `Maximum\nfishery size` in the <view> would
    // hash to two distinct names and produce a phantom duplicate variable.
    // Collapse the escape into a single space at the read boundary so all
    // downstream lookups see one canonical form. Mirrors the XMILE 1.0
    // convention; simlin's reference reader does the same.
    if (c == '\\' && (p[1] == 'n' || p[1] == 'r')) {
      ++p;
      inSpace = true;
      continue;
    }
    if (std::isspace(c)) {
      inSpace = true;
      continue;
    }
    if (inSpace && seenNonSpace)
      out.push_back(' ');
    out.push_back(static_cast<char>(c));
    seenNonSpace = true;
    inSpace = false;
  }
  return out;
}

std::string XmileReader::FoldNameKey(const std::string &normalizedName) {
  std::string key;
  key.reserve(normalizedName.size());
  for (char c : normalizedName) {
    unsigned char uc = static_cast<unsigned char>(c);
    if (uc == '_' || uc == ' ') {
      if (!key.empty() && key.back() != ' ')
        key.push_back(' ');
    } else {
      key.push_back(static_cast<char>(std::tolower(uc)));
    }
  }
  while (!key.empty() && key.back() == ' ')
    key.pop_back();
  return key;
}

const std::vector<std::string> *XmileReader::StocksForFlow(const std::string &foldedFlowName) const {
  auto it = _flowToStocks.find(foldedFlowName);
  if (it == _flowToStocks.end())
    return nullptr;
  return &it->second;
}

UnitExpression *XmileReader::ParseUnitsString(const std::string &text) {
  // A body that is empty once trimmed states no unit, which is not the same as
  // stating a malformed one -- it gets no UnitExpression rather than a bad one.
  size_t start = 0;
  while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start])))
    ++start;
  size_t end = text.size();
  while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])))
    --end;
  if (start >= end)
    return nullptr;

  // Recursive descent parser for unit expressions. Grammar:
  //
  //   expr  := factor (('/' | '*') factor)*
  //   factor := '(' expr ')' | ident
  //
  // Precedence and associativity match VensimParse: left-associative, '/'
  // and '*' at the same level. The writer emits "a/(b*c)" for a unit with
  // numerator a and denominator [b, c]; the '(' group must be parsed atomically
  // so that "a/(b*c)" produces denominator b*c, not a mis-parsed "a/b * c".
  //
  // p is a reference into the string, advanced in place by each method. Both
  // productions skip leading whitespace before consuming. On malformed input
  // (unmatched parens, empty token) they return nullptr -- deleting any
  // partially-built operand first -- and the caller falls back to storing
  // only the raw text string.
  struct Parser {
    SymbolNameSpace *sns;
    const char *sEnd;

    void SkipSpace(const char *&p) const {
      while (p < sEnd && std::isspace(static_cast<unsigned char>(*p)))
        ++p;
    }

    // Find-or-create a Units symbol under the ">name" key the Vensim parser
    // uses for the units namespace.
    Units *InsertUnits(const std::string &name) const {
      std::string key = ">" + name;
      Symbol *sym = sns->Find(key);
      if (sym && sym->isType() == Symtype_Units)
        return static_cast<Units *>(sym);
      return new Units(sns, key);
    }

    UnitExpression *ParseFactor(const char *&p) const {
      SkipSpace(p);
      if (p >= sEnd)
        return nullptr;

      if (*p == '(') {
        ++p;
        UnitExpression *inner = ParseExpr(p);
        if (!inner)
          return nullptr;
        SkipSpace(p);
        if (p >= sEnd || *p != ')') {
          delete inner;
          return nullptr;
        }
        ++p;
        return inner;
      }

      // Identifier: everything up to the next operator, paren, or end.
      // Whitespace INSIDE the token is preserved ("kg of stuff" is one name),
      // but trailing whitespace is trimmed before returning.
      const char *tokStart = p;
      while (p < sEnd && *p != '/' && *p != '*' && *p != '(' && *p != ')')
        ++p;
      const char *tokEnd = p;
      while (tokEnd > tokStart && std::isspace(static_cast<unsigned char>(*(tokEnd - 1))))
        --tokEnd;
      if (tokEnd == tokStart)
        return nullptr;
      std::string name(tokStart, tokEnd);
      return new UnitExpression(sns, InsertUnits(name));
    }

    UnitExpression *ParseExpr(const char *&p) const {
      UnitExpression *acc = ParseFactor(p);
      if (!acc)
        return nullptr;
      for (;;) {
        SkipSpace(p);
        if (p >= sEnd || (*p != '/' && *p != '*'))
          break;
        bool isDiv = (*p == '/');
        ++p;
        UnitExpression *rhs = ParseFactor(p);
        if (!rhs) {
          delete acc;
          return nullptr;
        }
        acc = isDiv ? acc->Divide(rhs) : acc->Multiply(rhs);
      }
      return acc;
    }
  };

  const Parser parser{pSymbolNameSpace, text.c_str() + end};
  const char *pos = text.c_str() + start;
  UnitExpression *result = parser.ParseExpr(pos);
  // Reject if unconsumed input remains after the parse (e.g. an unmatched ')').
  parser.SkipSpace(pos);
  if (pos != parser.sEnd) {
    delete result;
    return nullptr;
  }
  return result;
}

void XmileReader::AttachVariableUnits(Variable *v, const char *unitsText) {
  if (!v || !unitsText)
    return;
  // First source wins, and it wins for both the raw string and the parsed
  // expression. Variable::AddUnits keeps whichever UnitExpression arrived first
  // and offers no replace, so writing the raw string unconditionally (as this
  // used to) let the two halves name different sources; both writers read the
  // parsed half first (MDLGenerator::UnitsCommentTrailer,
  // XMILEGenerator::generateSimSpecs), so the raw text was the half that got
  // silently dropped. Gating both on one test keeps them in agreement no matter
  // how many times a Variable is visited, and leaves the call order as the
  // single place precedence is decided.
  if (!v->GetUnitsString().empty() || v->Units())
    return;
  v->SetUnitsString(unitsText);
  UnitExpression *ue = ParseUnitsString(unitsText);
  if (ue) {
    // The guard above already established that no UnitExpression is attached,
    // so this hands ownership over; the delete is belt-and-braces against a
    // future Variable that reports no units yet still refuses the add.
    if (!v->AddUnits(ue))
      delete ue;
  }
}

void XmileReader::ApplyDeferredTimeUnits() {
  if (!_haveTimeUnits)
    return;
  // Running after the whole document has been walked is what makes a control
  // variable's own <units> element beat the document-wide default:
  // AttachVariableUnits is a no-op on a Variable that already carries units.
  for (const ControlVar &c : _controls)
    AttachVariableUnits(c.var, _timeUnits.c_str());
}

bool XmileReader::ProcessSimSpecs(tinyxml2::XMLElement *simSpecs, std::vector<std::string> &errs) {
  // method= is case-insensitive across the corpus (Stella emits "euler",
  // xmutil emits "Euler"). Missing or unrecognized values fall back to Euler;
  // RK4/RK2 are the only other XMILE-defined integrators.
  Integration_Type integration = Integration_Type_EULER;
  if (const char *method = simSpecs->Attribute("method")) {
    if (StringMatch(method, "rk4"))
      integration = Integration_Type_RK4;
    else if (StringMatch(method, "rk2"))
      integration = Integration_Type_RK2;
  }
  _model->SetIntegrationType(integration);

  // The XMILE writer falls back to 0/100/1 defaults when the corresponding
  // control variables are absent (XMILEGenerator::generateSimSpecs), so the
  // reader mirrors those defaults for missing child elements. SAVEPER
  // defaults to dt per the XMILE spec.
  //
  // Whether each value was SPELLED OUT is tracked alongside it: a stand-in for
  // an absent child is the reader's own invention, and must lose to a
  // <variables> declaration of the same control that does state a value (see
  // ProcessControlDeclaration). An element that is present but holds no usable
  // number still counts as stated -- the document addressed the value, badly --
  // which keeps the resulting control identical to what it has always been.
  double startVal = 0.0;
  double stopVal = 100.0;
  double dtVal = 1.0;
  bool haveStart = false;
  bool haveStop = false;
  bool haveDt = false;

  if (tinyxml2::XMLElement *e = simSpecs->FirstChildElement("start")) {
    startVal = e->DoubleText(0.0);
    haveStart = true;
  }
  if (tinyxml2::XMLElement *e = simSpecs->FirstChildElement("stop")) {
    stopVal = e->DoubleText(100.0);
    haveStop = true;
  }
  if (tinyxml2::XMLElement *e = simSpecs->FirstChildElement("dt")) {
    dtVal = e->DoubleText(1.0);
    haveDt = true;
    // dt reciprocal="true" means the body is N and the actual dt is 1/N. The
    // xmutil writer never emits reciprocal form (its dt is always the resolved
    // double), so a self-round-trip never sees it -- but Stella XMILE does,
    // and the reader must interoperate. Resolve at read time so all downstream
    // code sees a plain dt.
    if (const char *recip = e->Attribute("reciprocal")) {
      std::string r(recip);
      if (r == "true" || r == "1") {
        if (dtVal != 0.0)
          dtVal = 1.0 / dtVal;
        else
          errs.push_back("<dt reciprocal=\"true\"> with zero body; ignoring reciprocal");
      }
    }
    // A dt of zero, or one that parsed to a non-finite value ("%lf" accepts
    // "nan" and "inf"), makes the model unsimulable AND leaks into SAVEPER
    // through the default below, so it is called out here rather than only
    // where an explicit save interval is checked. The value is reported, not
    // replaced: substituting the writer's 1.0 default would silently ship a
    // different model than the document describes. <start>/<stop> get no
    // equivalent check because every finite value is legal there (negative,
    // zero, and descending ranges are all representable, and the writer already
    // repairs stop <= start) and neither feeds another field's default.
    if (!std::isfinite(dtVal) || dtVal <= 0.0)
      errs.push_back(
          "<dt> is not a finite positive number; TIME STEP -- and the SAVEPER that defaults to it -- is "
          "unusable as written");
  }
  // The save interval has two spellings in the wild and the reader accepts
  // both. XMILE 1.0 defines no save-interval property on <sim_specs> at all
  // (export cadence lives in the <data> section), so neither is standard: the
  // isee:save_interval attribute is the isee vendor extension that Stella,
  // simlin, and XMILEGenerator all actually emit, while the <save_step> child
  // element is the explicit spelling this reader has accepted since it was
  // written. When a document carries both -- a contradiction no writer
  // produces -- the element wins: it is the unprefixed, self-describing form,
  // and a vendor attribute is the weaker claim. tinyxml2 does no namespace
  // processing, so the attribute's literal name carries the "isee:" prefix.
  //
  // A value that is unparseable, non-finite, or non-positive is not a cadence
  // at all, so it is reported and the next-best source is used rather than
  // being stored as-is. The guard stops there: any finite positive interval is
  // the modeler's own and is preserved verbatim, however small. It is
  // deliberately NOT a sanity filter for the writer's isee:sim_duration (which
  // divides the run length by SAVEPER) -- that quotient guards itself, because
  // SAVEPER can also reach the writer as an unchecked dt via the default below.
  double saveStepVal = dtVal;
  auto takeSaveInterval = [&](tinyxml2::XMLError rc, double v, const char *spelling) {
    if (rc == tinyxml2::XML_NO_TEXT_NODE) {
      // An empty element (<save_step/>) expressed no interval at all, which is
      // a different thing from expressing a malformed one.
      errs.push_back(std::string(spelling) + " is empty; SAVEPER falls back to dt");
      return false;
    }
    if (rc != tinyxml2::XML_SUCCESS) {
      errs.push_back(std::string(spelling) + " is not a number; SAVEPER falls back to dt");
      return false;
    }
    // isfinite before the sign test: tinyxml2 converts through sscanf("%lf"),
    // which glibc happily fills with a NaN for "nan" -- and every comparison
    // against a NaN is false, so `v <= 0.0` alone would wave it through.
    if (!std::isfinite(v)) {
      errs.push_back(std::string(spelling) + " is not a finite number; SAVEPER falls back to dt");
      return false;
    }
    if (v <= 0.0) {
      errs.push_back(std::string(spelling) + " is not positive; SAVEPER falls back to dt");
      return false;
    }
    saveStepVal = v;
    return true;
  };
  bool haveSaveInterval = false;
  if (tinyxml2::XMLElement *e = simSpecs->FirstChildElement("save_step")) {
    double v = 0.0;
    // rc is sequenced before the call: the query's out-parameter must be
    // written before takeSaveInterval's arguments are evaluated.
    tinyxml2::XMLError rc = e->QueryDoubleText(&v);
    haveSaveInterval = takeSaveInterval(rc, v, "<save_step>");
  }
  if (!haveSaveInterval) {
    double v = 0.0;
    tinyxml2::XMLError rc = simSpecs->QueryDoubleAttribute("isee:save_interval", &v);
    if (rc != tinyxml2::XML_NO_ATTRIBUTE)
      haveSaveInterval = takeSaveInterval(rc, v, "isee:save_interval");
  }

  // Populate both the control Variables (for the writer's GetConstanValue
  // path) and the Model setters (for the engine's compiled-in fast path).
  // Mirrors how VensimParse drops out of its Control section. The equations
  // land here only for the values the document stated; ApplyControlValues fills
  // in the rest once the whole document has been walked.
  const double values[kControlCount] = {startVal, stopVal, dtVal, saveStepVal};
  const bool stated[kControlCount] = {haveStart, haveStop, haveDt, haveSaveInterval};
  for (int i = 0; i < kControlCount; i++) {
    _controls[i].var = SetControlVariable(kControlNames[i], values[i], stated[i]);
    _controls[i].value = values[i];
    _controls[i].stated = stated[i];
  }
  _model->set_initial_time(startVal);
  _model->set_finall_time(stopVal);  // misspelled in Model.h; mirrors header
  _model->set_dt(dtVal);

  // time_units is the model's unit of time, so it is the default for every
  // control variable -- which is also how a Vensim .Control group spells it,
  // and what the .mdl writer re-emits from these variables. It must go through
  // AttachVariableUnits, not a bare SetUnitsString: the XMILE writer reads the
  // unit back through Model::GetUnits (TIME STEP -> FINAL TIME -> INITIAL
  // TIME), which returns the PARSED UnitExpression. With only the raw string
  // stored, GetUnits returned NULL and every non-default time unit was
  // rewritten to the writer's hardcoded "Months" default on each pass.
  //
  // Only recorded here, applied by ApplyDeferredTimeUnits once the document has
  // been fully walked: a control variable that spells out its own <units> is
  // making the more specific claim and must win, and that comparison is only
  // meaningful after <model> has been read. SetControlVariable returned each
  // control Variable into _controls[i].var, so no entry is null unless the name
  // is occupied by a non-Variable symbol -- in which case there is nothing to
  // attach units to.
  if (const char *tu = simSpecs->Attribute("time_units")) {
    _timeUnits = tu;
    _haveTimeUnits = true;
  }

  return true;
}

bool XmileReader::ProcessModelUnits(tinyxml2::XMLElement *units, std::vector<std::string> &errs) {
  (void)errs;
  // A UnitEquiv keeps <eqn> and <alias> apart, so a derived-unit formula stays a
  // formula on the way back out; only the .mdl writer has to flatten the two
  // into one comma-separated "22:" list, and it does that at emit time
  // (UnitEquiv::MdlPayload), which is also where the field sanitizing that
  // protects the "22:" line lives.
  for (tinyxml2::XMLElement *u = units->FirstChildElement("unit"); u; u = u->NextSiblingElement("unit")) {
    const char *uname = u->Attribute("name");
    if (!uname)
      continue;
    UnitEquiv equiv;
    equiv.name = uname;
    if (tinyxml2::XMLElement *eqnEl = u->FirstChildElement("eqn")) {
      // A present-but-empty <eqn/> -- which real Stella exports carry -- states
      // no formula, so it stays empty rather than becoming a blank field that
      // both writers would then have to render as something.
      if (const char *txt = eqnEl->GetText())
        equiv.eqn = txt;
    }
    for (tinyxml2::XMLElement *aliasEl = u->FirstChildElement("alias"); aliasEl;
         aliasEl = aliasEl->NextSiblingElement("alias")) {
      if (const char *txt = aliasEl->GetText())
        equiv.aliases.push_back(txt);
    }
    _model->UnitEquivs().push_back(equiv);
  }
  return true;
}

ModelGroup *XmileReader::ProcessGroup(tinyxml2::XMLElement *groupEl, std::vector<std::string> &errs) {
  const char *name = groupEl->Attribute("name");
  if (!name) {
    errs.push_back("<group> with no name attribute");
    return nullptr;
  }
  std::string normName = NormalizeName(name);
  ModelGroup *group = RegisterGroup(normName, groupEl->Attribute("owner"), errs);

  for (tinyxml2::XMLElement *vEl = groupEl->FirstChildElement("var"); vEl; vEl = vEl->NextSiblingElement("var")) {
    const char *vname = vEl->GetText();
    if (!vname)
      continue;
    std::string n = NormalizeName(vname);
    Variable *v = FindVariable(n);
    if (!v) {
      // Forward reference to a variable declared later in the file (or never
      // declared explicitly). InsertVariable creates a placeholder so the
      // group membership round-trips; MarkVariableTypes classifies it.
      v = InsertVariable(n);
      if (!v)
        continue;
    }
    AddGroupMember(group, v, errs);
  }
  return group;
}

ModelGroup *XmileReader::RegisterGroup(const std::string &normName, const char *ownerAttr,
                                       std::vector<std::string> &errs) {
  // The same group can appear under multiple <view> elements in a multi-view
  // file; reusing keeps Model::Groups() unique by name.
  ModelGroup *group = FindGroupByName(normName);
  if (!group) {
    group = new ModelGroup(normName, /*owner=*/nullptr);
    _model->Groups().push_back(group);
  }
  if (!ownerAttr)
    return group;
  const std::string ownerName = NormalizeName(ownerAttr);
  if (ownerName.empty())
    return group;
  // Held rather than resolved -- see ResolveGroupOwners for why the lookup
  // cannot happen here. A repeated <group> element re-states the same claim;
  // only a claim that names a DIFFERENT owner is a contradiction, and the first
  // one wins, matching how the reader settles every other doubly-stated fact.
  for (const std::pair<ModelGroup *, std::string> &claim : _pendingGroupOwners) {
    if (claim.first != group)
      continue;
    if (claim.second != ownerName)
      errs.push_back("<group name=\"" + normName + "\">: owner=\"" + ownerName + "\" contradicts the earlier owner=\"" +
                     claim.second + "\"; keeping the first");
    return group;
  }
  _pendingGroupOwners.emplace_back(group, ownerName);
  return group;
}

void XmileReader::AddGroupMember(ModelGroup *group, Variable *v, std::vector<std::string> &errs) {
  if (!group || !v)
    return;
  ModelGroup *current = v->GetGroup();
  if (current == group)
    return;  // the same group listed under two views, or a repeated <var>
  if (current) {
    errs.push_back("<group name=\"" + group->sName + "\">: '" + v->GetName() + "' already belongs to group '" +
                   current->sName + "'; keeping the first grouping");
    return;
  }
  v->SetGroup(group);
  group->vVariables.push_back(v);
}

void XmileReader::ResolveGroupOwners(std::vector<std::string> &errs) {
  for (const std::pair<ModelGroup *, std::string> &claim : _pendingGroupOwners) {
    ModelGroup *group = claim.first;
    const std::string &ownerName = claim.second;
    ModelGroup *owner = FindGroupByName(ownerName);
    if (!owner) {
      errs.push_back("<group name=\"" + group->sName + "\">: owner=\"" + ownerName +
                     "\" names no group in this document; the group is left unowned");
      continue;
    }
    if (owner == group) {
      errs.push_back("<group name=\"" + group->sName + "\">: a group cannot own itself; the group is left unowned");
      continue;
    }
    // Every pOwner link is set right here, and only after this walk has cleared
    // it, so the chain being walked is acyclic by construction -- which both
    // bounds the loop and is what the writers require. Refusing the link that
    // would close the cycle (rather than the whole chain) keeps as much of the
    // stated nesting as can be honored.
    bool cycles = false;
    for (ModelGroup *up = owner->pOwner; up; up = up->pOwner) {
      if (up == group) {
        cycles = true;
        break;
      }
    }
    if (cycles) {
      errs.push_back("<group name=\"" + group->sName + "\">: owner=\"" + ownerName +
                     "\" would close a group ownership cycle; the group is left unowned");
      continue;
    }
    group->pOwner = owner;
  }
  _pendingGroupOwners.clear();
}

ModelGroup *XmileReader::FindGroupByName(const std::string &norm) {
  for (ModelGroup *g : _model->Groups()) {
    if (g->sName == norm)
      return g;
  }
  return nullptr;
}

// True if a <view> element has any sketch-geometry children (stocks, auxes,
// flows, aliases, or connectors). A <view> whose only children are <group>
// elements carries no sketch geometry; in that case the reader processes the
// groups in-place via ProcessGroup and does NOT allocate a VensimView, so the
// writer's empty-views branch (which is the only path that emits groups under
// <views>) keeps firing on the next round-trip. Before view-geometry support
// the reader never allocated views at all, so the writer always took that
// empty-views path; the geometry-only criterion preserves that contract for
// group-only views.
static bool ViewHasGeometry(tinyxml2::XMLElement *viewEl) {
  for (tinyxml2::XMLElement *c = viewEl->FirstChildElement(); c; c = c->NextSiblingElement()) {
    const char *name = c->Name();
    if (!name)
      continue;
    if (XmileReader::IsForeignNamespace(name))
      continue;
    std::string tag(name);
    if (tag == "stock" || tag == "aux" || tag == "flow" || tag == "alias" || tag == "connector")
      return true;
  }
  return false;
}

bool XmileReader::ProcessViews(tinyxml2::XMLElement *views, std::vector<std::string> &errs) {
  // Two shapes of children may appear at this level:
  //
  //   <group ...>          - direct under <views>. The writer emits this from
  //                          the empty-views-but-has-groups branch in
  //                          XMILEGenerator::generateSectorViews when the
  //                          model has sectors but no sketch.
  //   <view ...>           - a sketch view with stocks, flows, auxes,
  //                          connectors, aliases, and possibly nested
  //                          <group> children.
  //
  // For <group> direct children, harvest them eagerly so they survive a
  // round-trip through the no-sketch path. For <view> children with real
  // sketch geometry, instantiate an XmileView to populate a VensimView; v1
  // processes only the first such <view>, additional ones produce a soft
  // warning. A <view> whose only children are <group> elements is treated as
  // an alias for <views><group>: extract the groups in place and skip the
  // VensimView allocation (see ViewHasGeometry's rationale).
  int viewCount = 0;
  for (tinyxml2::XMLElement *child = views->FirstChildElement(); child; child = child->NextSiblingElement()) {
    const char *cname = child->Name();
    if (!cname)
      continue;
    if (IsForeignNamespace(cname))
      continue;
    std::string tag(cname);
    if (tag == "group") {
      if (!ProcessGroup(child, errs))
        return false;
    } else if (tag == "view") {
      if (!ViewHasGeometry(child)) {
        for (tinyxml2::XMLElement *vc = child->FirstChildElement("group"); vc; vc = vc->NextSiblingElement("group")) {
          if (!ProcessGroup(vc, errs))
            return false;
        }
        // Geometry-less views are group-only containers; they carry no
        // drawable elements and do not occupy a numbered view slot, so
        // viewCount is intentionally not incremented here.
        continue;
      }
      if (viewCount == 0) {
        VensimView *view = new VensimView();
        // XMILE <view> has no required title attribute; fall back to "main"
        // so the MDL sketch header has a non-empty title (the writer emits
        // the title verbatim into the \\\---/// header line).
        if (const char *vn = child->Attribute("name"))
          view->SetTitle(vn);
        else
          view->SetTitle("main");
        _model->AddView(view);
        XmileView xv(this, _model, view);
        if (!xv.ProcessView(child, errs))
          return false;
      } else {
        errs.push_back(std::string("multi-view XMILE: skipping view #") + std::to_string(viewCount + 1) +
                       " (v1 supports only the first view)");
      }
      ++viewCount;
    }
  }
  return true;
}

bool XmileReader::ProcessDimensions(tinyxml2::XMLElement *dimsEl, std::vector<std::string> &errs) {
  // Parity target: Vensim's `Dim: a, b, c` (VYacc.y:117) produces an Equation
  // with iEqType=':' whose RHS is an ExpressionSymbolList wrapping the element
  // SymbolList. MarkTypes (Variable.cpp:134-149) keys off the Symlist shape to
  // flip Dim to XMILE_Type_ARRAY and each element to XMILE_Type_ARRAY_ELM, so
  // matching that shape from the XMILE side is what makes the downstream
  // typing pass work without special-casing the source format.
  for (tinyxml2::XMLElement *dim = dimsEl->FirstChildElement("dim"); dim; dim = dim->NextSiblingElement("dim")) {
    const char *dimName = dim->Attribute("name");
    if (!dimName) {
      errs.push_back("<dim> with no name attribute");
      return false;
    }
    std::string normName = NormalizeName(dimName);
    // A subscript family is not a value, so it can never be what a control
    // variable holds -- and Vensim reserves the four control names, so the dim
    // and the control cannot coexist as two symbols on the way out. Dropping the
    // dim (rather than appending its element list as a second equation, which
    // .Control would emit as a duplicate definition) keeps the control intact.
    // The declaration paths handle the same collision through
    // ProcessControlDeclaration; a <dim> reaches neither DeclareVariable nor an
    // equation-bearing element, so it is checked here.
    const int controlIdx = ControlIndexOf(normName);
    if (controlIdx >= 0) {
      errs.push_back(std::string("<dim name=\"") + dimName + "\">: '" + normName + "' is the Vensim control variable " +
                     kControlNames[controlIdx] + " and cannot also name a dimension; the dimension is dropped");
      continue;
    }
    Variable *dimVar = InsertVariable(normName);
    if (!dimVar) {
      errs.push_back(std::string("<dim name=\"") + dimName + "\">: name collides with a non-variable symbol");
      return false;
    }
    EnsureCanonicalName(dimVar, normName);

    SymbolList *symList = nullptr;
    bool hasSize = (dim->Attribute("size") != nullptr);
    if (hasSize) {
      int n = dim->IntAttribute("size", 0);
      if (n <= 0) {
        errs.push_back(std::string("<dim name=\"") + dimName + "\"> has invalid size attribute");
        return false;
      }
      symList = BuildIndexedElementList(n, errs);
    } else {
      symList = BuildElementList(dim, errs);
    }
    // Checked for BOTH branches: an ExpressionSymbolList carrying a null
    // SymList() is dereferenced unconditionally by Variable::MarkTypes,
    // XMILEGenerator::generateDimensions and MDLGenerator::EmitDimensionEntry,
    // so a dimension that could not be built has to fail the parse rather than
    // travel as one that looks built.
    if (!symList)
      return false;

    ExpressionSymbolList *rhs = new ExpressionSymbolList(pSymbolNameSpace, symList, NULL);
    AddEquationFor(dimVar, nullptr, rhs, ':');
  }
  return true;
}

SymbolList *XmileReader::BuildElementList(tinyxml2::XMLElement *dimEl, std::vector<std::string> &errs) {
  SymbolList *list = nullptr;
  for (tinyxml2::XMLElement *elem = dimEl->FirstChildElement("elem"); elem; elem = elem->NextSiblingElement("elem")) {
    const char *elemName = elem->Attribute("name");
    if (!elemName) {
      errs.push_back("<elem> with no name attribute");
      return nullptr;
    }
    std::string normName = NormalizeName(elemName);
    Variable *elemVar = InsertVariable(normName);
    if (!elemVar) {
      errs.push_back(std::string("<elem name=\"") + elemName + "\">: name collides with a non-variable symbol");
      return nullptr;
    }
    EnsureCanonicalName(elemVar, normName);
    if (!list)
      list = new SymbolList(pSymbolNameSpace, elemVar, false);
    else
      list->Append(elemVar, false);
  }
  if (!list) {
    const char *dimName = dimEl->Attribute("name");
    errs.push_back(std::string("<dim name=\"") + (dimName ? dimName : "(?)") +
                   "\"> has no <elem> children and no size attribute");
  }
  return list;
}

SymbolList *XmileReader::BuildIndexedElementList(int n, std::vector<std::string> &errs) {
  // XMILE permits numeric element names ("1","2",...) and the writer emits
  // them as <elem name="1"/> rather than reconstructing the size="N" shorthand,
  // so eagerly materializing the elements here is round-trip-safe -- a
  // self-round-trip lands in the explicit-elem path on the second read.
  //
  // Limitation: the synthesized
  // element Variable names are bare integer strings ("1", "2", ...).  The
  // equation lexer treats bare digit sequences as numeric literals, so an
  // equation referencing inflow[1] would parse the "1" as a number token rather
  // than resolving to the element Variable named "1".  In practice no model in
  // the current corpus uses indexed-dim subscript references in equation text
  // directly, so the collision is harmless today.  Supporting such references
  // would require either a non-numeric element-name shape (e.g. "_1", "e1")
  // or special-cased bracketed-index parsing in the lexer.
  SymbolList *list = nullptr;
  for (int i = 1; i <= n; ++i) {
    std::string name = std::to_string(i);
    Variable *elemVar = InsertVariable(name);
    if (!elemVar) {
      // size="n" states the dimension's extent exactly, so materializing n-1 of
      // the n elements is not a partial success -- it is a dimension of the
      // wrong size, which every consumer downstream would then trust. Report it
      // the way BuildElementList reports the same collision and give up.
      errs.push_back(std::string("<dim size=\"") + std::to_string(n) + "\">: element name \"" + name +
                     "\" collides with a non-variable symbol");
      return nullptr;
    }
    if (!list)
      list = new SymbolList(pSymbolNameSpace, elemVar, false);
    else
      list->Append(elemVar, false);
  }
  return list;
}

Variable *XmileReader::DeclareVariable(tinyxml2::XMLElement *el, std::vector<std::string> &errs) {
  const char *name = el->Attribute("name");
  if (!name) {
    errs.push_back(std::string("<") + el->Name() + "> with no name attribute");
    return nullptr;
  }
  // XMILE allows literal newlines inside name attributes for visual line
  // wrapping (the simlin/Stella corpus uses names like "fractional \ngrowth
  // rate"). Vensim identifiers are single-line, so the reader collapses to a
  // single-space separator at the read boundary -- matching what MDLGenerator
  // emits on the way out.
  std::string normName = NormalizeName(name);
  Variable *v = InsertVariable(normName);
  if (!v) {
    errs.push_back(std::string("<") + el->Name() + " name=\"" + name + "\">: name collides with a non-variable symbol");
    return nullptr;
  }
  EnsureCanonicalName(v, normName);
  return v;
}

Expression *XmileReader::ParseEqnFor(tinyxml2::XMLElement *ctxEl, const char *eqnText, std::vector<std::string> &errs) {
  std::vector<std::string> eqnErrs;
  Expression *rhs = ParseEquation(eqnText ? eqnText : "", eqnErrs);
  const bool hardErr = ForwardEqnDiagnostics(ElementContext(ctxEl), eqnErrs, errs);
  if (!rhs || hardErr)
    return nullptr;
  return rhs;
}

void XmileReader::AddEquationFor(Variable *v, SymbolList *lhsSubs, Expression *rhs, int token) {
  ExpressionVariable *lhsVar = new ExpressionVariable(pSymbolNameSpace, v, lhsSubs);
  LeftHandSide *lhs = new LeftHandSide(pSymbolNameSpace, lhsVar, NULL, NULL, 0);
  Equation *eq = new Equation(pSymbolNameSpace, lhs, rhs, token);
  v->AddEq(eq);
}

bool XmileReader::ProcessAuxOrFlow(tinyxml2::XMLElement *el, std::vector<std::string> &errs) {
  // Handles <aux> and <flow> alike: a flow is structurally identical to an aux
  // from the reader's perspective (one or more equations attached to a
  // Variable). Post-parse, MarkStockFlows reclassifies a flow that is
  // referenced from a stock's <inflow>/<outflow> as XMILE_Type_FLOW; an
  // orphan <flow> stays XMILE_Type_AUX (MarkTypes only knows INTEG / AsFlow).
  //
  // <non_negative/> is an XMILE clamp marker; Vensim .mdl has no equivalent
  // representation, so it is dropped on output -- but not silently: an advisory
  // (WarnIfNonNegative, below) tells the user the converted model can go
  // negative where the source could not.
  Variable *v = DeclareVariable(el, errs);
  if (!v)
    return false;
  // A declaration of one of the four control names is <sim_specs>'s business,
  // not an equation of its own; the non_negative advisory below would be noise
  // on a variable whose equation is about to be dropped.
  if (ProcessControlDeclaration(el, v, errs))
    return true;
  WarnIfNonNegative(el, v, errs);

  // Per-element form is signaled by any <element subscript="..."> child; the
  // <dimensions> sibling is then informational only. Apply-to-all (with or
  // without a <dimensions> child) uses a single Equation -- the scalar case
  // is the no-<dimensions> sub-case of apply-to-all.
  if (el->FirstChildElement("element"))
    return ProcessPerElementEquations(el, v, errs);
  return ProcessAppliesToAllEquation(el, v, el->FirstChildElement("dimensions"), errs);
}

bool XmileReader::ProcessStandaloneGf(tinyxml2::XMLElement *el, std::vector<std::string> &errs) {
  Variable *v = DeclareVariable(el, errs);
  if (!v)
    return false;
  if (ProcessControlDeclaration(el, v, errs))
    return true;
  // Same shape as the "no eqn + gf" branch of ProcessAppliesToAllEquation: the
  // equation's RHS is the ExpressionTable itself with the '(' token, matching
  // VensimParse::AddTable's standalone lookup form.
  ExpressionTable *table = ProcessGf(el, errs);
  if (!table)
    return false;
  AddEquationFor(v, nullptr, table, '(');
  AttachUnitsAndDoc(el, v);
  return true;
}

bool XmileReader::ProcessAppliesToAllEquation(tinyxml2::XMLElement *varEl, Variable *v, tinyxml2::XMLElement *dimsChild,
                                              std::vector<std::string> &errs) {
  const char *vname = varEl->Attribute("name");
  tinyxml2::XMLElement *eqnEl = varEl->FirstChildElement("eqn");
  tinyxml2::XMLElement *gfEl = varEl->FirstChildElement("gf");
  const char *eqnText = (eqnEl && eqnEl->GetText()) ? eqnEl->GetText() : "";
  // A whitespace-only or newline-only <eqn> body (e.g. `<eqn>\n</eqn>`) must
  // not be treated as a real equation: it would route to the WITH LOOKUP branch
  // and then fail to parse, silently dropping a standalone-GF variable. The
  // canonical "no eqn" state is either a null GetText() or a body whose only
  // characters are whitespace.
  // 
  // Human - but we treat 0+0 as "" - basically this is an XMUTIL convention that helps in round tripping
  if (strcmp(eqnText, "0+0") == 0)
    eqnText = "";
  const bool hasEqn =
      std::any_of(eqnText, eqnText + std::strlen(eqnText), [](unsigned char c) { return !std::isspace(c); });

  // Four combinations of <eqn> / <gf>:
  //   (1) eqn + no gf    -> regular aux/flow equation (scalar or a2a)
  //   (2) eqn + gf       -> WITH LOOKUP: rhs = ExpressionLookup(input, table)
  //   (3) no eqn + gf    -> standalone graphical function, token '(' a la
  //                         VensimParse::AddTable (VensimParse.cpp:173)
  //   (4) no eqn + no gf -> soft-warn and leave the Variable equation-free;
  //                         MarkVariableTypes will catch the dangling symbol
  //                         downstream if it matters.
  //
  // The LHS allocation is deferred into each success branch so case (4) never
  // produces orphan unconfirmed allocations -- the namespace would still tear
  // them down at destruction, but skipping the allocation entirely keeps
  // case (4) allocation-free.

  if (!hasEqn && !gfEl) {
    errs.push_back(std::string("<") + varEl->Name() + " name=\"" + (vname ? vname : "(?)") +
                   "\"> has neither <eqn> nor <gf>");
    return true;
  }

  Expression *rhs = nullptr;
  int eqToken = '=';

  if (hasEqn && !gfEl) {
    rhs = ParseEqnFor(varEl, eqnText, errs);
    if (!rhs)
      return false;
  } else if (hasEqn && gfEl) {
    Expression *input = ParseEqnFor(varEl, eqnText, errs);
    if (!input)
      return false;
    ExpressionTable *table = ProcessGf(gfEl, errs);
    if (!table) {
      delete input;
      return false;
    }
    rhs = new ExpressionLookup(pSymbolNameSpace, input, table);
  } else {
    // Standalone graphical function: the Equation's expression is the
    // ExpressionTable itself with token '(' (matches the standalone shape
    // VensimParse::AddTable produces for `var( [(0,0)(1,1)], ... )`).
    ExpressionTable *table = ProcessGf(gfEl, errs);
    if (!table)
      return false;
    rhs = table;
    eqToken = '(';
  }

  SymbolList *lhsSubs = dimsChild ? BuildAppliesToAllSubs(dimsChild, errs) : nullptr;
  AddEquationFor(v, lhsSubs, rhs, eqToken);

  AttachUnitsAndDoc(varEl, v);
  return true;
}

void XmileReader::AttachUnitsAndDoc(tinyxml2::XMLElement *varEl, Variable *v) {
  // <units> / <doc> are stored both as raw strings (the writer's fallback
  // path) and as a parsed UnitExpression (the writer's primary path, and the
  // shape the comparator reads via Variable::Units()). Without the parsed
  // expression, an XMILE -> MDL -> Vensim re-parse round trip would compare
  // the raw "a/b/c" text against the canonical "a/(b*c)" that the Vensim
  // parser produces and flag a spurious "units differ" diff.
  if (tinyxml2::XMLElement *u = varEl->FirstChildElement("units")) {
    if (const char *ut = u->GetText())
      AttachVariableUnits(v, ut);
  }
  if (tinyxml2::XMLElement *d = varEl->FirstChildElement("doc")) {
    if (const char *dt = d->GetText())
      v->SetComment(dt);
  }
}

bool XmileReader::ProcessPerElementEquations(tinyxml2::XMLElement *varEl, Variable *v, std::vector<std::string> &errs) {
  // Per-element + <gf> (variable-level or per-element) is rare and
  // intentionally unsupported: the per-element equations always take
  // precedence here. Surface a warning so the corpus tells us if it ever
  // appears.
  if (varEl->FirstChildElement("gf")) {
    errs.push_back(ElementContext(varEl) +
                   "<gf> on a per-element subscripted variable is not supported (skipping <gf>)");
  }
  for (tinyxml2::XMLElement *elemEl = varEl->FirstChildElement("element"); elemEl;
       elemEl = elemEl->NextSiblingElement("element")) {
    if (elemEl->FirstChildElement("gf")) {
      const char *subs = elemEl->Attribute("subscript");
      errs.push_back(ElementContext(varEl) + "<gf> on <element subscript=\"" + (subs ? subs : "(?)") +
                     "\"> is not supported (skipping <gf>)");
    }
    const char *subs = elemEl->Attribute("subscript");
    if (!subs) {
      errs.push_back(ElementContext(varEl) + "<element> with no subscript attribute");
      return false;
    }
    SymbolList *lhsSubs = ParseSubscriptList(subs);
    if (!lhsSubs) {
      errs.push_back(ElementContext(varEl) + "empty or malformed subscript=\"" + subs + "\"");
      return false;
    }

    tinyxml2::XMLElement *eqnEl = elemEl->FirstChildElement("eqn");
    if (!eqnEl) {
      errs.push_back(ElementContext(varEl) + "<element subscript=\"" + subs + "\"> has no <eqn>");
      return false;
    }
    Expression *rhs = ParseEqnFor(varEl, eqnEl->GetText(), errs);
    if (!rhs)
      return false;
    AddEquationFor(v, lhsSubs, rhs, '=');
  }
  // Variable-level <units> and <doc> apply to all per-element equations -- the
  // writer reads back one units string per variable, not one per element.
  AttachUnitsAndDoc(varEl, v);
  return true;
}

SymbolList *XmileReader::ParseSubscriptList(const std::string &subscriptAttr) {
  // XMILEGenerator emits comma-space ("a, b") but Stella and some hand-edited
  // files emit a bare comma. Splitting on ',' and trimming whitespace handles
  // both shapes uniformly. Empty fragments (trailing or doubled commas) are
  // dropped silently; a fully-empty result is signalled by returning nullptr
  // so the caller can attach context to the diagnostic.
  SymbolList *list = nullptr;
  size_t i = 0;
  const size_t n = subscriptAttr.size();
  while (i < n) {
    size_t comma = subscriptAttr.find(',', i);
    size_t end = (comma == std::string::npos) ? n : comma;
    size_t start = i;
    while (start < end && std::isspace(static_cast<unsigned char>(subscriptAttr[start])))
      ++start;
    size_t finish = end;
    while (finish > start && std::isspace(static_cast<unsigned char>(subscriptAttr[finish - 1])))
      --finish;
    if (finish > start) {
      std::string token = subscriptAttr.substr(start, finish - start);
      Variable *elemVar = InsertVariable(NormalizeName(token.c_str()));
      if (elemVar) {
        if (!list)
          list = new SymbolList(pSymbolNameSpace, elemVar, false);
        else
          list->Append(elemVar, false);
      }
    }
    if (comma == std::string::npos)
      break;
    i = comma + 1;
  }
  return list;
}

SymbolList *XmileReader::BuildAppliesToAllSubs(tinyxml2::XMLElement *dimsEl, std::vector<std::string> &errs) {
  SymbolList *list = nullptr;
  for (tinyxml2::XMLElement *dim = dimsEl->FirstChildElement("dim"); dim; dim = dim->NextSiblingElement("dim")) {
    const char *dname = dim->Attribute("name");
    if (!dname) {
      // A <dim> inside a variable's <dimensions> block with no name attribute
      // is malformed. Report it to match the diagnostic emitted by
      // ProcessDimensions for the same malformed input, then continue building
      // as much of the subscript list as possible so downstream passes can
      // still diagnose further errors in the same variable.
      errs.push_back("<dim> with no name attribute");
      continue;
    }
    Variable *dimVar = InsertVariable(NormalizeName(dname));
    if (!dimVar)
      continue;
    if (!list)
      list = new SymbolList(pSymbolNameSpace, dimVar, false);
    else
      list->Append(dimVar, false);
  }
  return list;
}

// Parse a comma-separated list of doubles into a vector. Whitespace around
// tokens is tolerated (XMILE permits line-wrapped <ypts>/<xpts> bodies).
// strtod stops at the first non-numeric character so " 5.0 " parses as 5.0
// with no trailing-garbage check needed. When errs is non-null and any tokens
// are skipped, a single diagnostic naming the context (e.g. "<gf><ypts>") is
// appended so the caller does not have to guess why xpts/ypts lengths diverge.
static std::vector<double> ParseDoubleList(const char *text, std::vector<std::string> *errs, const char *context,
                                           char sep) {
  std::vector<double> out;
  if (!text)
    return out;
  const char *p = text;
  int skipped = 0;
  bool in_bad_token = false;
  while (*p) {
    const char *sep_start = p;
    while (*p && (std::isspace(static_cast<unsigned char>(*p)) || *p == sep))
      ++p;
    // Crossing any separator characters means we entered a new token region,
    // so a subsequent bad run counts as a fresh distinct bad token.
    if (p != sep_start)
      in_bad_token = false;
    if (!*p)
      break;
    char *endp = nullptr;
    double v = std::strtod(p, &endp);
    if (endp == p) {
      // No digits consumed -- skip one byte and resume so a stray non-numeric
      // token doesn't trap the loop. Count the transition into a bad run once
      // rather than once per byte so `skipped` reflects distinct token count.
      if (!in_bad_token) {
        ++skipped;
        in_bad_token = true;
      }
      ++p;
      continue;
    }
    in_bad_token = false;
    out.push_back(v);
    p = endp;
  }
  if (skipped > 0 && errs) {
    std::string msg = std::to_string(skipped) + " malformed token(s) skipped while parsing";
    if (context) {
      msg += " ";
      msg += context;
    }
    errs->push_back(msg);
  }
  return out;
}

ExpressionTable *XmileReader::ProcessGf(tinyxml2::XMLElement *gf, std::vector<std::string> &errs) {
  ExpressionTable *table = new ExpressionTable(pSymbolNameSpace);

  // XMILE 1.0 allows type="continuous" (default), "extrapolate", or "discrete".
  // Only "extrapolate" has a direct xmutil representation; "discrete" maps onto
  // continuous with a warning so the rest of the model still loads.
  if (const char *gtype = gf->Attribute("type")) {
    std::string t(gtype);
    if (t == "extrapolate") {
      table->SetExtrapolate(true);
    } else if (t == "discrete") {
      errs.push_back("<gf type=\"discrete\"> treated as continuous (xmutil has no discrete representation)");
    }
  }

  tinyxml2::XMLElement *yptsEl = gf->FirstChildElement("ypts");
  if (!yptsEl || !yptsEl->GetText()) {
    errs.push_back("<gf> has no <ypts>");
    delete table;
    return nullptr;
  }
  // XMILE lets <xpts>/<ypts> override the default comma item-separator via a sep
  // attribute (e.g. sep=";" for "0;5;10"); we split on that character instead.
  auto listSep = [](tinyxml2::XMLElement *el) -> char {
    const char *s = el->Attribute("sep");
    return (s && s[0]) ? s[0] : ',';
  };
  std::vector<double> ys = ParseDoubleList(yptsEl->GetText(), &errs, "<gf><ypts>", listSep(yptsEl));
  if (ys.empty()) {
    errs.push_back("<gf><ypts> is empty");
    delete table;
    return nullptr;
  }

  // Explicit <xpts> wins; otherwise derive even-spaced xs from <xscale>. The
  // fishbanks corpus uses the xscale-only shape, so the derived path is on the
  // round-trip critical path.
  std::vector<double> xs;
  tinyxml2::XMLElement *xptsEl = gf->FirstChildElement("xpts");
  if (xptsEl && xptsEl->GetText()) {
    xs = ParseDoubleList(xptsEl->GetText(), &errs, "<gf><xpts>", listSep(xptsEl));
  } else if (tinyxml2::XMLElement *xscaleEl = gf->FirstChildElement("xscale")) {
    double xmin = xscaleEl->DoubleAttribute("min", 0.0);
    double xmax = xscaleEl->DoubleAttribute("max", 1.0);
    if (xmax <= xmin) {
      errs.push_back(std::string("<xscale min=") + std::to_string(xmin) + " max=" + std::to_string(xmax) +
                     "> not strictly increasing; graphical function may produce constant or reversed output");
    }
    int n = static_cast<int>(ys.size());
    if (n == 1) {
      xs.push_back(xmin);
    } else {
      double step = (xmax - xmin) / (n - 1);
      for (int i = 0; i < n; ++i)
        xs.push_back(xmin + i * step);
    }
  } else {
    errs.push_back("<gf> has neither <xpts> nor <xscale>");
    delete table;
    return nullptr;
  }

  // Length mismatch is permissive -- silently truncate to the shorter side
  // after warning. Mirrors MDLFormat::WriteLookupBody (MDLFormat.cpp:280).
  size_t npairs = std::min(xs.size(), ys.size());
  if (xs.size() != ys.size()) {
    errs.push_back("<gf> has mismatched <xpts>/<ypts> lengths; truncating to " + std::to_string(npairs));
  }
  for (size_t i = 0; i < npairs; ++i)
    table->AddPair(xs[i], ys[i]);
  return table;
}

Expression *XmileReader::BuildNetFlowSubscripted(const std::vector<std::string> &inflowNames,
                                                 const std::vector<std::string> &outflowNames, SymbolList *lhsSubs,
                                                 std::vector<std::string> &errs) {
  // Shape constraint enforced by is_all_plus_minus (Expression.cpp:86): a
  // left-leaning tree of binary +/- operators with bare ExpressionVariable
  // leaves on the right of each subtraction. Anything else (a unary minus,
  // a parenthesized subgroup, a constant on the right) makes MarkStockFlows
  // mark the flow list invalid and the writer falls back to emitting the raw
  // INTEG, losing the per-flow round-trip.
  //
  // When lhsSubs is non-null each flow reference receives an independent
  // SymbolList::Clone so the per-ExpressionVariable destructor can free its
  // own pSubList without crashing the sibling references at teardown.
  //
  // A flow name held by a non-Variable symbol (a registered builtin such as
  // STEP) makes InsertVariable answer NULL. There is no partial expression
  // worth keeping: the tree built here IS the stock's equation, and an
  // ExpressionVariable with a NULL Variable is dereferenced unconditionally by
  // both writers. So the builder fails, freeing what it has built -- matching
  // what DeclareVariable already does for a <flow> DECLARED under that name.
  auto makeRef = [&](const std::string &name, const char *role) -> Expression * {
    Variable *v = InsertVariable(name);
    if (!v) {
      errs.push_back(std::string("<") + role + ">" + name + "</" + role +
                     ">: name collides with a non-variable symbol");
      return nullptr;
    }
    SymbolList *subsCopy = lhsSubs ? lhsSubs->Clone() : nullptr;
    return new ExpressionVariable(pSymbolNameSpace, v, subsCopy);
  };

  Expression *acc = nullptr;
  for (const std::string &name : inflowNames) {
    Expression *ev = makeRef(name, "inflow");
    if (!ev) {
      delete acc;
      return nullptr;
    }
    if (!acc)
      acc = ev;
    else
      acc = new ExpressionAdd(pSymbolNameSpace, acc, ev);
  }
  // No inflows but at least one outflow: lead with a unary minus on the first
  // outflow ("-O1 - O2 ...") rather than seeding "0 - O1 - O2 ...". A leading
  // literal 0 makes is_all_plus_minus (Expression.cpp) reject the whole chain,
  // leaving the outflows unclassified as flows; the unary-minus form is exactly
  // what Vensim writes for an outflow-only stock and what the classifier
  // accepts, so the flow types survive the round trip.
  size_t outflowStart = 0;
  if (!acc && !outflowNames.empty()) {
    Expression *ev = makeRef(outflowNames[0], "outflow");
    if (!ev)
      return nullptr;
    acc = new ExpressionUnaryMinus(pSymbolNameSpace, ev, nullptr);
    outflowStart = 1;
  }
  // No inflows AND no outflows: a literal 0. is_all_plus_minus rejects pure
  // numbers, but a stock with neither inflow nor outflow is a degenerate edge
  // case the writer wouldn't round-trip anyway.
  if (!acc)
    acc = new ExpressionNumber(pSymbolNameSpace, 0.0);
  for (size_t i = outflowStart; i < outflowNames.size(); i++) {
    Expression *ev = makeRef(outflowNames[i], "outflow");
    if (!ev) {
      delete acc;
      return nullptr;
    }
    acc = new ExpressionSubtract(pSymbolNameSpace, acc, ev);
  }
  return acc;
}

Expression *XmileReader::BuildIntegExpression(Expression *netFlow, Expression *init, std::vector<std::string> &errs) {
  Function *integ = static_cast<Function *>(pSymbolNameSpace->Find("INTEG"));
  if (!integ) {
    // The XmileReader ctor seeds this; reaching this branch means the
    // namespace was tampered with after construction.
    errs.push_back("INTEG function not registered (xmutil function table is empty)");
    return nullptr;
  }
  ExpressionList *args = new ExpressionList(pSymbolNameSpace);
  args->Append(netFlow);
  args->Append(init);
  return new ExpressionFunctionMemory(pSymbolNameSpace, integ, args);
}

bool XmileReader::ProcessStock(tinyxml2::XMLElement *stock, std::vector<std::string> &errs) {
  // Synthesizes an INTEG expression at read time so the existing post-parse
  // pipeline (Model::MarkVariableTypes -> MarkTypes -> MarkStockFlows) sees
  // the same shape VensimParse produces for `s = INTEG(in - out, init)`.
  // MarkStockFlows walks the INTEG arg-0 expression to populate
  // stock->Inflows() / Outflows(); the writers (XMILE and MDL) read from those
  // lists. There is no separate "record the flow names" channel.
  //
  // Subscripted stocks come in two shapes:
  //   * apply-to-all: a single top-level <eqn> + <dimensions> child. One
  //     Equation, LHS subscripted with the dim list, net-flow references
  //     mirror the LHS subscripts (so MarkStockFlows still sees in - out).
  //   * per-element: one <element subscript="..."><eqn>...</eqn></element>
  //     per element. One Equation per element, each with its own LHS subscript
  //     and a net-flow expression whose flow references carry that same
  //     subscript -- which is why every reference needs an independent
  //     SymbolList::Clone (sharing would double-free at teardown).
  Variable *v = DeclareVariable(stock, errs);
  if (!v)
    return false;
  // A control name cannot be a stock: synthesizing the INTEG would put
  // `TIME STEP = INTEG(...)` in .Control, which Vensim rejects. Returning here
  // also leaves _flowToStocks untouched, so the view pass does not try to anchor
  // a pipe on a stock that was never built.
  if (ProcessControlDeclaration(stock, v, errs))
    return true;
  // DeclareVariable verified the name attribute; EnsureCanonicalName made the
  // Variable's stored name the normalized form.
  const char *name = stock->Attribute("name");
  const std::string &normName = v->GetName();
  WarnIfNonNegative(stock, v, errs);

  // <gf> has no valid semantics on a stock -- the XMILE writer asserts it
  // (the XMILE_Type_AUX || XMILE_Type_FLOW assert in XMILEGenerator.cpp's
  // <gf> emission) and no engine consumes it. Surface a warning so a malformed
  // corpus file is visible, then ignore the element.
  if (stock->FirstChildElement("gf")) {
    errs.push_back(ElementContext(stock) + "<gf> on a stock is not supported (skipping <gf>)");
  }

  std::vector<std::string> inflowNames;
  std::vector<std::string> outflowNames;
  for (tinyxml2::XMLElement *child = stock->FirstChildElement("inflow"); child;
       child = child->NextSiblingElement("inflow")) {
    if (const char *t = child->GetText())
      inflowNames.push_back(NormalizeName(t));
  }
  for (tinyxml2::XMLElement *child = stock->FirstChildElement("outflow"); child;
       child = child->NextSiblingElement("outflow")) {
    if (const char *t = child->GetText())
      outflowNames.push_back(NormalizeName(t));
  }

  // Record the structural stock<->flow association for the view pass: a
  // flow's pipe endpoint may only anchor on a stock that lists it here (see
  // StocksForFlow).
  for (const std::string &f : inflowNames)
    _flowToStocks[FoldNameKey(f)].push_back(FoldNameKey(normName));
  for (const std::string &f : outflowNames)
    _flowToStocks[FoldNameKey(f)].push_back(FoldNameKey(normName));

  if (stock->FirstChildElement("element")) {
    // Per-element subscripted stock.
    for (tinyxml2::XMLElement *elemEl = stock->FirstChildElement("element"); elemEl;
         elemEl = elemEl->NextSiblingElement("element")) {
      const char *subs = elemEl->Attribute("subscript");
      if (!subs) {
        errs.push_back(ElementContext(stock) + "<element> with no subscript attribute");
        return false;
      }
      SymbolList *elementSubList = ParseSubscriptList(subs);
      if (!elementSubList) {
        errs.push_back(ElementContext(stock) + "empty subscript=\"" + subs + "\"");
        return false;
      }

      tinyxml2::XMLElement *eqnEl = elemEl->FirstChildElement("eqn");
      if (!eqnEl) {
        errs.push_back(ElementContext(stock) + "<element subscript=\"" + subs + "\"> has no <eqn>");
        return false;
      }
      Expression *init = ParseEqnFor(stock, eqnEl->GetText(), errs);
      if (!init)
        return false;

      Expression *netFlow = BuildNetFlowSubscripted(inflowNames, outflowNames, elementSubList, errs);
      if (!netFlow)
        return false;
      Expression *integExpr = BuildIntegExpression(netFlow, init, errs);
      if (!integExpr)
        return false;

      AddEquationFor(v, elementSubList, integExpr, '=');
    }
  } else {
    // Apply-to-all (subscripted or scalar). Scalar is just the no-<dimensions>
    // sub-case (BuildNetFlowSubscripted with a null lhsSubs).
    tinyxml2::XMLElement *eqnEl = stock->FirstChildElement("eqn");
    if (!eqnEl) {
      errs.push_back(std::string("<stock name=\"") + name + "\"> has no <eqn>");
      return false;
    }
    Expression *init = ParseEqnFor(stock, eqnEl->GetText(), errs);
    if (!init)
      return false;

    tinyxml2::XMLElement *dimsChild = stock->FirstChildElement("dimensions");
    SymbolList *lhsSubs = dimsChild ? BuildAppliesToAllSubs(dimsChild, errs) : nullptr;
    Expression *netFlow = BuildNetFlowSubscripted(inflowNames, outflowNames, lhsSubs, errs);
    if (!netFlow)
      return false;
    Expression *integExpr = BuildIntegExpression(netFlow, init, errs);
    if (!integExpr)
      return false;

    AddEquationFor(v, lhsSubs, integExpr, '=');
  }

  AttachUnitsAndDoc(stock, v);
  return true;
}

bool XmileReader::ValidateLookupTargets(std::vector<std::string> &errs) {
  std::set<std::string> phantoms;
  for (Variable *v : _model->GetVariables(nullptr)) {
    for (Equation *eq : v->GetAllEquations())
      CollectPhantomLookups(eq->GetExpression(), phantoms);
    for (Equation *eq : v->GetAllInitEquations())
      CollectPhantomLookups(eq->GetExpression(), phantoms);
  }
  for (const std::string &name : phantoms) {
    errs.push_back("'" + name + "' is applied as a function or lookup (" + name +
                   "(...)) but is never defined; it may be an unsupported or misspelled function name");
  }
  return phantoms.empty();
}

void XmileReader::WarnIfNonNegative(tinyxml2::XMLElement *varEl, Variable *v, std::vector<std::string> &errs) {
  tinyxml2::XMLElement *nn = varEl->FirstChildElement("non_negative");
  if (!nn)
    return;
  // A self-closing <non_negative/> or an empty body means the clamp is on; an
  // explicit body is a case-insensitive, whitespace-trimmed boolean, so
  // <non_negative>false</non_negative> turns it off (no advisory).
  bool clamp = true;
  if (const char *body = nn->GetText()) {
    std::string t(body);
    size_t a = t.find_first_not_of(" \t\r\n");
    if (a != std::string::npos) {
      size_t b = t.find_last_not_of(" \t\r\n");
      std::string val = t.substr(a, b - a + 1);
      clamp = !StringMatch(val, "false") && val != "0";
    }
  }
  if (!clamp)
    return;
  errs.push_back("warning: <non_negative> clamp on '" + v->GetName() +
                 "' is not representable in Vensim .mdl output; the converted model can go negative");
}

void XmileReader::ScanForShadowedKeywords(tinyxml2::XMLElement *model) {
  // Only <variables> children can declare a variable a bare identifier in an
  // equation would resolve to. <dimensions> declares dimension and element
  // Variables, but those are reachable only from subscript positions, which
  // the grammar routes away from xpyy_resolve_symbol's keyword check.
  tinyxml2::XMLElement *variables = model->FirstChildElement("variables");
  if (!variables)
    return;
  for (tinyxml2::XMLElement *child = variables->FirstChildElement(); child; child = child->NextSiblingElement()) {
    const char *tag = child->Name();
    if (!tag || IsForeignNamespace(tag))
      continue;
    const char *name = child->Attribute("name");
    if (name && StringMatch(NormalizeName(name), "pi"))
      _declaresPi = true;
  }
}

bool XmileReader::ProcessModel(tinyxml2::XMLElement *model, std::vector<std::string> &errs) {
  // Multi-<model> rejection is handled by ProcessFile's envelope pre-pass;
  // here we only walk the <variables> children of the one <model> we're
  // given.
  ScanForShadowedKeywords(model);
  tinyxml2::XMLElement *variables = model->FirstChildElement("variables");
  if (!variables)
    return true;
  for (tinyxml2::XMLElement *child = variables->FirstChildElement(); child; child = child->NextSiblingElement()) {
    const char *name = child->Name();
    if (!name)
      continue;
    if (IsForeignNamespace(name))
      continue;
    std::string tag(name);
    if (tag == "module") {
      // <module> elements signal a submodel, which is not supported. The
      // error message names the offending
      // module when an attribute is present so the diagnostic points the
      // human at the failing element.
      const char *modname = child->Attribute("name");
      std::string msg = "<module";
      if (modname) {
        msg += " name=\"";
        msg += modname;
        msg += "\"";
      }
      msg += ">: modules are not supported";
      errs.push_back(msg);
      return false;
    }
    if (tag == "aux" || tag == "flow") {
      if (!ProcessAuxOrFlow(child, errs))
        return false;
    } else if (tag == "gf") {
      // A top-level <gf name="..."> under <variables> is a standalone named
      // graphical function -- a lookup variable referenced elsewhere as
      // name(input). It is a sibling of <aux>, not a child of one, so it needs
      // its own handler; without it the table data (and the variable itself)
      // would be silently dropped and any name(input) reference left dangling.
      if (!ProcessStandaloneGf(child, errs))
        return false;
    } else if (tag == "stock") {
      if (!ProcessStock(child, errs))
        return false;
    } else if (tag == "dimensions") {
      // The <dimensions> child of <variables> is informational on subscripted
      // variables and is handled inside ProcessAuxOrFlow / ProcessStock.
      // A stray <dimensions> at <variables> scope (legal in some XMILE
      // dialects but never emitted by xmutil) is silently dropped here.
      continue;
    } else if (IsStellaUIWidget(name)) {
      continue;
    } else {
      // Unknown element inside <variables>. Quiet for now -- if a corpus run
      // surfaces something noteworthy, we can tighten to an error.
      continue;
    }
  }
  // The top-level <dimensions> element is a sibling of <variables> under
  // <model>. Walk it after the variables so any subscripted variable that
  // referenced the dim by name during equation parsing already has its
  // placeholder Variable in the namespace -- ProcessDimensions then attaches
  // the dim Variable's defining equation. MarkTypes (post-parse) flips both
  // sides to XMILE_Type_ARRAY / XMILE_Type_ARRAY_ELM.
  if (tinyxml2::XMLElement *dims = model->FirstChildElement("dimensions")) {
    if (!ProcessDimensions(dims, errs))
      return false;
  }
  // <views> contains layout geometry and (in some writer paths) the only
  // surviving record of group membership; ProcessViews reads both.
  if (tinyxml2::XMLElement *views = model->FirstChildElement("views")) {
    if (!ProcessViews(views, errs))
      return false;
  }
  return true;
}

Variable *XmileReader::SetControlVariable(const std::string &name, double value, bool stated) {
  // Mirror the shape VensimParse produces for control variables: a Variable
  // in the namespace whose first equation is a constant-numeric expression.
  // The writer's GetConstanValue path reads from this equation; the engine
  // also has parallel _initial_time / _final_time / _dt fields the caller
  // updates separately.
  //
  // ProcessFile's pre-pass dispatches every <sim_specs> before <model>, so the
  // variable is normally freshly created and equation-free here; a document
  // carrying two <sim_specs> elements is the one way it is not. Appending in
  // that case would leave the Variable with two equations, which MDLGenerator
  // emits as two .Control entries -- a duplicate definition Vensim rejects --
  // so the first statement of a value stands and SettleControl reconciles the
  // Model field back to it.
  Variable *v = InsertVariable(name);
  if (!v)
    return nullptr;
  if (stated && v->GetAllEquations().empty())
    AddEquationFor(v, nullptr, new ExpressionNumber(pSymbolNameSpace, value), '=');
  return v;
}

int XmileReader::ControlIndexOf(const std::string &name) {
  static_assert(sizeof(kControlNames) / sizeof(kControlNames[0]) == kControlCount,
                "kControlNames and ControlIndex must describe the same four control variables");
  // Compare under the namespace's own identifier equivalence rather than by
  // string: ToLowerSpace lowercases and folds '_' / whitespace runs to a single
  // space, so `TIME_STEP` -- the spelling SDEverywhere and PySD exports use --
  // is the same symbol as the `TIME STEP` <sim_specs> registers, and a literal
  // compare would miss it. MDLGenerator::IsControlVar answers the same question
  // the same way when deciding what belongs in .Control.
  std::string *canon = SymbolNameSpace::ToLowerSpace(name);
  int idx = -1;
  for (int i = 0; i < kControlCount; i++) {
    std::string *want = SymbolNameSpace::ToLowerSpace(kControlNames[i]);
    const bool hit = (*canon == *want);
    delete want;
    if (hit) {
      idx = i;
      break;
    }
  }
  delete canon;
  return idx;
}

bool XmileReader::ProcessControlDeclaration(tinyxml2::XMLElement *el, Variable *v, std::vector<std::string> &errs) {
  const int idx = ControlIndexOf(v->GetName());
  if (idx < 0)
    return false;  // an ordinary variable: the caller owns it entirely

  // XMILE does not reserve these names -- <sim_specs> is where every XMILE tool
  // reads the run's start, stop and dt, and a variable in <variables> that
  // happens to be called TIME_STEP is, to XMILE, just a variable. Vensim .mdl
  // does reserve them, so the two collapse onto one symbol on the way out and
  // the conversion has to pick a value. It picks <sim_specs>: that is the value
  // the source document actually simulates with, and it is the value already
  // sitting in the engine's own initial_time/final_time/dt fields -- fields
  // Model::GetConstanValue falls back to, so letting the declaration overwrite
  // the equation would leave the two halves of the same answer disagreeing.
  //
  // The declaration therefore contributes no equation, with ONE exception: when
  // <sim_specs> never stated the value, the number it supplied was invented by
  // this reader (1.0 for a missing <dt>, and so on). An invented default must
  // not beat something the modeler wrote, so a declaration that states a usable
  // constant is allowed through to fill the gap and ApplyControlValues then
  // pushes it into the engine field. Everything else the declaration carries --
  // <units>, <doc> -- is attached either way; only the equation is at stake.
  const char *tag = el->Name();
  const bool auxOrFlow = tag && (std::strcmp(tag, "aux") == 0 || std::strcmp(tag, "flow") == 0);
  // A control variable is a scalar constant. A subscripted declaration, a
  // graphical function, or a stock's INTEG is not a value the engine's double
  // fields or Model::GetConstanValue can represent at all, so those shapes can
  // never supply one -- and a synthesized INTEG under a control name would put
  // `TIME STEP = INTEG(...)` in .Control, which Vensim rejects outright.
  const bool scalarShape = auxOrFlow && !el->FirstChildElement("dimensions") && !el->FirstChildElement("element") &&
                           !el->FirstChildElement("gf");
  tinyxml2::XMLElement *eqnEl = scalarShape ? el->FirstChildElement("eqn") : nullptr;
  double declared = 0.0;
  const bool haveDeclared = eqnEl && ParseNumericLiteral(eqnEl->GetText(), &declared);

  if (haveDeclared && v->GetAllEquations().empty()) {
    // Nothing has claimed this control yet -- <sim_specs> did not state it, and
    // no earlier declaration filled it -- so let the caller attach the
    // declaration through the ordinary path, <units> and <doc> included.
    return false;
  }

  // <units> and <doc> belong to the Variable whatever becomes of the equation --
  // a control's own <units> is still the more specific claim than the
  // document-wide time_units. AttachVariableUnits needs the Variable to have
  // content, which it only gets with its first equation, so a control still
  // waiting on ApplyControlValues holds the element until SettleControl has one.
  // First source wins there as it does everywhere else, hence the null test.
  if (!v->GetAllEquations().empty())
    AttachUnitsAndDoc(el, v);
  else if (!_controls[idx].pendingUnitsAndDoc)
    _controls[idx].pendingUnitsAndDoc = el;

  // The value the control already holds. Reaching here with a usable constant
  // declaration means something already claimed the slot, so this is the value
  // that wins; with no usable declaration there is nothing to compare and the
  // message says only that the declaration was dropped.
  double settled = 0.0;
  const bool haveSettled = ConstantValueOf(v, &settled);
  if (haveDeclared && haveSettled && declared == settled)
    return true;  // an agreeing restatement says nothing worth reporting

  std::string detail;
  if (haveDeclared && haveSettled)
    detail =
        "the declared " + mdl::FormatMDLNumber(declared) + " is dropped in favor of " + mdl::FormatMDLNumber(settled);
  else
    detail = "this declaration cannot state one (a control variable is a scalar constant) and is dropped";
  errs.push_back(ElementContext(el) + "'" + v->GetName() + "' is the Vensim control variable " + kControlNames[idx] +
                 ", whose value comes from <sim_specs>: " + detail);
  return true;
}

void XmileReader::ApplyControlValues() {
  // Runs once the whole document has been walked, so a <variables> declaration
  // of a control name <sim_specs> left unstated has already had its chance to
  // supply the value (see ProcessControlDeclaration). Two jobs: give every
  // control the one equation it is still missing, and make the engine's own
  // time fields agree with the constant that equation holds.
  //
  // Each fallback is what <sim_specs> recorded, which for a document that
  // carried none is the Model's own default (seeded in the ctor). SAVEPER is
  // settled last and is the one that cannot use its recorded value unless the
  // document stated it: SAVEPER defaults to dt, and dt is only final once
  // TIME STEP has settled -- a declaration may just have supplied it.
  SettleControl(kInitialTime, _controls[kInitialTime].value);
  SettleControl(kFinalTime, _controls[kFinalTime].value);
  SettleControl(kTimeStep, _controls[kTimeStep].value);
  SettleControl(kSaveper, _controls[kSaveper].stated ? _controls[kSaveper].value : _model->dt());
}

void XmileReader::SettleControl(int idx, double fallback) {
  Variable *v = _controls[idx].var;
  if (!v) {
    // No <sim_specs> ran, so nothing materialized the control -- but <variables>
    // may still have declared it. Never create one here: a document that states
    // no sim specs at all leaves MDLGenerator::GenerateControl to synthesize the
    // .Control entries from the Model's own fields, and inventing a Variable
    // would change which of those two paths runs.
    v = FindVariable(kControlNames[idx]);
    if (!v)
      return;
  }
  if (v->GetAllEquations().empty())
    AddEquationFor(v, nullptr, new ExpressionNumber(pSymbolNameSpace, fallback), '=');
  // The Variable now has content, so a declaration's <units>/<doc> that had to
  // wait for it can land -- still ahead of ApplyDeferredTimeUnits, which is what
  // keeps a control's own <units> beating the document-wide time_units.
  if (tinyxml2::XMLElement *pending = _controls[idx].pendingUnitsAndDoc)
    AttachUnitsAndDoc(pending, v);

  // Model::GetConstanValue reads the equation and falls back to the field, so a
  // field that disagrees with a constant equation is a split brain no consumer
  // can see. Only a constant can be mirrored; for anything else the field keeps
  // what <sim_specs> put there, which is exactly what GetConstanValue then
  // returns for that variable.
  double value = 0.0;
  if (!ConstantValueOf(v, &value))
    return;
  switch (idx) {
  case kInitialTime:
    _model->set_initial_time(value);
    break;
  case kFinalTime:
    _model->set_finall_time(value);  // misspelled in Model.h; mirrors header
    break;
  case kTimeStep:
    _model->set_dt(value);
    break;
  default:
    break;  // SAVEPER has no Model field of its own
  }
}

Variable *XmileReader::FindVariable(const std::string &name) {
  Symbol *sym = pSymbolNameSpace->Find(name);
  if (sym && sym->isType() == Symtype_Variable)
    return static_cast<Variable *>(sym);
  return nullptr;
}

Expression *XmileReader::ParseEquation(const std::string &text, std::vector<std::string> &errs) {
  // The bison shims (xpyy_set_result, xpyylex, xpyyerror) reach into the
  // reader through XPObject. We stash the lexer, errs sink, and result slot
  // for the duration of the parse, then clear them so subsequent calls or
  // unrelated reader use can't see stale pointers. The lex instance is
  // stack-local because XMILE has no global per-document equation state --
  // each <eqn> stands alone.
  XmileEqLex lex;
  lex.Initialize(text.c_str(), text.size());

  // No save/restore of prior slot values: ParseEquation is never re-entered
  // (the bison actions only build Expression nodes; none of them parses), so
  // the slots are always null on entry and clearing on exit is sufficient.
  _currentLex = &lex;
  _currentErrs = &errs;
  _lastParsedExpr = nullptr;

  const int rc = xpyyparse();

  Expression *result = _lastParsedExpr;

  _currentLex = nullptr;
  _currentErrs = nullptr;
  _lastParsedExpr = nullptr;

  if (rc != 0) {
    if (errs.empty())
      errs.push_back("parse error in equation: " + text);
    // Free any partial Expression we may have stashed via xpyy_set_result
    // before the abort -- xpyyerror is allowed to set it before YYABORT
    // (the apostrophe rule, for one, does not).
    delete result;
    return nullptr;
  }
  return result;
}

Variable *XmileReader::InsertVariable(const std::string &name) {
  Symbol *sym = pSymbolNameSpace->Find(name);
  if (sym) {
    // A non-Variable entry under this name is a type collision (e.g. a
    // Function registered under "INTEG"); surfacing it as a soft failure
    // (nullptr) lets the equation parser tag the error rather than crashing.
    // Non-Variable symbols return nullptr; the caller may surface a typed
    // error if needed.
    if (sym->isType() == Symtype_Variable)
      return static_cast<Variable *>(sym);
    return nullptr;
  }
  // The Variable ctor registers itself in pSymbolNameSpace; the next Find for
  // the same name will hit.
  return new Variable(pSymbolNameSpace, name);
}

void XmileReader::EnsureCanonicalName(Variable *v, const std::string &declaredName) {
  if (!v || v->GetName() == declaredName)
    return;
  // The hashtable key uses ToLowerSpace, which folds `_` and ` ` together; the
  // pre- and post-rename keys are therefore the same entry, so a direct SetName
  // (rather than going through SymbolNameSpace::Rename, which refuses to
  // re-key onto a colliding slot) is safe and leaves the hash consistent. The
  // alternate-name field is set by Variable::AddEq on the first equation; we
  // mutate sName before equations are attached at the declaration site, so the
  // later AddEq picks up the canonical form.
  v->SetName(declaredName);
}
