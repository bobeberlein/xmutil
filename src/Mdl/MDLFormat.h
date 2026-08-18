#pragma once
#ifndef __MDLFORMAT_H
#define __MDLFORMAT_H

#include <string>
#include <vector>

class ExpressionTable;

// Free functions for formatting identifiers and numbers as Vensim .mdl text.
// Ported from the simlin MDL writer (src/simlin-engine/src/mdl/writer.rs).
namespace mdl {

// Whether a free-text field spans multiple lines or must collapse to one.
// Multiline preserves internal breaks as LF; SingleLine collapses each run of
// line breaks to a single space.
enum class FreeTextLineMode { Multiline, SingleLine };

// Make modeler-authored free text (variable units/documentation, group names,
// unit-equivalence tokens) structurally safe to embed in a .mdl entry, so a
// raw structural character cannot terminate the construct early and re-parse the
// remainder as phantom variables (#849). The substitutions, all documented and
// never a silent drop: `|` -> `/` (a NON-whitespace substitute, so a field-final
// `|` cannot become a trailing space the reader trims -- which would break
// comment-field idempotence); the four sketch section-terminator runs -> a
// space; each char in `extraForbidden` (a `~` in a units field, a `,` in a `22:`
// token) -> a space. Line endings normalize LOSSLESSLY to LF (`\r\n` and lone
// `\r` -> `\n`) so a field carrying a `\r` from a CRLF source is a fixpoint
// rather than accreting a carriage return each write; Print's single final
// LF->CRLF pass restores CRLF. Ported from simlin's sanitize_free_text
// (src/simlin-engine/src/mdl/writer.rs).
std::string SanitizeFreeText(const std::string &raw, FreeTextLineMode mode, const std::string &extraForbidden);

// True if the (already-dequoted) name must be wrapped in double quotes for
// Vensim. Interior spaces are allowed unquoted; quoting is required for an empty
// name, a leading or trailing space, a leading character outside [A-Za-z_], or
// any interior character outside [A-Za-z0-9_ ].
bool NeedsMDLQuoting(const std::string &name);

// Escape interior quotes, backslashes, and newlines for a quoted Vensim
// identifier. A real newline becomes the two-character escape \n; an existing
// \n escape (backslash followed by 'n') is preserved rather than double-escaped.
std::string EscapeMDLQuotedIdent(const std::string &name);

// Strip an optional single pair of surrounding quotes, then quote and escape iff
// needed. The single entry point the walker uses for every emitted identifier;
// idempotent on already-quoted input.
std::string FormatMDLIdent(const std::string &rawName);

// Vensim number text: ":NA:" for the -1e38 sentinel, an integer form for whole
// values, otherwise the shortest decimal that round-trips back to the same
// double.
std::string FormatMDLNumber(double v);

// Split an .mdl equation string into tokens at natural line-break boundaries:
// after a comma (the trailing space stays with the comma token), around each
// bracket/paren, and before a binary +-*/^ operator. A leading or post-operator
// +/- is treated as a unary sign and kept attached. Quoted '...' literals and
// "..." identifiers are atomic. Concatenating the returned tokens reproduces the
// input exactly.
std::vector<std::string> TokenizeForWrapping(const std::string &eqn);

// Wrap a long equation with Vensim "\<newline><two tabs>" continuations so no
// physical line exceeds maxLineLen (call with 80). An equation already within
// the budget is returned unchanged. Breaks happen between tokens, trimming a
// trailing space before the break.
std::string WrapEquation(const std::string &eqn, size_t maxLineLen);

// Render a lookup/graphical-function table as the Vensim body
// "[(xmin,ymin)-(xmax,ymax)],(x0,y0),(x1,y1),...". The range box is computed
// from the min/max of the stored x/y vectors because xmutil's parsed range
// (ExpressionTable::AddRange) is unreliable. With no points the box collapses to
// "[(0,0)-(0,0)]". The body carries no extrapolation flag: MDL has none at the
// definition level. The extrapolate kind is preserved instead by the WRITER, as
// a TABXL(table, x) call site at each reference (see MDLGenerator's
// _extrapolateLookups); a standalone extrapolating table with no call site
// cannot be marked and the writer warns that it is emitted clamped to continuous.
std::string WriteLookupBody(ExpressionTable *table);

}  // namespace mdl

#endif  // __MDLFORMAT_H
