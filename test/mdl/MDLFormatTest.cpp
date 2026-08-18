#include <cstdlib>
#include <string>
#include <vector>

#include "../../src/Mdl/MDLFormat.h"
#include "../../src/Model.h"
#include "../../src/Symbol/Variable.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

// Strip every byte that is part of a Vensim line continuation (the backslash,
// the embedded newline, and the indent tabs) plus ordinary spaces. What remains
// is the non-whitespace content, which a correct WrapEquation must preserve
// exactly: wrapping only inserts continuations between tokens (and trims a
// trailing space before the break), so the visible glyphs are unchanged.
std::string StripContinuationsAndSpaces(const std::string &s) {
  std::string out;
  for (char c : s) {
    if (c == '\\' || c == '\n' || c == '\t' || c == ' ')
      continue;
    out += c;
  }
  return out;
}

}  // namespace

// AC2.5: identifier quoting. Spaces are allowed unquoted in Vensim names, but
// an empty name, leading/trailing space, a leading non-identifier character, or
// any interior character outside [A-Za-z0-9_ ] forces quoting.
TEST(MDLFormat_ident_no_quoting_when_simple) {
  CHECK_EQ_STR(mdl::FormatMDLIdent("Susceptible Population"), "Susceptible Population");
  CHECK_EQ_STR(mdl::FormatMDLIdent("Flow"), "Flow");
  CHECK_EQ_STR(mdl::FormatMDLIdent("_underscore_start"), "_underscore_start");
  CHECK_EQ_STR(mdl::FormatMDLIdent("a1 b2 c3"), "a1 b2 c3");
}

TEST(MDLFormat_ident_quotes_when_required) {
  // Interior special character.
  CHECK_EQ_STR(mdl::FormatMDLIdent("Lag #"), "\"Lag #\"");
  CHECK_EQ_STR(mdl::FormatMDLIdent("M&Ms"), "\"M&Ms\"");
  // Leading digit is not a valid identifier start.
  CHECK_EQ_STR(mdl::FormatMDLIdent("100% true"), "\"100% true\"");
  // Leading/trailing space.
  CHECK_EQ_STR(mdl::FormatMDLIdent(" leading"), "\" leading\"");
  CHECK_EQ_STR(mdl::FormatMDLIdent("trailing "), "\"trailing \"");
  // Empty.
  CHECK_EQ_STR(mdl::FormatMDLIdent(""), "\"\"");
}

// An already-quoted input must not be double-quoted: the parser may store the
// surrounding quotes in the symbol name, so FormatMDLIdent strips one pair
// before deciding. The result must be byte-stable across that round trip.
TEST(MDLFormat_ident_does_not_double_quote) {
  CHECK_EQ_STR(mdl::FormatMDLIdent("\"M&Ms\""), "\"M&Ms\"");
  CHECK_EQ_STR(mdl::FormatMDLIdent("\"100% true\""), "\"100% true\"");
  // A name that does not actually need quoting, but arrives quoted, comes back
  // bare (strip-then-requote is idempotent on the unquoted core).
  CHECK_EQ_STR(mdl::FormatMDLIdent("\"Flow\""), "Flow");
}

// Escaping inside a quoted identifier: interior quotes and backslashes are
// escaped, real newlines become the two-character \n escape, and an existing
// \n escape is preserved rather than double-escaped.
TEST(MDLFormat_escapes_interior_characters) {
  // An interior double quote forces quoting and is escaped.
  CHECK_EQ_STR(mdl::FormatMDLIdent("say \"hi\""), "\"say \\\"hi\\\"\"");
  CHECK_EQ_STR(mdl::EscapeMDLQuotedIdent("a\"b"), "a\\\"b");
  CHECK_EQ_STR(mdl::EscapeMDLQuotedIdent("a\\b"), "a\\\\b");
  CHECK_EQ_STR(mdl::EscapeMDLQuotedIdent("a\nb"), "a\\nb");
  // A literal backslash followed by 'n' is a display newline; keep it as \n.
  CHECK_EQ_STR(mdl::EscapeMDLQuotedIdent("a\\nb"), "a\\nb");
}

TEST(MDLFormat_needs_quoting_predicate) {
  CHECK(!mdl::NeedsMDLQuoting("Flow"));
  CHECK(!mdl::NeedsMDLQuoting("Susceptible Population"));
  CHECK(!mdl::NeedsMDLQuoting("_x9"));
  CHECK(mdl::NeedsMDLQuoting(""));
  CHECK(mdl::NeedsMDLQuoting(" x"));
  CHECK(mdl::NeedsMDLQuoting("x "));
  CHECK(mdl::NeedsMDLQuoting("9x"));
  CHECK(mdl::NeedsMDLQuoting("a&b"));
}

// A name with multibyte UTF-8 characters is a legal bare Vensim identifier (the
// lexer accepts bytes >= 0x80 as identifier characters), so it must NOT be
// quoted -- a corpus model with names like "variable é" was being needlessly
// quoted, breaking the round trip (bare in source, quoted on re-emit).
TEST(MDLFormat_unicode_name_not_quoted) {
  CHECK(!mdl::NeedsMDLQuoting("this is a french variable with \xC3\xA9 \xC3\xA0 \xC3\xA8"));
  CHECK_EQ_STR(mdl::FormatMDLIdent("variable \xC3\xA9"), "variable \xC3\xA9");
  // A multibyte leading character is also a valid identifier start. The escape
  // is split from the following ASCII letter so the compiler does not fold the
  // letter into a greedy \x hex escape.
  CHECK(
      !mdl::NeedsMDLQuoting("\xC3\xB1"
                            "ame"));
}

// The parser stores quoted identifiers in their .mdl source form -- already
// escaped (a literal interior quote as \", a literal backslash kept as-is). When
// FormatMDLIdent receives such an already-quoted name it must emit the interior
// verbatim, NOT re-run the escaper, or the name double-escapes (\" -> \\\", a
// lone \ -> \\) and corrupts on every round trip. A bare (logical) name is still
// escaped as before.
TEST(MDLFormat_quoted_name_not_double_escaped) {
  // Already-quoted, already-escaped: comes back byte-identical.
  CHECK_EQ_STR(mdl::FormatMDLIdent("\"var with \\\"quotes\\\" and \\ slash\""),
               "\"var with \\\"quotes\\\" and \\ slash\"");
  // A bare logical name with a raw interior quote is still escaped + quoted (the
  // bare path is unchanged; this case does not arise from the parser but the
  // function must still behave for synthesized names).
  CHECK_EQ_STR(mdl::FormatMDLIdent("say \"hi\""), "\"say \\\"hi\\\"\"");
}

// AC2.5: number formatting. Whole values lose the trailing decimal, the :NA:
// sentinel (-1e38) is recognized, and a fractional value round-trips through
// strtod back to the same double.
TEST(MDLFormat_number_na_sentinel) {
  CHECK_EQ_STR(mdl::FormatMDLNumber(-1e38), ":NA:");
}

TEST(MDLFormat_number_whole_values) {
  CHECK_EQ_STR(mdl::FormatMDLNumber(3.0), "3");
  CHECK_EQ_STR(mdl::FormatMDLNumber(0.0), "0");
  CHECK_EQ_STR(mdl::FormatMDLNumber(-5.0), "-5");
  CHECK_EQ_STR(mdl::FormatMDLNumber(100.0), "100");
}

TEST(MDLFormat_number_fractional_values) {
  CHECK_EQ_STR(mdl::FormatMDLNumber(0.5), "0.5");
}

// AC2.5 byte-stability against the real parser: a quoted variable name that
// requires quoting (interior '&', leading digit) must come back out of
// FormatMDLIdent identical to its source quoted spelling, whether or not the
// parser stored the surrounding quotes in Symbol::GetName().
TEST(MDLFormat_quoted_name_round_trips_through_parser) {
  const std::string text =
      "{UTF-8}\r\n"
      "\"M&Ms\" = 3\r\n"
      "\t~\t\r\n"
      "\t~\t\r\n"
      "\t|\r\n"
      "\"100% true\" = 4\r\n"
      "\t~\t\r\n"
      "\t~\t\r\n"
      "\t|\r\n";
  Model *m = roundtrip::ParseVensim(text);
  CHECK(m != nullptr);
  if (!m)
    return;
  bool saw_mms = false;
  bool saw_pct = false;
  for (Variable *v : m->GetVariables(nullptr)) {
    std::string formatted = mdl::FormatMDLIdent(v->GetName());
    if (formatted == "\"M&Ms\"")
      saw_mms = true;
    if (formatted == "\"100% true\"")
      saw_pct = true;
  }
  CHECK(saw_mms);
  CHECK(saw_pct);
  delete m;
}

TEST(MDLFormat_number_round_trips) {
  // A value with no exact short decimal representation must still parse back to
  // the identical double after formatting (shortest round-trippable output).
  const double inputs[] = {0.1, 0.2, 1.0 / 3.0, 3.14159265358979, 1234.5678, -2.5e-7};
  for (double v : inputs) {
    std::string s = mdl::FormatMDLNumber(v);
    double parsed = strtod(s.c_str(), nullptr);
    CHECK(parsed == v);
  }
}

// AC2.6 (wrapping): a tokenizer that splits an equation at natural boundaries
// must preserve the text exactly -- concatenating the tokens reproduces the
// input. This is the invariant WrapEquation relies on to avoid corrupting an
// equation when it inserts continuations.
TEST(MDLFormat_tokenize_for_wrapping_round_trips) {
  const char *inputs[] = {
      "a + b * c",
      "IF THEN ELSE(x > 0, PULSE(1, 2), 0)",
      "first variable - second variable + third variable",
      "-a * (b - c) / d ^ e",
      "f('a quoted literal', \"Quoted Ident\", x)",
  };
  for (const char *in : inputs) {
    std::vector<std::string> toks = mdl::TokenizeForWrapping(in);
    std::string joined;
    for (const std::string &t : toks)
      joined += t;
    CHECK_EQ_STR(joined, in);
  }
}

// AC2.6: an equation within the line budget passes through verbatim.
TEST(MDLFormat_wrap_equation_short_passthrough) {
  CHECK_EQ_STR(mdl::WrapEquation("a = b + c", 80), "a = b + c");
  // Exactly at the limit is still unchanged (<= maxLineLen).
  std::string at_limit(80, 'x');
  CHECK_EQ_STR(mdl::WrapEquation(at_limit, 80), at_limit);
}

// AC2.6: an equation over the line budget wraps with the Vensim "\<newline><two
// tabs>" continuation, and removing the continuations preserves every visible
// glyph (no token dropped, duplicated, or reordered).
TEST(MDLFormat_wrap_equation_inserts_continuations) {
  // A long sum of identifiers, comfortably over 80 characters.
  const std::string eqn =
      "total = alpha plus + beta plus + gamma plus + delta plus + epsilon plus + zeta plus + eta plus";
  CHECK(eqn.size() > 80);
  std::string wrapped = mdl::WrapEquation(eqn, 80);
  // The continuation sequence is present.
  CHECK(wrapped.find("\\\n\t\t") != std::string::npos);
  // The wrapped form is genuinely different from the input.
  CHECK(wrapped != eqn);
  // Stripping continuations and spaces reconstructs the same non-whitespace
  // content as the original: wrapping never alters the token stream itself.
  CHECK_EQ_STR(StripContinuationsAndSpaces(wrapped), StripContinuationsAndSpaces(eqn));
  // No physical line exceeds the budget (continuations actually bound width).
  size_t lineStart = 0;
  while (lineStart < wrapped.size()) {
    size_t nl = wrapped.find('\n', lineStart);
    std::string line =
        (nl == std::string::npos) ? wrapped.substr(lineStart) : wrapped.substr(lineStart, nl - lineStart);
    CHECK(line.size() <= 80);
    if (nl == std::string::npos)
      break;
    lineStart = nl + 1;
  }
}

// AC2.6: a quoted literal or identifier containing a delimiter character (a
// comma or operator) must not be split -- the whole quoted run stays inside a
// single token, so a continuation can never land in the middle of it.
TEST(MDLFormat_wrap_equation_quotes_are_atomic) {
  std::vector<std::string> toks = mdl::TokenizeForWrapping("x = 'a, b + c' + \"d - e\"");
  // Each quoted run appears intact within exactly one token (the tokenizer
  // appends the quote to the running token rather than emitting it standalone,
  // matching the simlin writer; the point is the interior comma/operators do
  // not create a split).
  bool literalIntact = false;
  bool identIntact = false;
  for (const std::string &t : toks) {
    if (t.find("'a, b + c'") != std::string::npos)
      literalIntact = true;
    if (t.find("\"d - e\"") != std::string::npos)
      identIntact = true;
  }
  CHECK(literalIntact);
  CHECK(identIntact);
}

// mdl::SanitizeFreeText must be IDEMPOTENT: its output is what a re-read hands
// back to the next write, so any input for which sanitize(sanitize(x)) differs
// from sanitize(x) makes the writer's output drift -- and, since every
// substitution here removes or replaces, drift means text loss on the second
// write rather than merely a different spelling.
//
// The interesting inputs are the ones where two substitutions INTERACT. The
// per-char pass rewrites '|' as '/', so a run of pipes against a "---" can
// SPELL a section terminator that was not in the source; running the terminator
// scan before that pass left the run in the output and let the next conversion
// eat it. Cases below cover that shape from both directions, plus the mixed
// forms (canonical and short runs, adjacent runs, terminators split by a pipe).
TEST(MDLFormat_sanitize_free_text_is_idempotent) {
  // Each C++ literal doubles every backslash; the comments give the byte form.
  const char *const inputs[] = {
      "",
      "plain text",
      "has | pipe",
      "\\\\\\---||||",               // three backslashes, ---, four pipes
      "||||---///",                  // four pipes, ---, three slashes
      "\\\\---||||",                 // the two-backslash (short) open run, pipe-spelled
      "||||---\\\\",                 // the short close run, pipe-spelled
      "\\\\\\---///",                // a literal canonical open run
      "///---\\\\\\",                // a literal canonical close run
      "\\\\\\---//|",                // one pipe completes the canonical open run
      "a\\\\\\---||||b",             // a run with text on both sides
      "\\\\\\---||||\\\\\\---||||",  // two runs back to back
      "\\\\\\\\\\\\---//////",       // six backslashes then six slashes
      "x\ny|z",
      "x\r\ny",
      "trailing break\n",
      "~ tilde ~",
  };
  const mdl::FreeTextLineMode modes[] = {mdl::FreeTextLineMode::SingleLine, mdl::FreeTextLineMode::Multiline};
  const char *const forbidden[] = {"", "~", ","};

  bool sawAChange = false;
  for (const char *raw : inputs) {
    for (mdl::FreeTextLineMode mode : modes) {
      for (const char *extra : forbidden) {
        const std::string once = mdl::SanitizeFreeText(raw, mode, extra);
        const std::string twice = mdl::SanitizeFreeText(once, mode, extra);
        CHECK_EQ_STR(once, twice);
        if (once != raw)
          sawAChange = true;
        // The result must also be free of the runs it exists to remove, on the
        // FIRST pass -- "idempotent" alone would be satisfied by a function
        // that never touched anything. Only the four runs that carry
        // backslashes are terminators; a bare "---///" with no backslash is
        // ordinary text the reader never mistakes for a boundary.
        const char *const runs[] = {"\\\\\\---///", "///---\\\\\\", "\\\\---///", "///---\\\\"};
        for (const char *run : runs)
          CHECK(once.find(run) == std::string::npos);
        CHECK(once.find('|') == std::string::npos);
      }
    }
  }
  CHECK(sawAChange);  // non-vacuity: the table really does exercise substitutions
}
