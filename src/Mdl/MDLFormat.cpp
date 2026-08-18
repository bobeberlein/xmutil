#include "MDLFormat.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "../Symbol/Expression.h"
#include "../Symbol/Parse.h"
#include "../XMUtil.h"

namespace mdl {

namespace {

bool IsIdentStart(char c) {
  // A byte >= 0x80 is part of a UTF-8 multibyte character. The Vensim lexer
  // (VensimLex::GetToken) accepts such bytes as bare identifier characters
  // (`c > 127`), so a name like "variable é" needs no quoting; treating high
  // bytes as non-identifier here would needlessly quote every unicode name and
  // break the round trip (bare in source, quoted on re-emit).
  if (static_cast<unsigned char>(c) >= 0x80)
    return true;
  return c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool IsIdentContinue(char c) {
  return IsIdentStart(c) || (c >= '0' && c <= '9');
}

// Trim trailing spaces, then report whether the trimmed token marks a position
// where a following +/- is unary rather than binary: an empty token, or one
// ending in '(' or ',', or one that is itself an operator. Mirrors the
// `trimmed.ends_with(...)` / equality checks in writer.rs:1145-1153.
bool TokenAllowsUnarySign(const std::string &tok) {
  size_t end = tok.size();
  while (end > 0 && tok[end - 1] == ' ')
    end--;
  size_t begin = 0;
  while (begin < end && tok[begin] == ' ')
    begin++;
  std::string trimmed = tok.substr(begin, end - begin);
  if (trimmed.empty())
    return true;
  char last = trimmed.back();
  if (last == '(' || last == ',')
    return true;
  return trimmed == "+" || trimmed == "-" || trimmed == "*" || trimmed == "/" || trimmed == "^";
}

// The four sketch section-terminator runs GenerateSketch emits (see
// MDLGenerator.cpp): the canonical open/close carry three backslashes, the short
// variants two. A run appearing verbatim in free text would let the settings
// reader mistake it for a real section boundary, so SanitizeFreeText neutralizes
// them to a space. Each C++ literal below doubles every backslash.
// (byte contents in the same order shown in GenerateSketch; no trailing
// backslash in these comments, which would splice the next line)
const char *kSectionTerminatorOpen = "\\\\\\---///";
const char *kSectionTerminatorClose = "///---\\\\\\";
const char *kSectionTerminatorOpenShort = "\\\\---///";
const char *kSectionTerminatorCloseShort = "///---\\\\";

void ReplaceAll(std::string &s, const std::string &from, const std::string &to) {
  if (from.empty())
    return;
  size_t pos = 0;
  while ((pos = s.find(from, pos)) != std::string::npos) {
    s.replace(pos, from.size(), to);
    pos += to.size();
  }
}

// Replace every sketch section-terminator run in `s` with a space, repeating
// until the string stops changing.
//
// Repeating is what makes the result terminator-free rather than merely
// terminator-fewer: `find` walks left to right, and splicing out a run joins the
// text on either side of it, so the join can spell a shorter run that the scan
// has already walked past. Each replacement swaps eight or nine characters for
// one, so the string strictly shrinks whenever anything changes and the loop
// terminates. The replacement itself cannot re-form a run, since neither run
// contains a space.
void NeutralizeSectionTerminators(std::string &s) {
  for (;;) {
    // Only pay the four scans when a backslash and a "---///"/"///---" fragment
    // are both present (the guard simlin uses).
    if (s.find('\\') == std::string::npos ||
        (s.find("---///") == std::string::npos && s.find("///---") == std::string::npos))
      return;
    const size_t before = s.size();
    // Replace the three-backslash (canonical) runs BEFORE the two-backslash
    // variants: a canonical run contains the short run as a substring, so the
    // reverse order would leave a stray backslash behind.
    ReplaceAll(s, kSectionTerminatorOpen, " ");
    ReplaceAll(s, kSectionTerminatorClose, " ");
    ReplaceAll(s, kSectionTerminatorOpenShort, " ");
    ReplaceAll(s, kSectionTerminatorCloseShort, " ");
    if (s.size() == before)
      return;
  }
}

// Emit a normalized line break for SanitizeFreeText: a canonical LF for a
// multi-line field, or a single collapsed space for a single-line field.
void PushLineBreak(std::string &out, mdl::FreeTextLineMode mode, bool &prevWasBreak) {
  if (mode == mdl::FreeTextLineMode::Multiline) {
    // Preserve internal breaks exactly (do not collapse blank lines).
    out += '\n';
  } else if (!prevWasBreak) {
    out += ' ';
    prevWasBreak = true;
  }
}

}  // namespace

std::string SanitizeFreeText(const std::string &raw, FreeTextLineMode mode, const std::string &extraForbidden) {
  std::string out;
  out.reserve(raw.size());
  bool prevWasBreak = false;
  for (size_t i = 0; i < raw.size(); i++) {
    char c = raw[i];
    if (c == '\r') {
      // Normalize CRLF and lone CR to the same break handling as LF; a following
      // LF is consumed so CRLF collapses to a single break.
      if (i + 1 < raw.size() && raw[i + 1] == '\n')
        i++;
      PushLineBreak(out, mode, prevWasBreak);
    } else if (c == '\n') {
      PushLineBreak(out, mode, prevWasBreak);
    } else if (c == '|') {
      out += '/';
      prevWasBreak = false;
    } else if (extraForbidden.find(c) != std::string::npos) {
      out += ' ';
      prevWasBreak = false;
    } else {
      out += c;
      prevWasBreak = false;
    }
  }

  // The terminator runs are neutralized AFTER the per-char pass, not before it,
  // because the per-char pass is itself a source of them: it rewrites '|' as
  // '/', so text the pre-pass saw as terminator-free ("\\\---||||") leaves that
  // pass spelling a live open-terminator run. Scanning first left the run in the
  // output, and the NEXT conversion's pre-pass -- now looking at slashes -- ate
  // it along with the rest of the field, so emit(1) and emit(2) disagreed and
  // text silently vanished on the second write. Running last means the scan sees
  // exactly the bytes the reader will.
  //
  // The per-char pass stays idempotent over this, so the order does not merely
  // move the problem: neutralizing only ever inserts a space, which that pass
  // passes through unchanged.
  NeutralizeSectionTerminators(out);
  return out;
}

bool NeedsMDLQuoting(const std::string &name) {
  if (name.empty())
    return true;
  // A leading or trailing space cannot survive a bare round trip.
  if (name.front() == ' ' || name.back() == ' ')
    return true;
  if (!IsIdentStart(name[0]))
    return true;
  for (char c : name) {
    // Interior spaces are legal in bare Vensim names.
    if (c == ' ')
      continue;
    if (!IsIdentContinue(c))
      return true;
  }
  return false;
}

std::string EscapeMDLQuotedIdent(const std::string &name) {
  std::string escaped;
  escaped.reserve(name.size());
  for (size_t i = 0; i < name.size(); i++) {
    char c = name[i];
    switch (c) {
    case '\n':
      // A real newline must become the two-character escape \n.
      escaped += "\\n";
      break;
    case '\\':
      if (i + 1 < name.size() && name[i + 1] == 'n') {
        // An existing \n escape (backslash + 'n', a display newline that XMILE
        // name attributes use) is preserved rather than double-escaped to \\n.
        escaped += "\\n";
        i++;
      } else {
        escaped += "\\\\";
      }
      break;
    case '"':
      escaped += "\\\"";
      break;
    default:
      escaped += c;
      break;
    }
  }
  return escaped;
}

std::string FormatMDLIdent(const std::string &rawName) {
  // A name that arrives already wrapped in quotes came from the parser, which
  // stores quoted identifiers in their .mdl source form -- the interior is
  // already escaped (a literal interior quote is stored as \", a literal
  // backslash as itself). Re-running EscapeMDLQuotedIdent over that would
  // double-escape it (\" -> \\\", \ -> \\), corrupting the name on a
  // parse -> emit -> re-parse round trip. So when the core was already quoted,
  // emit it verbatim inside quotes; only a bare (logical) name -- e.g. a
  // synthesized identifier -- is escaped.
  bool wasQuoted = rawName.size() >= 2 && rawName.front() == '"' && rawName.back() == '"';
  std::string core = wasQuoted ? rawName.substr(1, rawName.size() - 2) : rawName;
  if (!NeedsMDLQuoting(core))
    return core;
  if (wasQuoted)
    return "\"" + core + "\"";
  return "\"" + EscapeMDLQuotedIdent(core) + "\"";
}

std::string FormatMDLNumber(double v) {
  if (v == kVensimNaSentinel)
    return ":NA:";
  if (std::isfinite(v) && v == std::trunc(v) && std::fabs(v) < 1e15) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(v));
    return buf;
  }
  // Non-integer tail: the shared shortest-round-trip formatter, so .mdl
  // numbers match the XMILE writer's lookup samples byte for byte.
  return ShortestDouble(v);
}

std::vector<std::string> TokenizeForWrapping(const std::string &eqn) {
  std::vector<std::string> tokens;
  std::string current;
  size_t i = 0;
  size_t n = eqn.size();
  while (i < n) {
    char c = eqn[i];
    switch (c) {
    case ',':
      current += c;
      i++;
      // Absorb a single trailing space so it stays with the comma token.
      if (i < n && eqn[i] == ' ') {
        current += eqn[i];
        i++;
      }
      tokens.push_back(current);
      current.clear();
      break;
    case '(':
    case ')':
    case '[':
    case ']':
      if (!current.empty()) {
        tokens.push_back(current);
        current.clear();
      }
      current += c;
      i++;
      tokens.push_back(current);
      current.clear();
      break;
    case '+':
    case '-':
    case '*':
    case '/':
    case '^': {
      // A +/- is unary when nothing precedes it on the current line and the
      // previous token (if any) ends at a position where a sign starts a
      // subexpression. An absent previous token also counts as unary.
      bool isUnary = current.empty() && (tokens.empty() || TokenAllowsUnarySign(tokens.back()));
      if (isUnary) {
        current += c;
        i++;
      } else {
        if (!current.empty()) {
          tokens.push_back(current);
          current.clear();
        }
        // Emit the operator as its own token so a break can precede it.
        current += c;
        i++;
        tokens.push_back(current);
        current.clear();
      }
      break;
    }
    case '\'':
    case '"':
      // A quoted token (single-quoted literal or double-quoted identifier) is
      // consumed whole, including its closing quote.
      current += c;
      i++;
      while (i < n) {
        char ch = eqn[i];
        current += ch;
        i++;
        if (ch == c)
          break;
      }
      break;
    default:
      current += c;
      i++;
      break;
    }
  }
  if (!current.empty())
    tokens.push_back(current);
  return tokens;
}

std::string WrapEquation(const std::string &eqn, size_t maxLineLen) {
  if (eqn.size() <= maxLineLen)
    return eqn;

  std::vector<std::string> tokens = TokenizeForWrapping(eqn);
  std::string result;
  size_t currentLineLen = 0;
  for (const std::string &token : tokens) {
    // Break before a token that would overflow the line, but only if the line
    // already holds content (never break to an empty line).
    if (currentLineLen + token.size() > maxLineLen && currentLineLen > 0) {
      size_t trimmed = result.size();
      while (trimmed > 0 && result[trimmed - 1] == ' ')
        trimmed--;
      result.resize(trimmed);
      result += "\\\n\t\t";
      currentLineLen = 0;
    }
    result += token;
    currentLineLen += token.size();
  }
  return result;
}

std::string WriteLookupBody(ExpressionTable *table) {
  const std::vector<double> &xs = *table->GetXVals();
  const std::vector<double> &ys = *table->GetYVals();

  // Vensim's range box must enclose every point; xmutil's stored AddRange is
  // buggy (it never records xmin and writes dX2 twice), so derive the box from
  // the data the same way XMILEGenerator derives its y-scale. An empty table
  // yields a degenerate "[(0,0)-(0,0)]" box, which still re-parses.
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0;
  if (!xs.empty()) {
    auto [lo, hi] = std::minmax_element(xs.begin(), xs.end());
    xmin = *lo;
    xmax = *hi;
  }
  if (!ys.empty()) {
    auto [lo, hi] = std::minmax_element(ys.begin(), ys.end());
    ymin = *lo;
    ymax = *hi;
  }

  std::string s = "[(" + FormatMDLNumber(xmin) + "," + FormatMDLNumber(ymin) + ")-(" + FormatMDLNumber(xmax) + "," +
                  FormatMDLNumber(ymax) + ")]";
  // Pairs are emitted in stored order, which preserves the monotone-x ordering
  // Vensim requires. xs and ys are populated in lockstep (AddPair), so a length
  // mismatch is impossible in practice; the min guard is belt-and-suspenders.
  size_t npairs = std::min(xs.size(), ys.size());
  for (size_t i = 0; i < npairs; i++)
    s += ",(" + FormatMDLNumber(xs[i]) + "," + FormatMDLNumber(ys[i]) + ")";
  return s;
}

}  // namespace mdl
