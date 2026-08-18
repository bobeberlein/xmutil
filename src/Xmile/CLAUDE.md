# Xmile reader + writer

Last verified: 2026-08-17

## Purpose
Both directions of the XMILE side of xmutil live here. The writer
(`XMILEGenerator`) serializes an in-memory `Model` to an XMILE 1.0 document --
inverse of the Vensim/Dynamo readers under `src/Vensim/` and `src/Dynamo/`.
The reader (`XmileReader` + `XmileView` + `XmileEqLex` + `XmileEqYacc.y` +
the `xmile::` helpers) walks an XMILE document and populates a `Model` --
inverse of `XMILEGenerator` and a sibling to `VensimParse`. This subsystem
is the only place that links the engine to the XMILE wire format in either
direction.

## Contracts
- **Exposes**:
  - `XMILEGenerator(Model*, xscale, yscale, fromDynamo)` and `Print(...)`
    -> XMILE document, empty string with messages in `errs` on error.
    Reached from `Model::PrintXMILE`.
  - `XmileReader(Model*)` and `ProcessFile(filename, contents, len, errs)`
    -> populates Model from one XMILE document. Mirrors
    `VensimParse::ProcessFile`, plus an explicit `errs` channel.
  - `Model::ParseXMILE(...)` constructs an `XmileReader` and drives one parse.
  - `XmileReader::ParseEquation(text, errs)` parses one `<eqn>` body into a
    caller-owned `Expression*`, or NULL with a diagnostic.
  - State-free static helpers on `XmileReader`: `IsForeignNamespace`,
    `IsStellaUIWidget`, `NormalizeName` (shared with `XmileView`).
  - `convert_xmile_to_xmile` / `convert_xmile_to_mdl` (`src/XMUtil.{h,cpp}`)
    return strdup'd heap strings or NULL on parse failure. Same ownership
    as `convert_mdl_to_xmile` / `convert_to_mdl`. `isLongName` is accepted
    for API symmetry but not consumed -- XMILE has no long/short
    distinction. `convert_xmile_to_xmile`'s `isAsSectors` is likewise
    accepted and not consumed: XMILE input is always emitted in the
    single-`<model>` sector form (see Key Decisions).
  - `xmile::LookupFunction`, `LookupBareKeyword`, `ReorderArgs`
    (`XmileFunctions.{h,cpp}`) translate XMILE names to Vensim names.
    `XmileParseFunctions.{h,cpp}` holds the yacc action shims; they reach
    the active reader through process-global `XPObject` (mirroring
    `VensimParseFunctions`'s `VPObject`).
- **Guarantees** (reader): emitted XMILE re-parses into a structurally
  equivalent Model (tests in `test/xmile/`); XMILE->MDL also round-trips
  structurally on the corpus. Unsupported envelope shapes get a descriptive
  `errs` message and the parse fails: `<macro>`, `<module>`, multiple
  `<model>` siblings. Vendor-namespaced elements (any tag with `:` --
  `isee:`, `simlin:`, ...) and the documented Stella UI widgets
  (`animation_object`, `button`, `gauge`, `graph`, `knob`, `loop_indicator`,
  `numeric_display`, `numeric_input`, `slider`, `spatial_map`) drop
  silently; unknown default-namespace envelope elements also drop, to
  leave the door open for future XMILE additions.
- **Guarantees** (writer): XMILE -> XMILE emits exactly ONE `<model>` element,
  so the emission always satisfies the reader's own envelope rule and can be
  re-imported (see Key Decisions). And XMILE -> XMILE is a FIXPOINT -- converting
  a document this writer produced reproduces it byte for byte. That is a
  stronger claim than the structural round trip above and is what makes the
  conversion a normalization rather than a drift; it is asserted over the whole
  corpus allow-list by `XmileCorpus_allowlist_normalization_is_a_fixpoint` and
  pinned construct-by-construct in `test/xmile/FixpointTest.cpp`. Two things it
  rests on, both easy to break: `Model::GetVariables` orders by name (see Key
  Decisions), and `ExpressionParen::OutputComputable` does not re-wrap a child
  whose rendering is already one enclosing group (see Gotchas).
- **Expects**: callers run `Model::RunPostParsePipeline`
  (`ConfirmAllAllocations`, `MarkVariableTypes` for main + every macro,
  `AdjustGroupNames`, `CheckGhostOwners`, `ResolveWildcardSubscripts`)
  before serialization. The reader
  does not -- matching the `VensimParse` contract. Skipping leaves
  unconfirmed allocations (XMILE parsing produces these; Vensim parsing
  does not) and unclassified Variable types.

## Dependencies
- **Uses**: `Model`, `Expression` nodes and `EXPTYPE` tags (`src/Symbol/`),
  `VensimView` (the reader populates a Vensim sketch under the hood -- see
  Key Decisions), the function registry seeded by `RegisterXmutilFunctions`
  (free function in `src/Vensim/VensimParse.cpp`), and the system `tinyxml2`
  library for DOM walking.
- **Used by**: `Model::PrintXMILE`, `Model::ParseXMILE`, the four
  `convert_*` extern-C entries in `src/XMUtil.cpp`, `Main.cpp`'s
  extension-dispatch, and `test/xmile/`.
- **Boundary**: do not pull simlin code in -- the reader was patterned after
  the simlin XMILE reader but must stand alone against xmutil's own Model,
  Expression tree, function registry, and view geometry.

## Key Decisions
- **Module decomposition is offered to Vensim/Dynamo input only; XMILE input is
  always emitted as sectors.** `XMILEGenerator` has two shapes.
  `generateModelAsModules` / `generateModelAsGroups` write a base `<model>` plus
  one SIBLING `<model>` per view (two or more views) or per populated group (two
  or more groups), tied together by `<module>` references;
  `generateModelAsSectors` writes one `<model>` whose `<views>` carry the groups.
  `Model::PrintXMILE` chooses. Rolling a model up into Stella modules adds
  structure Vensim and Dynamo cannot express, so it is a nicety there -- but
  `XmileReader::ProcessFile` refuses a document with sibling `<model>` elements
  (that shape means `<module>` submodels, out of scope), so applied to XMILE
  input it is not a normalization at all: `XMUtil model.xmile` on any model with
  two populated groups emitted a document the very next run rejected with
  "multiple `<model>` elements are not supported". `Model::ParseXMILE` therefore
  records `bFromXmile` and `PrintXMILE` emits `bAsSectors || bFromXmile`.
  A separate flag rather than having the parse write `bAsSectors`: `bAsSectors`
  is the CALLER's stated preference and stays the caller's to set in either
  order, so folding provenance into it would make the answer depend on whether
  `SetAsSectors` ran before or after the parse. It mirrors `bFromDyanmo`, which
  is already a parse-recorded fact this writer reads back. The cost is real and
  bounded: `generateSectorViews` writes its `<group>` membership list only for a
  model with NO view, so a model with both a sketch `<view>` and groups loses the
  grouping on the way out (`simlin-test/land_model`, and the reason it is still
  in the corpus deferred table). Hoisting that emission out of the `views.empty()`
  guard is not free -- it changes the `.mdl` -> XMILE default output for most of
  the `.mdl` corpus -- so it is its own piece of work. Regression tests:
  `test/xmile/SingleModelNormalizationTest.cpp`, which also pins that
  `convert_mdl_to_xmile` still decomposes.
- **Cross-view flow proxies are made on the module path only, at emission
  time.** One `<model>` per view means a stock's flows must be defined in the
  stock's own `<model>`, so for each inflow/outflow drawn in another view the
  module path stands in a local `"<stock> flow" = <the modeler's flow>` proxy
  (`Variable::PreventFlowGhost`, from upstream). `XMILEGenerator::Print` runs
  `Model::LocalizeCrossViewFlows` right before `generateModelAsModules`, NOT in
  the post-parse pipeline: the pipeline cannot know the output target, and the
  proxies are wrong everywhere else -- in the sector form they are pointless
  structure that also breaks the XMILE -> XMILE fixpoint (a flow the sketch does
  not place has no view, so it counted as "another view"), and in `.mdl` output
  they are variables the source never had, minted again on every re-read. A
  displaced flow that no stock lists any more gets the type `MarkTypes` gave it
  back (`UndoFlowPromotion`; a plain-aux drawing emits as `<aux>`, not as an
  unattached `<flow>` with invented `pts`), decided after every stock is
  processed so a flow another stock still lists in its own view stays a flow.
  Proxies carry the `Variable::SynthesizedFlowProxy` provenance flag, which the
  `.mdl` writer uses to leave them out should a model be printed both ways.
  Regression tests: `test/mdl/CrossViewFlowTest.cpp`.
- **`Model::GetVariables` sorts by name, at the source rather than per
  writer.** The namespace hash table's bucket order records insertion history,
  and it reaches emitted output through `generateModelAsSectors` /
  `generateDimensions` / `generateModelAsGroups` / `generateModelAsModules`
  (the last two also *assign* views and groups while walking it, so the
  assignment inherited the same nondeterminism). Sorting inside `GetVariables`
  rather than at each XMILE call site is what makes the property hold for
  callers that do not exist yet -- the same reasoning as `SymbolNameLess` for
  the pointer-keyed sets. The `.mdl` writer's own explicit sorts are now
  redundant but harmless, and on the DEFAULT (short-name) path its output is
  byte-unchanged by this. It is NOT unchanged under `--longnames`, and that is
  by design rather than an oversight: the long-name pass (`VensimParse.cpp`,
  `DynamoParse.cpp`) walks `GetVariables` renaming each variable to its `~`
  comment through `SymbolNameSpace::Rename`, which refuses a name already
  taken -- so when several variables share one comment the FIRST one walked
  keeps it and the rest keep their original names. Sorting changes who is
  first, so it changes emitted variable NAMES: `C-LEARN v77` has 15 comments
  shared by two or more variables, and Dynamo input has long names on by
  default. Nothing is lost -- the previous winner was whichever variable the
  hash happened to reach first, which is exactly the nondeterminism this sort
  exists to remove -- but "byte-unchanged" is a claim about the default path
  only.
- **The XMILE reader populates `VensimView`, not a new view class.** The
  writers in `src/Mdl/` and `src/Xmile/` see exactly one view
  representation. Programmatic ctors on `VensimValveElement`,
  `VensimCommentElement`, and `VensimConnectorElement` (the last with
  polarity) were added so the XMILE shape and the parse-driven sketch shape
  coexist.
- **The equation parser is a separate bison/flex pair** (`XmileEqYacc.y` +
  `XmileEqLex`), not the Vensim one: XMILE equation grammar differs at the
  lexical and function-name layer (lowercase, underbar-as-space,
  square-bracket subscripts). Generated `XmileEqYacc.tab.{cpp,hpp}` are
  committed; regenerate from `.y`, do not hand-edit. `format.sh` excludes
  them from clang-format -- clang-format is not idempotent on bison output.
- **Function name translation is table-driven** (`xmile::LookupFunction` /
  `xmile::ReorderArgs`). Tables mirror the un-rename rules in the simlin
  writer (the design and implementation plans that spell them out were kept
  out of the tree; see this branch's git history for `docs/`).
  Arg reorder applies only where XMILE and Vensim signatures disagree
  (DELAY N, SMOOTH N, RANDOM NORMAL). The underbar->space + uppercase
  fallback path is intentional.
- **`SymbolList::Clone` deep-copies a SymbolList** because one synthesized
  INTEG expression for an arrayed stock holds several `ExpressionVariable`
  children, each owning its `SymbolList` outright (its dtor deletes it).
  Sharing one pointer across owners would double-free at namespace
  teardown. The Vensim grammar rebuilds per call site; the DOM walker has
  nowhere to re-run, so it Clones.
- **`Model::RunPostParsePipeline` is the shared post-parse sequence.** Six
  call sites (the four `convert_*` entries plus the two round-trip test
  helpers) all need it; consolidating prevents drift.
  `ConfirmAllAllocations` is part of the sequence because XMILE parsing
  leaves objects unconfirmed (Vensim parsing confirms after each equation);
  without it, a later exception path would discard XMILE-built objects.
- **`RegisterXmutilFunctions(sns)` is a free function** extracted from
  `VensimParse::ReadyFunctions`. Invoked from both the Vensim parser and
  the `XmileReader` ctor so both readers populate identical function
  registries without sharing a base class.

## Key Files
- `XMILEGenerator.{h,cpp}` - the `Model` -> XMILE walker.
- `XmileReader.{h,cpp}` - the XMILE -> `Model` walker (envelope + DOM
  dispatch, `<header>`, `<sim_specs>`, `<model_units>`, `<dimensions>`,
  `<aux>`/`<stock>`/`<flow>`, INTEG synthesis, `<gf>` lookups).
- `XmileView.{h,cpp}` - one `<view>` -> one `VensimView`, in three passes
  (allocate elements with cloud synthesis at unmatched flow endpoints,
  resolve connectors, walk groups).
- `XmileEqLex.{h,cpp}`, `XmileEqYacc.y`, `XmileEqYacc.tab.{cpp,hpp}` - the
  equation parser; generated tables committed.
- `XmileFunctions.{h,cpp}` - XMILE -> Vensim name + arg-order translation
  (`xmile::` namespace; pure lookups).
- `XmileParseFunctions.{h,cpp}` - yacc action shims; reach the active
  reader through process-global `XPObject`.

## Gotchas
- **Parentheses in the XMILE serialization have two owners, and only one of
  them may nest.** The binary-operator nodes
  (`EO2SubClass*` in `src/Symbol/Expression.h`) emit `e1 op e2` with no parens
  at all, so grouping survives a round trip ONLY as an `ExpressionParen` node
  built from a literal `(` in the source -- dropping one reassociates the
  expression. Separately, several `Function::OutputComputable` overrides wrap
  their whole rendering (`( IF c THEN a ELSE b )`, SAMPLE IF TRUE, the expanded
  PULSE / PULSE TRAIN forms, RANDOM BINOMIAL). Composing the two double-wrapped:
  the reader read the function's own parens back as a grouping node and the
  writer re-wrapped, growing one layer per conversion, forever.
  `ExpressionParen::OutputComputable` therefore suppresses its emission when the
  child's rendering is already one enclosing group, decided from the RENDERED
  TEXT (`IsWholeParenGroup`), not from the node kind: self-delimitation depends
  on the function, on its arity, and for `FunctionMemoryBase` on which arguments
  the current `ContextInfo` selects, so a node-kind table that missed one case
  would leave that construct growing unbounded. Reading the text has its own
  failure mode, and it is the dangerous one -- the predicate has to LEX the
  text the way the re-parser will, not count characters. A quoted identifier
  carries its parens as name characters (`"x("`), and counting those as
  grouping both dropped needed parens (one unmatched `(` in one name and `)`
  in another let the depth reach zero at the end of a string that was not one
  group) and, from a single unmatched paren, made every string look unbalanced
  so the redundant layer never collapsed. `IsWholeParenGroup` therefore skips
  quoted spans the way `XmileEqLex::ScanQuotedSymbol` does and answers "keep
  the paren" for anything it cannot lex confidently (a backslash inside a
  quoted name, where the Vensim and XMILE lexers disagree about the closing
  quote). The apostrophe is the character with more than one reading -- a
  Vensim literal delimiter (`GET DIRECT DATA('f(x).xlsx',...)` puts parens
  inside one) and a legal bare-name character (`don't`) -- so the predicate
  SCANS TWICE, once under each reading, and suppresses only when every reading
  the text admits agrees. A lone apostrophe closes no literal, so that text
  admits only the bare-name reading and the question is settled rather than
  abandoned; where a literal's parens would change the count the two readings
  disagree and the paren is kept. Refusing the character outright, which is what
  came before, answered "keep" for every literal-bearing rendering and left the
  self-delimiting ones carrying a spare layer. Adding a rendering that can emit
  an unmatched paren OUTSIDE a quoted span means revisiting it. This
  path is XMILE-only -- the `.mdl` writer unwraps `ExpressionParen` outright
  and re-derives its parens from precedence (`src/Mdl/CLAUDE.md`).
- **A name that needs quoting only survives an XMILE round trip if it arrived
  quoted.** `Variable::OutputComputable` emits `SpaceToUnderBar(alternate
  name)` and adds no quotes of its own, so the quotes in an emitted `<eqn>`
  are the ones the Vensim reader stored (it keeps a quoted identifier in
  source form). The XMILE reader strips them (`XmileEqLex::ScanQuotedSymbol`
  interns the interior), so a name like `x(` read FROM XMILE is written back
  bare and the next read misparses it as a call. Bounded to names that are not
  bare-legal identifiers, which is why the quoted-name paren guards in
  `test/xmile/FixpointTest.cpp` close their loop through `.mdl` (whose writer
  does re-quote) instead of XMILE -> XMILE.
- `convert_xmile_*` return strdup'd heap strings; the caller must `free`
  them, the same as `convert_to_mdl` / `convert_mdl_to_xmile`. Advisory
  reader diagnostics (discrete `<gf>`, multi-view skip, ...) are logged and
  cleared after a successful parse -- only Print-phase errors return NULL.
  All four entry points catch exceptions at the extern-C boundary.
- **Equation-diagnostics split: "warning: " prefix = advisory, else fatal.**
  The bison shims (`XmileParseFunctions.cpp`) can push a diagnostic *and* return
  a placeholder Expression, so a diagnostic on a successfully-parsed equation
  used to be silently dropped. `ForwardEqnDiagnostics` (`XmileReader.cpp`) now
  forwards every per-eqn diagnostic to the document errs and fails the parse if
  any is unprefixed; genuine advisories (dropped `<non_negative>` clamp) carry a
  leading `warning: `. An unknown multi-arg call is a hard error;
  a single-arg unknown call is lowered to a lookup and caught later (below).
- **Function arity is a `[min, max]` range; the reduced form is padded and a real
  mismatch is FATAL.** `xpyy_call` checks the arg count against
  `Function::MinNumberArgs()`/`MaxNumberArgs()` (both default to `NumberArgs()`;
  `RegisterXmutilFunctions` widens the lower bound for builtins whose trailing arg
  XMILE may omit -- DELAY FIXED, TREND, RAMP, RANDOM UNIFORM all accept
  `[NumberArgs-1, NumberArgs]`). For an in-range-but-short call, `PadOptionalArgs`
  synthesizes the missing trailing arg so the stored AST is full-arity and both
  writers emit VALID Vensim (`delay(x,d)` -> `DELAY FIXED(x,d,x)` with the input
  deep-copied via `Expression::Clone`; `trend`/`uniform` -> trailing `0`; `ramp`
  -> trailing `FINAL TIME`), so the accept-set is genuinely valid-Vensim, not
  merely tolerated. A count outside the range is an unprefixed (fatal) diagnostic
  naming the resolved Vensim function, the range, and the got-count -- a
  wrong-arity call would emit `.mdl` Vensim rejects. Padding and the check both
  run AFTER `xmile::ReorderArgs`, so they see the Vensim-signature count.
  Reorder-dependent builtins (DELAY N, SMOOTH N, RANDOM NORMAL) stay strict
  because their reduced-arity form cannot be soundly reordered/padded. `min`/`max`
  resolve by arity in `LookupFunction` (1 arg -> VMIN/VMAX array reducer, 2 args
  -> MIN/MAX scalar pair), so neither needs a widened range.
- **XMILE `pi()` and bare `pi` lower to a numeric literal, unless the document
  declares its own `pi`.** Vensim has no PI builtin, so `xpyy_call` (call form)
  and `xpyy_resolve_symbol` (bare, no subscripts, case-insensitive) both return
  `ExpressionNumber(3.141592653589793)`.
  `ExpressionNumber::OutputComputable` emits the shortest round-trip-exact decimal
  (`std::to_chars`, like `mdl::FormatMDLNumber`), not the ostream default 6 sig
  figs, so an XMILE->XMILE round trip of a full-precision constant does not drift.
  Reifying UNCONDITIONALLY (like Stella / simlin's `reify_0_arity_builtins`) was
  not dead code, though: XMILE reserves the name but nothing stops a document
  from declaring `<aux name="pi">`, and such a document emitted `pi = 7`
  alongside `area = 3.141592653589793 * 2` -- the declaration kept, every
  reference to it rewritten, no diagnostic, exit status 0. Both shims now
  consult `XmileReader::DeclaresPi()`, settled by `ScanForShadowedKeywords`: a
  pre-pass over the `<model>`'s `<variables>` children that runs before any of
  its equations are parsed, rather than a namespace lookup at the reference.
  XMILE fixes no order between a declaration and a reference, so deciding at the
  reference would resolve the first half of a document to the constant and the
  second half to the variable, purely on element order. Scoped to `<variables>`
  because dimension and element Variables are reachable only from subscript
  positions, which the grammar routes away from the keyword check. Regression
  tests: `test/xmile/PiKeywordTest.cpp`, which pins both orderings and the
  undeclared case.
- **`ValidateLookupTargets` runs at the end of `ProcessFile`.** Every
  `ExpressionLookup` whose target variable ended the parse with no equation
  (`table(x)` applied to a never-defined `table`) is a phantom and fails the
  conversion. Forward references to a `<gf>` declared later are legal, so this
  can only run once the whole document is walked. Scoped to lookup-application
  targets -- ordinary ghost references are untouched.
- **XMILE `pulse()` is translated structurally, not by name.** XMILE PULSE is an
  area impulse; `BuildXmilePulse` (`XmileParseFunctions.cpp`) lowers
  `pulse(v, first[, interval])` to `(v / TIME STEP) * PULSE(first, TIME STEP)`
  (or `PULSE TRAIN(first, TIME STEP, interval, FINAL TIME)`; interval literal
  <= 0 degrades to the single-pulse form). The XMILE writer re-expands Vensim
  PULSE/PULSE TRAIN to an IF-expression, so an XMILE->XMILE round trip of a
  `pulse()` model does not reconstruct the call (structure differs, semantics
  preserved -- `input_functions` stays deferred for this reason).
- **Flow-pipe encoding convention**: a flow's sketch pipe is TWO connector
  records that BOTH originate at the valve (`From() == valve uid`, where the
  valve sits at `flow-var uid - 1`). Real Vensim sketches encode pipes the
  same way (see the simlin SIR.mdl testdata), and `XMILEGenerator`'s `<pts>`
  reconstruction depends on it (the `From() == local_uid - 1` predicate); if
  either side changes how pipe connectors are allocated, the other must
  follow. The `.mdl` writer holds up the other end: `SuppressedSketchSlots`
  (`src/Mdl/CLAUDE.md`) never emits an attached valve without the variable
  record that belongs at `valve_uid + 1`.
- **`generateView` treats every UID off the wire as untrusted, because the
  element vector is genuinely sparse.** `VensimView::ReadView` leaves a NULL in
  every slot no record claims, so a hand-authored `.mdl` may skip UIDs freely,
  and a connector's `From()`/`To()` -- plus the `+ 1` flow pairing above -- are
  raw file data that can name an empty slot or one past the end. Every index
  into `elements` therefore goes through the local `at()` helper, and an endpoint
  re-pointed from a valve to `valve_uid + 1` is RE-TESTED rather than assumed to
  be a variable: re-pointing without re-testing was a plain segfault on ordinary
  user input, and was also what a `.mdl` with a missing flow record walked into.
  For the same reason `Type()` is checked on the base pointer BEFORE the
  downcast; casting first and asking afterwards reads the wrong object's vtable.
  Regression tests: `test/mdl/SketchRoundTripTest.cpp`.
- Pipe endpoints anchor only on stocks structurally connected to the flow
  (the `<inflow>`/`<outflow>` lists, recorded by `ProcessStock` into the
  reader's `StocksForFlow` map because `Variable::Inflows()` is not populated
  until the post-parse pipeline); geometry picks which of the at-most-two
  candidate stocks each endpoint touches, and anything else is a cloud.
- View name maps key through `XmileReader::FoldNameKey` (ASCII lower,
  `_`/space runs folded), mirroring the namespace's `ToLowerSpace` fold, so
  connector endpoints resolve in the sketch whenever they resolve in the
  namespace.
- **A `<group owner="...">` is settled at the END of the parse, and a variable
  belongs to exactly one group.** Both facts are read back out by both writers --
  `generateSectorViews` emits `owner="..."` and the `<var>` membership list (the
  path everything read from XMILE takes), `generateModelAsGroups` follows
  `pOwner` to nest one `<module>` inside another and walks `vVariables` to decide
  which `<model>` each variable's equation lands in (the Vensim/Dynamo path)
  -- and `MDLGenerator::GenerateEquations` emits one equation per membership.
  So a mistake in either one ships a broken model rather than a cosmetic
  difference. Owner resolution is deferred (`ResolveGroupOwners`, called from
  `ProcessFile`'s epilogue) because XMILE fixes no order among `<group>`
  elements: resolving at the attribute meant a child declared before its parent
  looked up a group that did not exist yet, and nothing revisited it, so half the
  legal orderings silently lost the nesting. Claims are settled in document order
  and one is refused -- diagnosed, group left unowned -- when the owner names no
  group, when a group names itself, or when the link would CLOSE a cycle;
  refusing only the closing link keeps the chain a forest (which is what
  `generateModelAsGroups` assumes) while honoring as much of the stated nesting
  as it can. Membership goes through `AddGroupMember`, which treats
  `Variable::GetGroup()` as the membership index rather than scanning
  `vVariables`: that is O(1) on a model with thousands of variables, and it keeps
  the Variable's single group pointer and the group's vector from disagreeing.
  Re-listing a variable in the group it is already in is the ordinary multi-view
  shape (`XMILEGenerator` emits the same group under every view it appears in)
  and is silent. A second, DIFFERENT group is something the XMILE DOCUMENT can
  state perfectly well and the in-memory `Model` cannot hold -- `Variable` has
  one `pGroup` and both writers read exactly one membership back out -- so the
  first claim wins (matching the Vensim reader, where the banner above the
  equation decides), the second is diagnosed, and the second membership is LOST:
  an XMILE -> XMILE round trip of such a document emits the variable under one
  group only. Before this, each re-listing appended again, and
  `simlin-test/land_model/land_model.stmx` really did emit `gdp deflator` three
  times and `Agriculture Land` twice in one `.mdl`.
  `XmileView::ProcessViewGroups` routes its `<item uid="N"/>` membership shape
  through the same two helpers, so the `<var>` and `<item>` shapes cannot drift.
  One asymmetry between the shapes is deliberate: an `<item>` that resolves to
  an `<alias>` is a GHOST placement, which states where a reference is drawn and
  not where the variable lives, so those claims are held back until every real
  placement in the view has been settled and are then dropped -- quietly, since
  nothing was contradicted -- for a variable that already has a group. Without
  that, first-claim-wins let a drawing outvote a definition purely on document
  order; in `land_model.stmx` EVERY `<item>` resolves to a ghost (the real
  `<stock>`/`<aux>` elements carry no `uid`), which is where the two spurious
  "already belongs to group" advisories came from. Known gap behind that: Stella
  also states membership as
  `<variables><group name="..."><entity name="..."/></group>`, which
  `ProcessModel`'s quiet unknown-element branch drops entirely -- so
  land_model's `gdp deflator` lands in the group its ghost sits in rather than
  the "View 1" its own `<entity>` list names. Reading `<entity>` is the real fix
  there; the ghost rule only stops a drawing from deciding a membership.
  Regression tests: `test/xmile/GroupRoundTripTest.cpp`.
- **A group with no variables is not emitted, so it cannot own anything, and
  the writer re-points past it.** Scoped to the module emission, which only
  Vensim/Dynamo input reaches now (Key Decisions); the sector path emits a
  `<group>` for an empty group like any other, so an owner claim there resolves
  without re-pointing. `generateModelAsGroups` writes a `<model>` per
  group that holds variables and hangs each child group's `<module>` off its
  owner's `<variables>`; an owner with no variables has no `<variables>` list to
  hang it from, so its children used to end up with a NULL `pModule` -- which
  the cross-level `<connect>` loop dereferenced. That is ordinary input, not a
  malformed document: a Vensim group banner with no equations under it is an
  empty group and `VensimParse` chains the NEXT banner's group to it
  (`test-models/tests/conditional_subscripts` has one), and an XMILE `<group>`
  can be emptied by losing its last member to a competing claim. Each owner link
  is therefore resolved to the nearest ancestor that really is emitted, in a
  prepass over `Model::Groups()`. Re-pointing only ever SHORTENS a link, so no
  `<connect>` is dropped and no expressible nesting is lost -- what an empty
  level of nesting expressed is not expressible in this scheme anyway. The walk
  is bounded by the group count rather than written as a plain `while`: nothing
  re-validates a Model on the way into a writer, and a `pOwner` cycle (which
  `ResolveGroupOwners` refuses to build and `VensimParse` cannot) would
  otherwise hang instead of diagnosing. An empty group gets no `<module>` at any
  level either, for the same reason it gets no `<model>`: a `<module>` naming a
  model that was never written is a reference to nothing.
- **The writer names every group it emits, and never asserts about it.** A
  `<group name="   ">` normalizes to the empty string, which the reader accepts
  and `Model::AdjustGroupNames` (which only uniquifies) leaves alone. The writer
  used to `assert` -- an abort in a debug build over data a reader accepted, and
  with `NDEBUG` a silent `name=""` that no cross level can resolve. `Print` now
  substitutes a placeholder through `SanitizeGroupNames` before either emission
  path runs, dodging both the variable name space and the other group names the
  way `AdjustGroupNames` does. Substituting rather than skipping the group is
  the point: `generateModelAsGroups` emits equations only through the groups, so
  a skipped group is a set of equations missing from the output. The sibling
  `assert` on a view title is gone for the same reason -- `MakeViewNamesUnique`,
  which `Print` calls two lines earlier, already establishes it.
- A round trip can flip a variable's display-name form (`foo bar` <->
  `foo_bar`) when a model forward-references an aux before declaring it.
  `XmileReader::EnsureCanonicalName` mutates `Symbol::sName` at the
  declaration site to keep the namespace hash (invariant under `_` <-> ` `
  swap) consistent; the reliability corpus exercises this path.
- Empty or whitespace-only `<eqn>` bodies are treated as empty (route to
  the standalone graphical-function path when a `<gf>` sibling exists).
- **A `<unit>`'s `<eqn>` is a FORMULA, not another name, and the three parts stay
  apart in memory.** `<eqn>kg*m/s^2</eqn>` on a Newton, or `<eqn>1</eqn>` on a
  dimensionless quantity, is a different claim from `<alias>`. The declaration
  used to be stored as one comma-joined string, with the writer guessing which
  field had been the equation by a rule that recognized only a literal `$`:
  every other equation came back out as a fabricated `<alias>`, and a Stella
  `<unit name="$"><eqn/>` lost its name to the same rule (the first alias was
  promoted, the `$` demoted to an equation). `ProcessModelUnits` and
  `generateModelUnits` now read and write `UnitEquiv` (`src/Model.h`) directly,
  so there is nothing to guess; only the `.mdl` writer flattens, and it owns
  both halves of that mapping (`src/Mdl/CLAUDE.md`). A present-but-empty
  `<eqn/>` -- which real Stella exports carry -- states no formula and stays
  empty, so it produces neither an `<eqn>` on the way out nor a blank `22:`
  field. Regression tests: `test/xmile/ModelUnitsRoundTripTest.cpp`.
- `Model::ParseXMILE` does NOT run the post-parse pipeline; callers do.
  The four extern-C drivers in `src/XMUtil.cpp` and the test helpers in
  `test/xmile/RoundTrip.cpp` all call `Model::RunPostParsePipeline`. What it
  DOES do besides driving the reader is set `bFromXmile` (Key Decisions) --
  which is why the fix for the multi-`<model>` emission lives there and not in
  `convert_xmile_to_xmile`: the round-trip test helpers go through
  `ParseXMILE` + `PrintXMILE` and never touch the C entries, so a fix in the
  entry point would have left the whole corpus uncovered.
- **Equation-reference subscripts are flat, not nested.** The equation grammar's
  `subs` production (`xpyy_sub_init`/`xpyy_sub_append` in
  `XmileParseFunctions.cpp`) folds each `sub_term` into one flat SymbolList --
  one `EntryType_SYMBOL`/`EntryType_BANG_SYMBOL` entry per subscript position,
  matching `VensimParse::SymList` and what both writers'
  SymbolList consumers expect (`MDLGenerator::RenderSubscripts`,
  `SymbolList::OutputComputable`). The older list-of-lists shape rendered as
  empty `[]` in XMILE output and crashed the MDL writer on the null-symbol `*`
  bang. The LHS subscript builders (`BuildAppliesToAllSubs`, `ParseSubscriptList`)
  were already flat, which is why apply-to-all arrays round-tripped before wave 2
  while equation references did not.
- **Bare `*` wildcards are bound post-parse, not at parse time.** `[*]` becomes a
  BANG_SYMBOL with a NULL symbol (the referenced variable's dimensions are not
  known during equation parsing -- it may be declared later). Once the whole
  model is parsed and `MarkVariableTypes` has set element/family ownership,
  `Model::ResolveWildcardSubscripts` binds each null bang to the referenced
  variable's dimension family at that position, so `SUM(a[*])` emits Vensim
  `SUM(a[DimA!])` and XMILE `SUM(a[*])`. `*:Sub` already carries the concrete
  dimension and is left alone. The subrange bang emits as `*:Sub` (the inverse of
  the `'*' ':' symbol` grammar rule); the older `Sub.*` spelling did not re-parse.
- **The module decomposition is unreachable from XMILE input, and the
  multi-VIEW branch of it is doubly so.** `Model::PrintXMILE` sends every
  XMILE-sourced model down the sector path (Key Decisions), so neither
  `generateModelAsModules`' per-view `<model>` nor `generateModelAsGroups`'
  per-group `<model>` can be produced from an XMILE document -- which is what
  makes XMILE -> XMILE re-importable. Independently, `XmileReader::ProcessViews`
  keeps only the FIRST `<view>` carrying sketch geometry and warns about the
  rest, so an XMILE-sourced Model never holds the two-or-more views
  `generateModelAsModules` tests for in the first place. Both facts are asserted
  (`Normalization_multi_view_model_emits_one_readable_model`); a change to
  either one alone leaves the other holding the property, which is why the
  reader-side behavior is pinned rather than assumed. What a multi-view document
  DOES lose is everything outside its first view -- geometry and the group
  membership stated there -- and that is the `<view>` limitation, not a writer
  asymmetry.
- **Unbound `*` wildcards are a hard conversion error, not a silent marker.**
  If `Model::ResolveWildcardSubscripts` cannot bind a bare `*` (the target is
  not arrayed at that position), the model is recorded in
  `Model::UnresolvedWildcards` and both `PrintMDL`/`PrintXMILE` fail via `errs`
  rather than emitting a literal `*` (invalid MDL, un-reparseable XMILE).
- **The XMILE equation lexer admits UTF-8 identifier bytes.** `XmileEqLex`
  treats any byte `>= 0x80` as an identifier constituent (both start and
  continuation), matching the Vensim lexer's `c > 127` rule, so accented-Latin /
  non-ASCII variable names lex and round-trip.
- **The save interval has two spellings, and no value is silently repaired.**
  XMILE 1.0 defines no save-interval property on `<sim_specs>` (export cadence
  lives in the `<data>` section), so SAVEPER travels either as the isee vendor
  attribute `isee:save_interval` -- what Stella, simlin, and `XMILEGenerator`
  all emit -- or as a `<save_step>` child element. `ProcessSimSpecs` accepts
  both, the element winning when a document carries both, and falls back to dt
  for a value that is absent, unparseable, non-finite, or non-positive, saying
  which. `isfinite` comes BEFORE the sign test: tinyxml2 converts through
  `sscanf("%lf")`, which accepts `"nan"`, and every comparison against a NaN is
  false, so a sign test alone waved it through. An empty `<save_step/>`
  expressed no interval rather than a malformed one and is reported as such. A
  finite positive interval is the modeler's and is never clamped, however small.
  `<dt>` gets the same finite-and-positive check because SAVEPER defaults to it,
  but is only reported, never replaced -- substituting the writer's default
  would ship a different model than the document describes. `<start>`/`<stop>`
  get no check: every finite value is legal there and neither feeds another
  field's default. The writer emits the attribute whenever SAVEPER is finite and
  differs from dt in EITHER direction (the old `> dt` test silently dropped a
  sub-dt SAVEPER; the finiteness test is what keeps a NaN from being written out
  as `isee:save_interval="nan"`, which this reader would then reject) and
  formats it with `ShortestDouble`, since `std::to_string`'s six decimals
  truncate a small interval to `0.000000`. `generateSimSpecs` guards its own
  `isee:sim_duration` quotient rather than trusting SAVEPER to be safe, because
  a zero or non-finite dt reaches it through that same default.
- **`time_units` is the control variables' DEFAULT, applied after the whole
  document is walked.** A control variable that spells out its own `<units>` is
  making the more specific claim and keeps it; `time_units` fills in only the
  controls that stayed silent. `ProcessSimSpecs` therefore merely records the
  attribute and `ApplyDeferredTimeUnits` (end of `ProcessFile`) attaches it: the
  envelope loop dispatches `<sim_specs>` BEFORE `<model>`, so an "only if
  absent" test inside `ProcessSimSpecs` is vacuous in the order every writer
  emits, and deciding there would make the answer depend on document order (and
  on whether a `<views><group><var>` happened to materialize the control first).
  `AttachVariableUnits` enforces first-source-wins for BOTH halves of a
  Variable's units -- `Variable::AddUnits` keeps the first `UnitExpression` and
  offers no replace, so a helper that wrote the raw string unconditionally left
  the two halves naming different sources, and since both writers read the
  parsed half first (`MDLGenerator::UnitsCommentTrailer`,
  `XMILEGenerator::generateSimSpecs`) the raw text was the half discarded.
  Attaching through `AttachVariableUnits` rather than a bare `SetUnitsString`
  matters for the reverse reason: `generateSimSpecs` reads the unit back through
  `Model::GetUnits`, which returns the parsed expression, so a raw-only store
  made every non-default time unit reappear as the hardcoded `"Months"`. One
  visible side effect, shared with how the Vensim units namespace already
  behaves: the first spelling of a unit to be registered wins for every later
  case-variant, and per-variable `<units>` is now registered first -- a document
  with `<units>deg/time</units>` and `time_units="Time"` renders both as
  `deg/time` / `time`. Regression tests:
  `test/xmile/SimSpecsRoundTripTest.cpp`.
- **A control variable carries EXACTLY ONE equation, and `<sim_specs>` states its
  value.** XMILE does not reserve `INITIAL TIME` / `FINAL TIME` / `TIME STEP` /
  `SAVEPER`; Vensim `.mdl` does, so a `<variables>` declaration of one of those
  names and the `<sim_specs>` child stating the same quantity collapse onto one
  symbol on the way out (the namespace folds `_` and space, so `TIME_STEP` and
  `TIME STEP` are one Variable). Two equations on that Variable is not cosmetic:
  `MDLGenerator::GenerateVariableEntry` emits one `.Control` entry per stored
  equation, so it ships a `.mdl` with a duplicate definition Vensim rejects --
  which six vendored fixtures (`test-models/tests/lookups/*`,
  `logicals/test_logicals_caseinsensitive`,
  `special_characters_xmile/test_special_variable_names`) actually did.
  `<sim_specs>` wins because it is where every XMILE tool reads the run's start,
  stop and dt -- so it is what the source document really simulates with -- and
  because its values are the ones already in the engine's own
  `initial_time`/`final_time`/`dt` fields, which `Model::GetConstanValue` falls
  back to: letting the declaration overwrite the equation would leave the two
  halves of one answer disagreeing. `ProcessControlDeclaration` therefore drops
  the declaration's equation (`<units>`/`<doc>` still attach, so a control's own
  units keep beating `time_units`), and `ProcessDimensions` refuses a `<dim>` on
  a control name for the same reason. A control's value must be a scalar
  constant, so a `<stock>`, a `<gf>`, a subscripted `<aux>`, or an `<eqn>`
  holding an expression can never supply one: `SAVEPER = TIME STEP` resolves to
  the constant, and a synthesized `TIME STEP = INTEG(...)` in `.Control` is
  prevented outright. The ONE thing a declaration may still do is fill a gap --
  a value `<sim_specs>` never spelled out is a reader-invented default (1.0 for
  a missing `<dt>`), and an invented default must not beat a number the modeler
  wrote. So `SetControlVariable` attaches only the STATED values, and
  `ApplyControlValues` (end of `ProcessFile`, before `ApplyDeferredTimeUnits`)
  settles the rest -- SAVEPER last, since its default is whatever dt settled at
  -- and mirrors each resulting constant back into the Model field.
  Order-independence comes from `ProcessFile`'s pre-pass hoisting every
  `<sim_specs>` ahead of `<model>`: XMILE fixes no order among the envelope's
  children, and without the hoist the winner would be whichever element came
  first. Name matching goes through `SymbolNameSpace::ToLowerSpace`
  (`XmileReader::ControlIndexOf`), the same fold `MDLGenerator::IsControlVar`
  uses to decide what belongs in `.Control`; the two must agree, or a name one
  catches and the other misses is filtered out of the main equation section and
  then emitted from an equation list nothing deduplicated. A control that is
  still waiting on `ApplyControlValues` cannot take `<units>` yet -- a Variable
  allocates the content holding its parsed `UnitExpression` only with its first
  equation (`Variable::AddEq`) -- so the declaration element is held in
  `ControlVar::pendingUnitsAndDoc` and re-visited from `SettleControl`; the
  tinyxml2 document outlives the epilogue, so the pointer stays valid. Stella's
  `DT` / `STARTTIME` / `STOPTIME` spellings do NOT fold to the Vensim names and
  are deliberately not aliased here (that is the `sumif.xmile` deferral, a
  separate feature). Regression tests:
  `test/xmile/SimSpecsRoundTripTest.cpp`.
- Known gap, deliberately not closed with the rule above: a control declared
  with a non-constant equation (`SAVEPER = TIME STEP`, the Vensim default) is
  resolved to a constant rather than kept symbolic. Keeping it symbolic needs
  `Model::_dt` to survive a Vensim parse, and `VensimParse` never calls
  `set_dt` -- so `ModelComparator`'s `GetConstanValue(name, m->dt())` reads the
  Model default on the reparsed side and the real dt on the XMILE side, and the
  round trip reports a sim-spec diff that is an artifact of the missing setter,
  not of the model.
