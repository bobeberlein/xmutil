#ifndef _XMUTIL_XMILE_XMILEREADER_H
#define _XMUTIL_XMILE_XMILEREADER_H
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tinyxml2 {
class XMLElement;
}

class Expression;
class ExpressionTable;
class Model;
class ModelGroup;
class SymbolList;
class SymbolNameSpace;
class UnitExpression;
class Units;
class Variable;
class XmileEqLex;

// XmileReader walks an XMILE document and populates a Model: it validates the
// document envelope (well-formed XML, root <xmile>) and reads <sim_specs>,
// <model_units>, <dimensions>, <model>, and <views>.
class XmileReader {
public:
  explicit XmileReader(Model *model);
  ~XmileReader();

  // ProcessFile parses contents (a buffer of len bytes -- not required to be
  // NUL-terminated; the design uses uint32_t lengths from the extern-C path).
  // On failure, descriptive messages are appended to errs and false is
  // returned. The errs parameter is new compared to VensimParse::ProcessFile;
  // it is the XMILE reader's stable error channel.
  bool ProcessFile(const std::string &filename, const char *contents, size_t len, std::vector<std::string> &errs);

  SymbolNameSpace *GetSymbolNameSpace() {
    return pSymbolNameSpace;
  }

  // Parse a single XMILE equation string into an Expression tree. On success
  // the caller owns the returned pointer; on failure the return is NULL and
  // a descriptive message is appended to errs. The text is treated as the
  // contents of one <eqn> element -- no surrounding XML, no equation
  // separators. Called per <eqn> by the DOM walker.
  Expression *ParseEquation(const std::string &text, std::vector<std::string> &errs);

  // Variable resolution helpers used by the equation parser's action shims.
  // FindVariable returns NULL when the name is not a Variable in the
  // namespace; InsertVariable creates the variable on miss. The semantics
  // mirror VensimParse::FindVariable / InsertVariable, which the equation
  // parser action code is patterned on.
  Variable *FindVariable(const std::string &name);
  Variable *InsertVariable(const std::string &name);

  // Stamp the canonical display name on a Variable at its declaration site.
  // The namespace folds `_` and space (ToLowerSpace) for hash lookup, so a
  // placeholder created by an earlier equation reference like `foo_bar` resolves
  // to the same Variable as a later `<aux name="foo bar">` declaration -- but
  // the Variable's stored display name (Symbol::sName) reflects whichever form
  // arrived first. Writers emit that stored form. If a model declares the
  // variable AFTER referencing it, the round trip flips the display from
  // "foo bar" to "foo_bar", causing the comparator to see two distinct names
  // across the round trip (the test of record is the reliability corpus, which
  // forward-references developer-required auxes from each flow's eqn before
  // the aux is declared). Because the canonical hash key is invariant under
  // `_`<->` ` swaps, mutating sName here keeps the hashtable consistent.
  void EnsureCanonicalName(Variable *v, const std::string &declaredName);

  // Per-parse state accessors. The equation parser stashes a stack-local
  // XmileEqLex and an errs vector on the reader for the duration of one
  // ParseEquation call so the bison action shims (which only see the global
  // XPObject) can reach them.
  XmileEqLex *CurrentLex() {
    return _currentLex;
  }
  std::vector<std::string> *CurrentErrs() {
    return _currentErrs;
  }
  void SetLastParsedExpr(Expression *e) {
    _lastParsedExpr = e;
  }

  // True when this document DECLARES a variable spelled `pi`. XMILE reserves
  // `pi` as the mathematical constant and Vensim has no PI builtin, so the
  // equation shims lower `pi` / `pi()` to a numeric literal -- but a document
  // that declares its own `pi` has said otherwise, and substituting 3.14159
  // for it changes what every referencing equation computes while still
  // emitting the now-unreferenced declaration.
  //
  // Answered from a pre-pass over the whole <model> (ScanForShadowedKeywords)
  // rather than from a namespace lookup at resolve time: XMILE fixes no order
  // between a declaration and a reference, so deciding at the reference would
  // make one half of a document see the constant and the other half the
  // variable, purely on element order.
  bool DeclaresPi() const {
    return _declaresPi;
  }

  // Static helpers exposed for sibling translation units (XmileView calls
  // NormalizeName / IsForeignNamespace). They have no per-reader state.

  // True if the qualified element name carries an XML namespace prefix (a
  // ':' in the tag string). XMILE default-namespace elements have no prefix;
  // isee:, simlin:, and similar vendor-namespaced siblings do. The reader
  // treats prefix presence as the skip signal -- full XML namespace
  // resolution is unnecessary because the filter is by-name-only.
  static bool IsForeignNamespace(const char *qualifiedName);

  // True if the unprefixed element name is a documented Stella UI widget.
  // These widgets carry no semantic content for the simulation model, so the
  // reader drops them silently. Out-of-list elements fall through to the
  // generic "unknown XMILE element" handling at the call site.
  static bool IsStellaUIWidget(const char *unprefixedName);

  // Collapse internal whitespace runs (newlines included) to a single space
  // and trim leading/trailing whitespace. XMILE allows line-wrapped name
  // attributes; Vensim identifiers are single-line, so the corpus needs this
  // normalization at the read boundary.
  static std::string NormalizeName(const char *raw);

  // Lookup key for matching variable names across spelling variants:
  // ASCII-lowered, with runs of '_'/' ' folded to one space and outer
  // separators trimmed. Mirrors the fold SymbolNameSpace::ToLowerSpace
  // applies to namespace hash keys (minus non-ASCII case folding, which no
  // known producer relies on for name matching), so two names compare equal
  // here exactly when the namespace would resolve them to the same Variable.
  // Used by XmileView's name maps and the flow->stocks association below.
  static std::string FoldNameKey(const std::string &normalizedName);

  // Folded names of the stocks that list the (folded) flow name in an
  // <inflow>/<outflow> child. Recorded by ProcessStock for XmileView's
  // pipe-endpoint resolution: a flow's pipe may only anchor on a stock it is
  // structurally connected to. Variable::Inflows()/Outflows() cannot serve
  // here -- they are populated by MarkStockFlows, which runs post-parse,
  // after the views have already been processed. Returns nullptr when the
  // flow is not referenced by any stock.
  const std::vector<std::string> *StocksForFlow(const std::string &foldedFlowName) const;

  // Reads one <group> element. Reuses an existing same-named ModelGroup when
  // present (the writer can emit the same group from multiple views), otherwise
  // creates one. Member <var> children whose names don't yet resolve get
  // placeholder Variables -- MarkVariableTypes (run after parse) classifies them
  // later.
  //
  // Public so XmileView's pass-3 group walk can delegate the writer-emitted
  // <var>name</var> shape to the same code path that handles the top-level
  // <views><group> shape, instead of duplicating the lookup/create/membership
  // logic across two call sites.
  ModelGroup *ProcessGroup(tinyxml2::XMLElement *groupEl, std::vector<std::string> &errs);

  // Find-or-create the ModelGroup named `normName` (a NormalizeName'd string)
  // and record the group's owner="..." claim, if any, for deferred resolution.
  // `ownerAttr` is the raw attribute value or nullptr when the element carries
  // none. Every group the reader registers -- from either <group> shape -- goes
  // through here, so the owner claim is captured exactly once per call site.
  ModelGroup *RegisterGroup(const std::string &normName, const char *ownerAttr, std::vector<std::string> &errs);

  // Make `v` a member of `group`, unless it already belongs to one.
  //
  // A Variable belongs to at most ONE group: Variable::SetGroup holds a single
  // pointer, and both writers walk ModelGroup::vVariables to decide what to emit
  // where (MDLGenerator::GenerateEquations emits one equation per membership,
  // XMILEGenerator::generateModelAsGroups one <aux>/<stock>), so a variable
  // listed twice ships a model with a duplicate definition. Membership is
  // therefore keyed off Variable::GetGroup() rather than a scan of vVariables:
  // that makes the test O(1) on a model with thousands of variables, and it
  // keeps the two halves of the relation -- the Variable's pointer and the
  // group's vector -- from ever disagreeing. Re-listing a variable in the group
  // it is already in is the ordinary multi-view shape and passes silently;
  // a second, DIFFERENT group contradicts the first and is diagnosed.
  void AddGroupMember(ModelGroup *group, Variable *v, std::vector<std::string> &errs);

  // The registered ModelGroup whose name equals norm (a NormalizeName'd
  // string), or nullptr. Shared by RegisterGroup's reuse lookup, the deferred
  // owner resolution, and XmileView's pass-3 group walk.
  ModelGroup *FindGroupByName(const std::string &norm);

private:
  // DOM walk dispatchers, invoked from ProcessFile. Each returns true on
  // success and pushes a descriptive message to errs on failure.
  // ProcessAuxOrFlow handles both <aux> and <flow>: the two tags are
  // structurally identical from the reader's perspective (one or more
  // equations attached to a Variable; diagnostics use el->Name() to keep the
  // offending tag in the message). Post-parse, MarkStockFlows reclassifies a
  // flow referenced from a stock's <inflow>/<outflow> as XMILE_Type_FLOW.
  bool ProcessSimSpecs(tinyxml2::XMLElement *simSpecs, std::vector<std::string> &errs);
  bool ProcessModelUnits(tinyxml2::XMLElement *units, std::vector<std::string> &errs);
  bool ProcessModel(tinyxml2::XMLElement *model, std::vector<std::string> &errs);

  // Record which reserved equation keywords this <model> overrides with a
  // declaration of its own, before any of its equations are parsed. See
  // DeclaresPi for why the question is settled up front rather than per
  // reference.
  void ScanForShadowedKeywords(tinyxml2::XMLElement *model);
  bool ProcessAuxOrFlow(tinyxml2::XMLElement *el, std::vector<std::string> &errs);
  bool ProcessStandaloneGf(tinyxml2::XMLElement *el, std::vector<std::string> &errs);
  bool ProcessStock(tinyxml2::XMLElement *stock, std::vector<std::string> &errs);

  // Shared declaration prologue for <aux>/<flow>/<gf>/<stock>: read the name
  // attribute, normalize it (XMILE permits line-wrapped names; Vensim
  // identifiers are single-line), find-or-create the Variable, and stamp the
  // canonical display name. Returns nullptr -- with a diagnostic naming the
  // element's tag -- on a missing name attribute or when the name collides
  // with a non-variable symbol.
  Variable *DeclareVariable(tinyxml2::XMLElement *el, std::vector<std::string> &errs);

  // Parse one <eqn> body (null text reads as empty) and forward its
  // diagnostics to the document errs channel tagged with ctxEl's context.
  // Returns nullptr when the parse failed or any forwarded diagnostic was a
  // hard error. A partially-built Expression is intentionally not deleted on
  // the hard-error path: namespace teardown owns unconfirmed allocations.
  Expression *ParseEqnFor(tinyxml2::XMLElement *ctxEl, const char *eqnText, std::vector<std::string> &errs);

  // Append the Equation `v[lhsSubs] <token> rhs` to v, building the
  // ExpressionVariable / LeftHandSide wrappers every declaration site needs.
  void AddEquationFor(Variable *v, SymbolList *lhsSubs, Expression *rhs, int token);

  // Post-DOM-walk validation: every ExpressionLookup whose target is a model
  // variable (the `table` in `table(x)`, or the explicit LOOKUP(table, input)
  // form) must resolve to a variable that was actually defined. The
  // single-argument fallback in xpyy_call optimistically lowers `name(x)` to a
  // lookup on `name` -- correct for a graphical function declared anywhere in
  // the document (forward references are legal), but a phantom when `name` is an
  // unsupported or misspelled function that is never defined. This can only be
  // judged once the whole document is parsed, so it runs at the end of
  // ProcessFile. Returns false (and appends one error per undefined target) when
  // any phantom is found. Scoped to lookup-application targets only -- ordinary
  // ghost references are left alone.
  bool ValidateLookupTargets(std::vector<std::string> &errs);

  // Emit a one-line advisory when a variable carries an XMILE <non_negative/>
  // clamp (or <non_negative>true</non_negative>). Vensim .mdl has no
  // non-negative concept, so the clamp is dropped on output; the advisory warns
  // that the converted model can go negative where the source could not. A
  // <non_negative>false</non_negative> body means the clamp is off and produces
  // no advisory. Shared by the aux/flow and stock paths.
  void WarnIfNonNegative(tinyxml2::XMLElement *varEl, Variable *v, std::vector<std::string> &errs);

  // Walks the top-level <dimensions> element under <model> (sibling of
  // <variables>). Each <dim name="X"> becomes a Variable whose first Equation
  // uses iEqType=':' with an ExpressionSymbolList RHS -- the same shape Vensim
  // produces for `X: a, b, c`. The post-parse MarkTypes flips the dim and its
  // elements to XMILE_Type_ARRAY / XMILE_Type_ARRAY_ELM; the reader never sets
  // those types directly. <dim size="N"/> expands to synthesized numeric
  // element names "1".."N", which XMILE permits as legal identifier strings.
  bool ProcessDimensions(tinyxml2::XMLElement *dimsEl, std::vector<std::string> &errs);

  // Builds a SymbolList of element Variables from a <dim>'s <elem name="..."/>
  // children. Names are looked up or created via InsertVariable so multiple
  // dims sharing an element name (e.g. two dims that both contain "a") resolve
  // to the same Variable. Returns nullptr and pushes an error if a child is
  // malformed or if there are no <elem> children and no size attribute.
  SymbolList *BuildElementList(tinyxml2::XMLElement *dimEl, std::vector<std::string> &errs);

  // Synthesizes element Variables named "1".."n" for an indexed <dim size="n"/>.
  // The XMILE writer never emits size="N" -- it always materializes the
  // elements -- but Stella authoring tools do, and the reader expands eagerly
  // so the rest of the pipeline never has to special-case indexed dims.
  // Returns nullptr with a diagnostic if any synthesized name is held by a
  // non-Variable symbol: size="n" states the extent exactly, so a short list is
  // a dimension of the wrong size rather than a partial success.
  SymbolList *BuildIndexedElementList(int n, std::vector<std::string> &errs);

  // <views> harvester: reads <group> children into ModelGroups and hands each
  // sketch <view>'s geometry to XmileView, layered on top of the same
  // ModelGroup vector.
  bool ProcessViews(tinyxml2::XMLElement *views, std::vector<std::string> &errs);

  // Settle every owner="..." claim RegisterGroup recorded, at the end of
  // ProcessFile. Deferred because XMILE fixes no order among <group> elements:
  // a child group may be declared BEFORE the parent it names, and resolving at
  // the point of the attribute would silently drop every such forward reference
  // (nothing revisits a group afterwards). Claims are settled in document order
  // and one is refused -- with a diagnostic, leaving the group unowned -- when
  // the named group does not exist, when a group names itself, or when the link
  // would close an ownership cycle. Refusing keeps the owner chain a forest,
  // which is what XMILEGenerator::generateModelAsGroups assumes when it nests
  // one <module> inside another by following pOwner.
  void ResolveGroupOwners(std::vector<std::string> &errs);

  // Helper used by ProcessSimSpecs to materialize a control Variable. The
  // writer reads control values via Model::GetConstanValue, which dispatches to
  // the variable's first equation, so the parallel Model setters and the
  // variable equations both need to be populated for round-trip parity.
  //
  // The constant Equation is attached here only when the document actually
  // SPELLED THE VALUE OUT (stated). A value the reader invented for an absent
  // <start>/<stop>/<dt>/save-interval is the weakest claim in the document and
  // must not beat an <aux> that states the same control, so it is left to
  // ApplyControlValues to attach once the whole document has been walked and
  // no declaration has claimed the slot. Returns the control Variable (whether
  // it was created here or already existed) so the caller can attach the
  // document-level time_units to it without a second namespace lookup; NULL
  // only if the name is occupied by a non-Variable symbol.
  Variable *SetControlVariable(const std::string &name, double value, bool stated);

  // The four Vensim control variables, in the order kControlNames
  // (XmileReader.cpp) spells them and _controls indexes them. INITIAL TIME /
  // FINAL TIME / TIME STEP come first because each has a matching Model field
  // and SAVEPER, which has none, defaults to whatever dt settles at.
  enum ControlIndex { kInitialTime = 0, kFinalTime, kTimeStep, kSaveper, kControlCount };

  // Index into kControlNames (XmileReader.cpp) of the Vensim control variable
  // `name` denotes -- INITIAL TIME, FINAL TIME, TIME STEP, SAVEPER -- or -1 for
  // an ordinary name. Matching goes through SymbolNameSpace::ToLowerSpace, the
  // namespace's own identifier equivalence, so the underbar spelling that
  // SDEverywhere and PySD exports use (TIME_STEP) is the same name as the one
  // <sim_specs> registers. MDLGenerator::IsControlVar decides the identical
  // question the same way on the way out; the two must agree, because a name
  // this misses but IsControlVar catches gets filtered out of the main equation
  // section and then emitted from .Control once per stored equation.
  static int ControlIndexOf(const std::string &name);

  // Dispose of a <variables> declaration -- <aux>, <flow>, <stock>, or <gf> --
  // whose name resolves to one of the four Vensim control variables. Returns
  // true when the declaration has been fully handled here and the caller must
  // attach NO equation of its own; false when el is not a control declaration
  // at all, or when it is the one shape that may legitimately supply a control
  // value the document has left unstated (see the .cpp for the precedence
  // rule). Callers invoke it straight after DeclareVariable, before any
  // equation is built.
  bool ProcessControlDeclaration(tinyxml2::XMLElement *el, Variable *v, std::vector<std::string> &errs);

  // Give every control variable exactly the one equation it is missing and make
  // the engine's own time fields agree with the constant that equation holds.
  // Runs at the end of ProcessFile, for the same reason ApplyDeferredTimeUnits
  // does: only then is it known whether a <variables> declaration supplied a
  // value <sim_specs> never stated.
  void ApplyControlValues();

  // One control variable's share of ApplyControlValues: attach `fallback` if
  // the document left the variable equation-free, then push the resulting
  // constant into the matching Model field. idx indexes kControlNames.
  void SettleControl(int idx, double fallback);

  // Build the INTEG arg-0 expression for a stock: sum(inflows) - sum(outflows)
  // as a left-leaning tree of +/- ops over bare ExpressionVariable leaves.
  // The shape is dictated by is_all_plus_minus (src/Symbol/Expression.cpp);
  // MarkStockFlows walks this tree to populate stock->Inflows()/Outflows().
  // With no inflows, seeds the accumulator with literal 0 so any outflows
  // produce a well-formed 0 - O1 - O2 ... chain. The flow Variables are
  // created on miss via InsertVariable so a stock can forward-reference a
  // <flow> declared later in the document. When lhsSubs is non-null each
  // flow reference receives an independent Clone of it (sharing would
  // double-free at namespace teardown -- see SymbolList::Clone).
  // Returns nullptr (with a diagnostic in errs) when a flow name is held by a
  // non-Variable symbol; that name can never become a flow, and the net-flow
  // expression IS the stock's equation, so there is no partial result worth
  // keeping -- the declaration form of the same collision already fails the
  // conversion through DeclareVariable.
  Expression *BuildNetFlowSubscripted(const std::vector<std::string> &inflowNames,
                                      const std::vector<std::string> &outflowNames, SymbolList *lhsSubs,
                                      std::vector<std::string> &errs);

  // Wrap the net-flow + init expressions into an INTEG ExpressionFunctionMemory.
  // Extracted as a helper so the scalar and per-element subscripted stock paths
  // share the construction. Returns nullptr (and pushes an error to errs) only
  // if the INTEG function table is missing -- normally the XmileReader ctor
  // seeds it, so a nullptr return signals a programmer error elsewhere.
  Expression *BuildIntegExpression(Expression *netFlow, Expression *init, std::vector<std::string> &errs);

  // Attach a variable-level <units> string. SetUnitsString carries the raw
  // text for the writer's fallback path; this helper also parses the slash-
  // and-star-separated tokens into a UnitExpression so the comparator (and any
  // downstream consumer that reads Variable::Units()) sees the same canonical
  // form a Vensim parse of the equivalent .mdl would produce. Skipping the
  // parse would leave Variable::Units() NULL on the XMILE side and force the
  // comparator to compare raw text against `GetEquationString()` -- two
  // semantically equal but textually different shapes -- on a round trip.
  //
  // FIRST source wins, and it wins for BOTH halves: a Variable that already
  // carries units is left untouched. Variable::AddUnits has no replace -- it
  // refuses a second UnitExpression -- so a helper that wrote the raw string
  // unconditionally would leave the two halves describing different sources,
  // and since both writers read the parsed half first the raw text would be the
  // half silently discarded. Precedence therefore lives entirely in the CALL
  // ORDER; see ApplyDeferredTimeUnits.
  void AttachVariableUnits(Variable *v, const char *unitsText);

  // Apply the document-level <sim_specs time_units="..."> to the control
  // variables, as the default for those that did not spell out their own
  // <units>. Runs at the end of ProcessFile rather than inside ProcessSimSpecs:
  // the envelope loop dispatches <sim_specs> before <model>, so at sim_specs
  // time no per-variable <units> has been read yet and an "only if absent" test
  // there would be vacuous. Deferring also makes the precedence independent of
  // where <sim_specs> sits relative to <model> in the document.
  void ApplyDeferredTimeUnits();

  // Attach a variable-level <units> child (via AttachVariableUnits) and a
  // <doc> child (as the Variable's comment). Shared by the apply-to-all,
  // per-element, and stock paths, which all read these two children the same
  // way off the variable's element.
  void AttachUnitsAndDoc(tinyxml2::XMLElement *varEl, Variable *v);

  // Parse a "/"/"*"-separated unit text into a UnitExpression. Empty or
  // whitespace-only input returns nullptr. Each token becomes one Units symbol
  // (via FindOrInsert under the ">"-prefixed namespace key the Vensim parser
  // uses); the result is left-associative so "a/b/c" parses as "(a/b)/c", i.e.
  // numerator a, denominator b*c -- identical to what VensimParse builds for
  // the same source text.
  UnitExpression *ParseUnitsString(const std::string &text);

  // Apply-to-all and per-element shared paths used by ProcessAuxOrFlow.
  // ProcessAppliesToAllEquation handles the scalar case (dimsChild == nullptr)
  // as well as the arrayed apply-to-all case; ProcessPerElementEquations emits
  // one Equation per <element subscript="...">.
  bool ProcessAppliesToAllEquation(tinyxml2::XMLElement *varEl, Variable *v, tinyxml2::XMLElement *dimsChild,
                                   std::vector<std::string> &errs);
  bool ProcessPerElementEquations(tinyxml2::XMLElement *varEl, Variable *v, std::vector<std::string> &errs);

  // Parse an <element subscript="a, b"> attribute into a SymbolList. The
  // separator follows XMILEGenerator (comma-space); for robustness against
  // single-comma writers we accept either. Each name is looked up via
  // InsertVariable so the same element Variable is shared between the
  // dimension definition and the subscripted reference.
  SymbolList *ParseSubscriptList(const std::string &subscriptAttr);

  // Build a SymbolList from a per-variable <dimensions><dim name="X"/>... child.
  // The names are dim Variables (not element Variables); the apply-to-all
  // semantics propagate through MarkTypes the same way Vensim handles
  // \`x[Dim] = ...\`.
  SymbolList *BuildAppliesToAllSubs(tinyxml2::XMLElement *dimsEl, std::vector<std::string> &errs);

  // Parse an XMILE <gf> element into an ExpressionTable*. Returns nullptr (and
  // appends a diagnostic to errs) on malformed input. Handles the two corpus
  // shapes for x points: explicit <xpts> and the <xscale min max>-derived even
  // spacing fallback (used by the fishbanks corpus). type="extrapolate" sets
  // the matching flag; type="discrete" is logged and treated as continuous
  // (xmutil has no discrete representation). Length mismatches between xpts
  // and ypts are truncated to min(|xs|, |ys|) with a warning, mirroring
  // MDLFormat::WriteLookupBody's defensive behavior.
  ExpressionTable *ProcessGf(tinyxml2::XMLElement *gf, std::vector<std::string> &errs);

  Model *_model;
  SymbolNameSpace *pSymbolNameSpace;

  // See StocksForFlow.
  std::unordered_map<std::string, std::vector<std::string>> _flowToStocks;

  // Unsettled group -> owner-name claims, in first-claim order. See
  // ResolveGroupOwners. A vector keeps the resolution (and so the diagnostics)
  // in document order; the linear scan RegisterGroup does over it is over the
  // model's GROUPS, of which even the largest corpus models have a few dozen.
  std::vector<std::pair<ModelGroup *, std::string>> _pendingGroupOwners;

  // The <sim_specs time_units="..."> text, held until the whole document has
  // been walked. See ApplyDeferredTimeUnits. _haveTimeUnits distinguishes an
  // absent attribute from an empty one.
  std::string _timeUnits;
  bool _haveTimeUnits;

  // What the document says about one control variable, held for the whole parse
  // so that a <variables> declaration of the same name can be measured against
  // it (ProcessControlDeclaration) and so the value can be attached once
  // everything is known (ApplyControlValues). `var` is null until <sim_specs>
  // materializes the Variable -- and stays null if the name is occupied by a
  // non-Variable symbol. `stated` records whether the document spelled the
  // value out, as opposed to the reader having defaulted it.
  // `pendingUnitsAndDoc` is a declaration element whose <units>/<doc> could not
  // be attached when it was walked: a Variable allocates the content that holds
  // the parsed UnitExpression only when its first equation arrives
  // (Variable::AddEq), and a control still waiting for ApplyControlValues has
  // none. SettleControl re-visits it. The pointer stays valid because the
  // tinyxml2 document outlives the whole of ProcessFile, epilogue included.
  struct ControlVar {
    Variable *var;
    double value;
    bool stated;
    tinyxml2::XMLElement *pendingUnitsAndDoc;
  };
  ControlVar _controls[kControlCount];

  // Equation-parser scratch slots. The reader owns no Expression* directly;
  // _lastParsedExpr is just a hand-off between the bison reduction and
  // ParseEquation's return statement. _currentLex / _currentErrs are
  // borrowed pointers to stack objects inside the active ParseEquation call.
  Expression *_lastParsedExpr;
  XmileEqLex *_currentLex;
  std::vector<std::string> *_currentErrs;

  // See DeclaresPi. Set by ScanForShadowedKeywords before the first equation
  // of the <model> is parsed.
  bool _declaresPi;
};

// Process-global pointer to the active reader. The equation parser uses it to
// resolve names through the reader during yacc actions, mirroring
// VensimParse's VPObject.
extern XmileReader *XPObject;

#endif
