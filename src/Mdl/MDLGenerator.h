#pragma once
#ifndef __MDLGENERATOR_H
#define __MDLGENERATOR_H

#include <string>
#include <unordered_set>
#include <vector>

class Model;
class ModelGroup;
class Expression;
class ExpressionFunction;
class ExpressionLogical;
class ExpressionVariable;
class Symbol;
class SymbolList;
class SymbolNameSpace;
class Variable;
class VensimViewElement;
class VensimVariableElement;
class VensimValveElement;
class VensimCommentElement;
class VensimConnectorElement;

class MDLGenerator {
public:
  explicit MDLGenerator(Model *model);

  // Returns the full .mdl text (CRLF line endings). Has no failure path of
  // its own: the error checks (e.g. unresolved wildcards) live in
  // Model::PrintMDL, which fails via its errs parameter before constructing
  // the generator.
  std::string Print();

  // True if `name` is one of the four Vensim sim-control variables
  // (INITIAL TIME, FINAL TIME, TIME STEP, SAVEPER), compared case-insensitively.
  // These are ordinary Variables in the namespace but are emitted only inside the
  // .Control group, never in the main equation section. Exposed so the test
  // comparator can apply the same classification.
  static bool IsControlVar(const std::string &name);

  // Render an Expression tree to native Vensim text. Functions emit as
  // GetName()(args) with no XMILE rewrites; operators emit infix with
  // precedence-derived parentheses; ExpressionParen nodes are unwrapped and the
  // parentheses re-derived purely from precedence (matching the simlin writer).
  std::string RenderExpression(Expression *e);

private:
  // Emit every Vensim macro as a ":MACRO: name(args) ... :END OF MACRO:" block,
  // before the main equation section (the parser defines a macro's name in the
  // main namespace as it reads the block, so callers in the main equations must
  // see it first). The body is the macro's local namespace, emitted via
  // GenerateVariableEntry -- with the reader-synthesized net-flow carriers of
  // that namespace suppressed, exactly as in the main equation loop.
  void GenerateMacros(std::string &out);
  // Emit the MAIN model's equation section group-by-group: for each ModelGroup
  // (skipping the .Control group, emitted by GenerateControl) emit its banner
  // then its non-control variables, then the ungrouped non-control variables.
  // Deliberately not parameterized by namespace: banners, group membership and
  // the .Control filter are all properties of the main model, so pointing this
  // at a macro's namespace would apply the main model's groups to it. A macro
  // body has none of those and is emitted by GenerateMacros instead.
  void GenerateEquations(std::string &out);
  // Emit the .Control group: its banner followed by INITIAL TIME, FINAL TIME,
  // TIME STEP, and SAVEPER. Each control var is emitted from its actual Variable
  // (so a SAVEPER = TIME STEP equation and the variables' real units/comments
  // round-trip); a control var absent from the namespace is synthesized as a
  // numeric entry from GetConstanValue.
  void GenerateControl(std::string &out);
  // Emit a group banner: a line of 56 '*', the name, 56 '*' + '~', the doc, and
  // the '|' terminator (port of simlin writer.rs:2934-2941).
  void EmitGroupBanner(std::string &out, const std::string &name, const std::string &doc);
  // The banner name that states both the group's identity and its nesting: the
  // owner-chain leaf names joined with '.', which is how Vensim encodes a
  // group's place in the forest and the only thing VensimParse can read the
  // nesting back out of. Emitting `sName` alone discarded every owner link.
  std::string GroupBannerPath(ModelGroup *group) const;
  // Re-serialize the model's VensimView geometry to Vensim sketch records. Each
  // VensimView emits the \\\---/// / V300 / *Title / $font framing then one
  // record per non-NULL, non-suppressed element (the element's array index is
  // the on-wire UID), with a single ///---\\\ terminator after all views. A
  // model with no VensimView emits one empty frame so the terminator -- and thus
  // the trailing settings section -- still re-parses; no geometry is fabricated.
  void GenerateSketch(std::string &out);
  // The UID slots of `elems` whose records GenerateSketch must leave out, so the
  // emitted sketch describes only what the emitted equation section defines. See
  // the definition for the three rules and why the surviving records keep their
  // UIDs instead of being renumbered.
  std::vector<bool> SuppressedSketchSlots(const std::vector<VensimViewElement *> &elems) const;
  // Per-element record emitters. Each appends one '\n'-terminated record line in
  // the exact field order the VensimView parser reads (VensimView.cpp). uid is
  // the element's array index in the view (VensimConnectorElement From()/To()
  // reference these indices, so they must be preserved). The only
  // parser-meaningful fields are attached (shape bit5), ghost (bits bit0,
  // inverted), polarity (ASCII-decimal 43/45/0), and from/to/coordinates; all
  // other fields are Vensim cosmetic defaults the parser discards.
  void EmitVariableRecord(std::string &out, int uid, VensimVariableElement *e);
  void EmitValveRecord(std::string &out, int uid, VensimValveElement *e);
  void EmitCommentRecord(std::string &out, int uid, VensimCommentElement *e);
  void EmitConnectorRecord(std::string &out, int uid, VensimConnectorElement *e);
  // Emit the trailing :L settings section: the integration method (15:), unit
  // equivalences (22:), and the Vensim default cosmetic lines. Must follow the
  // sketch terminator emitted by GenerateSketch (the parser only reads settings
  // after ///---\\\).
  void GenerateSettings(std::string &out);
  // Emit one variable's entry/entries (one per stored equation for arrayed
  // variables), or nothing for an unsupported/unwanted kind.
  void GenerateVariableEntry(std::string &out, Variable *v);
  // Emit a stock as INTEG(net_flow, init), one entry per stored equation so an
  // arrayed stock keeps each element's own net flow and initial value. The init
  // (arg 1) is rendered straight from the stored INTEG node. The net flow (arg 0)
  // is too when it is the user's own +/- of named flows; when MarkStockFlows
  // synthesized a carrier variable instead, its original expression is inlined
  // back in so the emitted .mdl matches the user's input and re-parses without
  // duplicating the carrier. The carrier is identified by the provenance flag
  // Variable::SynthesizedNetFlow, never by its name -- a modeler may legally name
  // a flow "<stock> net flow". See EmitStockEntry's body for the full rationale.
  void EmitStockEntry(std::string &out, Variable *v);
  // Emit a dimension/subrange definition "name: e1, e2, e3[ -> map] ~~|".
  void EmitDimensionEntry(std::string &out, Variable *v);
  // Render a flat comma-separated element list (no brackets) for a dimension
  // definition's left side or a subdimension map's elements.
  std::string RenderDimensionElements(SymbolList *sl);
  // Render a dimension definition's "->" mapping clause: a plain target
  // dimension or a "(Range: elements)" subdimension map.
  std::string RenderDimensionMap(SymbolList *map);

  std::string RenderOperator(Expression *e);
  std::string RenderLogical(ExpressionLogical *lg);
  std::string RenderFunction(ExpressionFunction *fn);
  std::string RenderVariableRef(ExpressionVariable *v);
  std::string RenderSubscripts(SymbolList *subs);
  // Render a table-like expression: a Lookup (embedded WITH LOOKUP or a named
  // lookup-variable call), a bare Table body, an arrayed NumberTable constant
  // list, or a bracketed Symlist element list.
  std::string RenderTableLike(Expression *e);

  // The "~\t<units>\n\t~\t<comment>\n\t|" trailer for a variable, with units
  // taken from the UnitExpression if present else the raw units string.
  std::string UnitsCommentTrailer(Variable *v);
  // Emit one already-composed entry ("<lhs> = <rhs>" for an equation,
  // "<lhs>(<body>)" for a standalone graphical function), wrapped at 80
  // columns, followed by the units/comment trailer and a trailing newline.
  void EmitWrappedEntry(std::string &out, const std::string &entry, Variable *v);

  // Wrap childStr in parentheses iff the child's precedence relative to its
  // parent requires it. parent and child may be paren-wrapped; the precedence
  // helpers unwrap internally.
  std::string ParenIfNecessary(Expression *parent, Expression *child, bool isRightChild, const std::string &childStr);

  // Record that the emitted equation text spells out this symbol's name, so a
  // sketch record may legally name it. A non-Variable symbol is ignored: only a
  // Variable is what VensimParse::FindVariable will hand a re-read sketch
  // record.
  void NoteVariableNamed(Symbol *s);
  // Warn (to the log/stderr channel, never the MDL text) when an emitted line is
  // longer than the Vensim sketch/settings reader's fixed line buffer, which
  // truncates rather than growing. `kind` names the line, `what` the element it
  // came from.
  void WarnIfLineTooLong(const std::string &line, const char *kind, const std::string &what);

  // Populate _extrapolateLookups with every standalone graphical-function
  // variable whose defining ExpressionTable is marked extrapolating. Called once
  // at the start of Print, before any equation is rendered.
  void BuildExtrapolateLookups();
  // After all equations are rendered, warn (to the log/stderr channel, never the
  // MDL text) for each extrapolating standalone lookup that had NO call site: MDL
  // has no definition-level extrapolate flag, so without a TABXL call site to
  // mark it the table is emitted clamped to continuous.
  void WarnUnreferencedExtrapolate();

  Model *_model;
  // Standalone extrapolating lookup variables. A LOOKUP(table, x) call to one is
  // emitted as TABXL(table, x) so re-import restores the extrapolate kind (a
  // plain table(x) re-imports as continuous). Populated by BuildExtrapolateLookups.
  std::unordered_set<Variable *> _extrapolateLookups;
  // The subset of _extrapolateLookups actually reached by a call site (and thus
  // emitted as TABXL). Filled during rendering; the complement is warned about.
  std::unordered_set<Variable *> _referencedExtrapolate;
  // Variables whose name the emitted equation text actually spells out -- as a
  // definition's left-hand side, an expression reference, a subscript, or a
  // dimension element. Re-reading the emitted .mdl interns exactly these names,
  // so this is the set a sketch record may legally name (GenerateSketch drops
  // the rest). It is RECORDED while the equations render rather than predicted
  // from a variable's type, so every reason the equation section has for
  // withholding a variable is honored here without being restated -- which is
  // why Print emits the sketch after all three equation passes.
  std::unordered_set<Variable *> _namedInEquations;
};

#endif  // __MDLGENERATOR_H
