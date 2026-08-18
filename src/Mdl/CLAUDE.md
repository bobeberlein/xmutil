# Mdl writer

Last verified: 2026-08-17

## Purpose
Serializes an in-memory `Model` back to Vensim `.mdl` text, so a model parsed
by xmutil (from `.mdl` or `.dyn`) can be re-emitted as native Vensim. This is
the inverse of `src/Vensim/` reading; it is a sibling to the XMILE writer in
`src/Xmile/`.

## Contracts
- **Exposes**:
  - `MDLGenerator(Model*)` and `std::string Print()` -> full `.mdl` text
    (CRLF line endings). `Print` has no failure path of its own; the error
    checks (unresolved wildcards) live in `Model::PrintMDL(errs)`
    (`src/Model.cpp`), which fails via `errs` before constructing the
    generator. Reached from there and the `convert_to_mdl(...)` `extern "C"`
    entry in `src/XMUtil.{h,cpp}`.
  - `convert_to_mdl` returns a caller-owned heap string, or NULL on parse
    failure (same ownership contract as `convert_mdl_to_xmile`).
  - `MDLGenerator::IsControlVar(name)` and `RenderExpression(Expression*)` are
    public only so the round-trip test comparator can reuse them.
  - `mdl::` free functions in `MDLFormat.h`: identifier quoting/escaping,
    number formatting, equation tokenizing/wrapping, lookup-body rendering.
- **Guarantees**: The emitted `.mdl` re-parses into a structurally equivalent
  Model (verified by the round-trip tests). Control vars (INITIAL TIME, FINAL
  TIME, TIME STEP, SAVEPER) are emitted only inside the `.Control` group, never
  in the main equation section.
- **Expects**: The caller has already run the post-parse pipeline
  (`MarkVariableTypes` for the main model and every macro, `AdjustGroupNames`,
  `CheckGhostOwners`, `ResolveWildcardSubscripts`) before calling `PrintMDL`.
  The writer renders a bang subscript via the bound symbol. A wildcard that
  `ResolveWildcardSubscripts` could not bind (the target is not arrayed at that
  position) is recorded in `Model::UnresolvedWildcards`, and `PrintMDL` then
  fails via `errs` rather than emitting an invalid literal `*` -- so skipping the
  resolution step, or feeding a genuinely malformed wildcard, surfaces as a
  conversion error, not broken output. The writer emits
  post-`AdjustGroupNames` group names, so skipping that step yields `.mdl` that
  does not round-trip.

## Dependencies
- **Uses**: `Model`, `Expression` tree node types and `EXPTYPE` tags
  (`src/Symbol/`), `VensimView`/element geometry (`src/Vensim/VensimView.h`),
  and the Vensim grammar token constants (`src/Vensim/VYacc.tab.hpp`).
- **Used by**: `Model::PrintMDL`, `convert_to_mdl`, and the `test/mdl/` suite.
  `MDLFormat.h` reaches further than the rest of this subsystem: `src/Model.h`
  includes it so `UnitEquiv::MdlPayload` can sanitize its own fields, which puts
  the one definition of the `22:` mapping next to the data it maps rather than
  duplicating the rule in the writer and the reader (see Key Decisions).
- **Boundary**: Do not pull simlin code in. This subsystem was ported from the
  simlin writer but must stand alone against xmutil's own model and grammar.

## Key Decisions
- **Parenthesization is derived from xmutil's OWN grammar precedence**
  (`src/Vensim/VYacc.y`, low to high: `+ -` < `:OR:` < comparisons < `:AND:` <
  `* /` < `^`), NOT from simlin's `writer.rs` precedence lattice. Output must
  re-parse under the parser that will read it; simlin's AST came from a
  different parser. See `MdlPrecedence`/`MdlUnaryPrecedence` in
  `MDLGenerator.cpp`. Parser-supplied `ExpressionParen` nodes are dropped and
  parens re-introduced only where precedence requires.
- **Lookup range box is recomputed** from the min/max of the stored x/y points,
  because `ExpressionTable`'s parsed range is unreliable (`WriteLookupBody`).
- **The `22:` unit-equivalence line is built by flattening a structured
  declaration, and the flattening is an exact INVERSE of the `22:` parse.**
  `Model::UnitEquivs()` holds `UnitEquiv` records (`src/Model.h`): canonical
  name, optional derived-unit equation, aliases. Vensim's `22:` line has no
  equation concept -- it is a flat comma-separated list of interchangeable names
  whose first field is canonical -- so `UnitEquiv::MdlPayload` /
  `UnitEquiv::ParseMdlPayload` own the mapping, and `GenerateSettings` /
  `VensimParse`'s `22:` branch each go through them rather than restating it.
  The pair is an exact inverse apart from the sanitizing below, so a `22:` line
  read from a `.mdl` re-emits byte for byte unless it carries a character
  `SanitizeFreeText` neutralizes
  (`ModelUnits_vensim_settings_lines_round_trip_byte_identically`)
  -- which is what the leading-`$` rule is for. `$,Dollar,Dollars,$s` is Vensim's
  own spelling of the currency unit XMILE writes as
  `<unit name="Dollar"><eqn>$</eqn>`, so a LEADING bare `$` parses as the
  equation and re-emits back in front. Recognizing it only in the leading
  position (upstream recognized it anywhere) is what keeps the inverse exact: a
  `$` anywhere else is just another spelling in Vensim's flat list, and hoisting
  it would rewrite the canonical name. A derived-unit formula has no Vensim
  representation and flattens into the name list rather than being dropped --
  lossy, but stable across a re-import, and never a silent loss of the text.
- **Field sanitizing for `22:` happens when the line is BUILT, not when the
  declaration is read.** `,` is the field separator and a `|` or line break would
  end the line early (#849), but all three are legal inside an XMILE `<unit>`;
  neutralizing them at read time corrupted the XMILE -> XMILE path for a hazard
  only the `.mdl` path has. `UnitEquiv::MdlPayload` runs each field through
  `mdl::SanitizeFreeText(..., SingleLine, ",")`.
- Sketch is re-serialized from `VensimView` geometry; an element's array index
  is its on-wire UID (connectors reference those indices). A model with no view
  still emits one empty frame so the terminator and trailing `:L` settings
  section re-parse. The frame header is POSITIONAL (opener / version / `*Title` /
  font line / records), so the modeler-authored view title goes through
  `mdl::SanitizeFreeText` in `SingleLine` mode -- a title carrying a line break
  would otherwise push the font line into a record slot and desynchronize the
  whole sketch.
- **The emitted sketch may only name variables the emitted equation section
  names**, and the filter for that is RECORDED, not predicted. A sketch record
  carries nothing about its subject but a name, and re-reading resolves that name
  through `VensimParse::FindVariable`, which sees only what the equation text
  interned -- so a record naming anything else produces a `.mdl` inconsistent
  with itself. `MDLGenerator` therefore collects `_namedInEquations` (every
  `Variable` whose name it actually wrote, as a definition LHS, an expression
  reference, a subscript, or a dimension element) while rendering, and
  `SuppressedSketchSlots` filters against that -- which is why `Print` emits the
  sketch after all three equation passes. Recording rather than re-deriving is
  the point: the equation section withholds a variable for several unrelated
  reasons (`Unwanted`, an untyped variable with no equation, a net-flow carrier
  inlined into its stock's `INTEG`, an array element emitted only through its
  dimension), and a predicate that restated them would drift. Note the test is
  "named", not "defined": `keeper = phantom` mentions `phantom` without defining
  it, and `phantom`'s record re-reads perfectly well, so it stays.
- **Suppressing a sketch record pulls two other things with it, and the survivors
  keep their UIDs.** The valve at `uid - 1` goes too when it is attached (Vensim
  encodes a flow as an attached valve immediately followed by the flow's variable
  record, and consumers resolve the pair by adjacency alone -- see
  `XMILEGenerator::generateView`), and so does every connector with an endpoint on
  a removed slot. The removed slots are left EMPTY rather than renumbered:
  connector endpoints are those UIDs, so renumbering would mean rewriting every
  one of them and would still have to special-case the valve pairing (a flow
  removed from under its valve otherwise lets whatever slid into `valve_uid + 1`
  be adopted as the flow -- wrong rather than merely absent). A gap costs nothing,
  since a sparse view is already ordinary: `VensimView::ReadView` leaves a NULL in
  every slot no record claims. The three resulting invariants -- no record names
  an undefined variable, every connector endpoint has a record, every attached
  valve is followed by a variable record -- are asserted in
  `test/mdl/SketchRoundTripTest.cpp`.

## Key Files
- `MDLGenerator.{h,cpp}` - the `Model` -> `.mdl` walker (equations, macros,
  `.Control`, sketch, settings).
- `MDLFormat.{h,cpp}` - identifier/number formatting, line wrapping, lookup body.

## Gotchas
- `Model::PrintMDL` returns "" on error (does not throw); always check `errs`.
- **A group banner's NAME is where Vensim states nesting, so the banner has to
  carry the whole path.** A nested group is written `<owner path>.<leaf>`, with a
  leading `.` on the outermost level -- `.Physics`, `.Physics.Forcing`,
  `.Physics.Forcing.Aerosols+CO+BC` are all real `C-LEARN v77` banners.
  `ModelGroup::sName` is that path with the separators folded to `-`, because the
  name also becomes an XMILE `<module name=...>`, which cannot carry a `.`.
  `EmitGroupBanner` used to write `sName` alone, discarding every `pOwner` link,
  and `VensimParse` used to answer an unresolvable banner with "the previous
  banner owns me" -- so the forest was not merely lost on the way out, a
  DIFFERENT one was invented on the way back in: a flat model came back as one
  linear chain, with `.Control` (written last) adopted by whatever user group
  came before it. `GroupBannerPath` now walks the owner chain and joins the leaf
  names with `.`; the reader resolves the parent path back to a group and leaves
  the group at the root when no such group exists (C-LEARN declares
  `.Input.Policy1` without ever declaring `.Input`, and synthesizing the missing
  level would only produce a group with no variables, which neither writer
  emits). Two consequences worth knowing: a `.` inside a group's own name is
  folded to `-` before emission, since it would otherwise read back as another
  level of nesting; and a model whose nesting did NOT come from a `.mdl` (an
  XMILE `<group name="Inner" owner="Outer">`) is re-read under its path,
  `Outer-Inner`. That rename is the format's rather than a choice -- a Vensim
  group has no identity apart from where it sits. Regression tests:
  `test/mdl/GroupNestingTest.cpp`.
- Stocks: when `Variable::MarkStockFlows` synthesized a net-flow carrier
  variable, its expression is inlined back into `INTEG(...)` so output matches
  user input and the carrier is not duplicated (`EmitStockEntry`). The carrier is
  identified by the in-memory `Variable::SynthesizedNetFlow` provenance flag set
  where it is created, NEVER by its "<stock> net flow" name -- a modeler may
  legally use that name, and matching on it deleted the modeler's flow together
  with its units, documentation, group and sketch element. The flag is
  deliberately not serialized: re-reading an emitted `.mdl` re-synthesizes and
  re-marks, which is what makes the inlining round-trip. The XMILE module path's
  cross-view flow proxies (`Variable::SynthesizedFlowProxy`, see
  `src/Xmile/CLAUDE.md`) are likewise left out of the equation section: the
  stock's `INTEG` still names the modeler's flow, so nothing needs inlining, and
  emitting a proxy would add a variable the source never had.
- A sketch record whose name does not resolve to a `Variable` leaves
  `VensimVariableElement::_variable` NULL and keeps no copy of the name, so there
  is nothing to re-emit. That is one input to `SuppressedSketchSlots` (Key
  Decisions) rather than a special case: dropping the record ALONE -- which is
  what an earlier NULL guard inside `EmitVariableRecord` did -- left the hole its
  attached valve still pointed into and the connectors that still named it, so it
  closed one segfault by relocating it into `XMILEGenerator::generateView` and
  left a dangling connector that survived every subsequent conversion. Emitting
  the whole cluster consistently is what makes such a model a byte fixpoint
  again.
- **`mdl::SanitizeFreeText` runs its section-terminator scan LAST, and to a
  fixpoint.** The per-char pass rewrites `|` as `/`, so it is itself a source of
  terminator runs: a field spelled with pipes carries no run on the way in and a
  live one on the way out, and the next conversion's scan -- now looking at
  slashes -- ate it along with the surrounding text. Scanning after the per-char
  pass means the scan sees the bytes the reader will; looping until the string
  stops changing is needed because splicing a run out joins the text on either
  side of it, which can spell a shorter run the left-to-right scan already walked
  past. The loop terminates because every replacement swaps eight or nine
  characters for one. Idempotence is a property of the whole function, asserted
  over a table of interacting inputs in `test/mdl/MDLFormatTest.cpp`.
- **An over-long emitted line is warned about, never truncated.**
  `VensimLex::ReadLine` pulls a line into a fixed `BUFLEN` (4096) buffer and hands
  the remainder back as the NEXT line, which shifts every following line by one
  slot -- an over-long `*Title` costs the re-read the entire sketch AND the
  trailing `:L` settings block at exit status zero. The safe length is 4094, not
  4095, because `ReadLine` tests its buffer bound before it tests for the line
  terminator. `WarnIfLineTooLong` reports the hazard for the frame title, each
  variable record, and each `22:` unit-equivalence line; truncating instead would
  violate `SanitizeFreeText`'s contract of a documented substitution and never a
  silent drop.
- Two shared-code fixes underpin round-trip fidelity and also affect the XMILE
  path (both verified to keep XMILE output byte-identical): `VensimParse`
  `mInMacro` is now initialized (uninitialized garbage was dropping group
  membership for all non-macro models), and `VensimView` element geometry
  (`_x/_y/_width/_height`) is now zero-initialized (connector records leave
  width/height unread -> was UB / non-deterministic round trips).
