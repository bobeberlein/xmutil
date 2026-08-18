#include "XmileEqLex.h"

#include <cassert>
#include <cctype>
#include <cstdlib>

#include "../XMUtil.h"

// The bison-generated header declares xpyylval as `extern YYSTYPE xpyylval`
// and forward-declares the XPTT_* token codes. We must define YYSTYPE before
// including the header; Parse.h supplies the ParseUnion typedef the macro
// expands to. This matches the pattern used in src/Vensim/VensimLex.cpp.
#include "../Symbol/Parse.h"
#define YYSTYPE ParseUnion
#include "XmileEqYacc.tab.hpp"

namespace {

// Any byte >= 0x80 is a UTF-8 lead or continuation byte; treating the whole
// high range as identifier-constituent accepts non-ASCII names (accented Latin,
// CJK, ...) without decoding codepoints. This mirrors the Vensim lexer, which
// admits identifier bytes with `c > 127` (VensimLex.cpp), so a name that the
// Vensim reader accepts on the XMILE->MDL round trip also lexes here.
bool IsIdentStart(unsigned char c) {
  return std::isalpha(c) || c == '_' || c >= 0x80;
}

bool IsIdentCont(unsigned char c) {
  return std::isalnum(c) || c == '_' || c >= 0x80;
}

}  // namespace

XmileEqLex::XmileEqLex() : pText(nullptr), iCurPos(0), iLength(0), iPushBackCount(0) {
}

void XmileEqLex::Initialize(const char *text, std::size_t len) {
  pText = text;
  iCurPos = 0;
  iLength = len;
  iPushBackCount = 0;
  sToken.clear();
}

unsigned char XmileEqLex::GetNextChar() {
  if (iPushBackCount > 0)
    return aPushBack[--iPushBackCount];
  if (!pText || iCurPos >= iLength)
    return 0;
  return static_cast<unsigned char>(pText[iCurPos++]);
}

void XmileEqLex::PushBack(unsigned char c) {
  // LIFO: GetNextChar pops the most recently pushed byte first, so callers
  // restoring multi-byte lookahead (MaybeColonNa) push in reverse order.
  assert(iPushBackCount < static_cast<int>(sizeof(aPushBack)));
  aPushBack[iPushBackCount++] = c;
}

const char *XmileEqLex::InternToken() {
  mInterns.emplace_back(sToken);
  return mInterns.back().c_str();
}

int XmileEqLex::ScanNumber(unsigned char first) {
  // Accepts `5`, `5.`, `.5`, `5.25`, and any of those followed by `e`/`E` with
  // an optional sign -- so each stage below reads one byte past what it wants
  // and pushes it back when it is not part of the number. That lookahead is
  // why the exponent test cannot simply peek: `5e` with no digits after it is
  // an identifier boundary, not a malformed number.
  //
  // first is either a digit or '.'; in the '.' case the caller has already
  // verified the next byte is a digit so we are committed to producing a
  // number token.
  sToken.clear();
  sToken.push_back(static_cast<char>(first));
  bool sawDot = (first == '.');

  // Consume a run of decimal digits into sToken, pushing back the first
  // non-digit byte.
  auto scanDigits = [this]() {
    unsigned char c;
    while ((c = GetNextChar())) {
      if (c >= '0' && c <= '9') {
        sToken.push_back(static_cast<char>(c));
      } else {
        PushBack(c);
        break;
      }
    }
  };

  if (!sawDot) {
    scanDigits();
    unsigned char nx = GetNextChar();
    if (nx == '.') {
      sToken.push_back('.');
      sawDot = true;
    } else if (nx) {
      PushBack(nx);
    }
  }

  if (sawDot)
    scanDigits();

  unsigned char e = GetNextChar();
  if (e == 'e' || e == 'E') {
    sToken.push_back(static_cast<char>(e));
    unsigned char sign = GetNextChar();
    if (sign == '+' || sign == '-') {
      sToken.push_back(static_cast<char>(sign));
    } else if (sign) {
      PushBack(sign);
    }
    scanDigits();
  } else if (e) {
    PushBack(e);
  }

  xpyylval.num = std::atof(sToken.c_str());
  return XPTT_number;
}

int XmileEqLex::ScanIdentOrKeyword(unsigned char first) {
  sToken.clear();
  sToken.push_back(static_cast<char>(first));
  unsigned char c;
  while ((c = GetNextChar())) {
    if (IsIdentCont(c)) {
      sToken.push_back(static_cast<char>(c));
    } else {
      PushBack(c);
      break;
    }
  }

  // Keyword spellings of the boolean operators and the if/then/else triple
  // are matched case-insensitively here. The grammar accepts both these
  // tokens and their C-style symbolic forms (&&, ||, !) under the same
  // XPTT_* code.
  struct Keyword {
    const char *text;
    int token;
  };
  static constexpr Keyword kKeywords[] = {
      {"if", XPTT_if}, {"then", XPTT_then}, {"else", XPTT_else}, {"and", XPTT_and},
      {"or", XPTT_or}, {"not", XPTT_not},   {"mod", XPTT_mod},
  };
  for (const Keyword &kw : kKeywords) {
    if (StringMatch(sToken, kw.text))
      return kw.token;
  }
  // Bare NaN literal lowers to the shared Vensim missing-data sentinel so the
  // value is indistinguishable from a parsed-from-MDL :NA: downstream. The
  // :NA: form is handled separately by MaybeColonNa.
  if (StringMatch(sToken, "nan")) {
    xpyylval.num = kVensimNaSentinel;
    return XPTT_number;
  }

  xpyylval.lit = InternToken();
  return XPTT_symbol;
}

int XmileEqLex::ScanQuotedSymbol() {
  // The opening '"' has already been consumed. We accumulate the interior
  // verbatim (no escape handling for v1) and consume the closing quote.
  sToken.clear();
  unsigned char c;
  while ((c = GetNextChar())) {
    if (c == '"')
      break;
    sToken.push_back(static_cast<char>(c));
  }
  xpyylval.lit = InternToken();
  return XPTT_symbol;
}

int XmileEqLex::MaybeColonNa() {
  // The leading ':' has already been consumed. Peek for "na:" / "NA:"; on
  // miss restore the input as if we never looked, so the caller returns the
  // bare ':' token (used in subscript syntax like `*:Dim`).
  unsigned char a = GetNextChar();
  unsigned char b = GetNextChar();
  unsigned char colon = GetNextChar();
  if ((a == 'n' || a == 'N') && (b == 'a' || b == 'A') && colon == ':') {
    xpyylval.num = kVensimNaSentinel;
    return XPTT_number;
  }
  if (colon)
    PushBack(colon);
  if (b)
    PushBack(b);
  if (a)
    PushBack(a);
  return ':';
}

std::string XmileEqLex::Snippet() const {
  if (!pText)
    return std::string();
  std::size_t start = (iCurPos > 8) ? iCurPos - 8 : 0;
  std::size_t end = (iCurPos + 8 < iLength) ? iCurPos + 8 : iLength;
  return std::string(pText + start, end - start);
}

int XmileEqLex::yylex() {
  unsigned char c;
  // Skip whitespace including CR/LF/TAB so multi-line XMILE equations are OK.
  do {
    c = GetNextChar();
  } while (c == ' ' || c == '\t' || c == '\r' || c == '\n');
  if (!c)
    return 0;  // bison accept signal

  if (IsIdentStart(c))
    return ScanIdentOrKeyword(c);

  if (c >= '0' && c <= '9')
    return ScanNumber(c);

  if (c == '.') {
    unsigned char nx = GetNextChar();
    if (nx >= '0' && nx <= '9') {
      PushBack(nx);
      return ScanNumber('.');
    }
    // A bare '.' is not part of XMILE equation syntax; pass it through so
    // the grammar surfaces an error pointed at the offending character.
    if (nx)
      PushBack(nx);
    return '.';
  }

  if (c == '"')
    return ScanQuotedSymbol();

  if (c == ':')
    return MaybeColonNa();

  if (c == '=') {
    unsigned char nx = GetNextChar();
    if (nx == '=') {
      // == is the C-style equality form; the grammar accepts both = and ==
      // under XPTT_eq.
      return XPTT_eq;
    }
    if (nx)
      PushBack(nx);
    return XPTT_eq;
  }

  if (c == '<') {
    unsigned char nx = GetNextChar();
    if (nx == '=')
      return XPTT_lte;
    if (nx == '>')
      return XPTT_neq;
    if (nx)
      PushBack(nx);
    return XPTT_lt;
  }

  if (c == '>') {
    unsigned char nx = GetNextChar();
    if (nx == '=')
      return XPTT_gte;
    if (nx)
      PushBack(nx);
    return XPTT_gt;
  }

  if (c == '!') {
    unsigned char nx = GetNextChar();
    if (nx == '=')
      return XPTT_neq;
    if (nx)
      PushBack(nx);
    return XPTT_not;
  }

  if (c == '&') {
    unsigned char nx = GetNextChar();
    if (nx == '&')
      return XPTT_and;
    // A single '&' isn't valid XMILE; pass the bare character through so the
    // grammar can produce a positioned error.
    if (nx)
      PushBack(nx);
    return '&';
  }

  if (c == '|') {
    unsigned char nx = GetNextChar();
    if (nx == '|')
      return XPTT_or;
    if (nx)
      PushBack(nx);
    return '|';
  }

  if (c == '%')
    return XPTT_mod;

  if (c == '/') {
    unsigned char nx = GetNextChar();
    if (nx == '/')
      return XPTT_safediv;
    if (nx)
      PushBack(nx);
    return '/';
  }

  if (c == '\'')
    return XPTT_apostrophe;

  // Single-char tokens that map straight to their ASCII code:
  // + - * ^ ( ) [ ] , @ -- the grammar references them directly.
  return c;
}
