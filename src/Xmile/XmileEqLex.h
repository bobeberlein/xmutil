#ifndef _XMUTIL_XMILE_XMILEEQLEX_H
#define _XMUTIL_XMILE_XMILEEQLEX_H

#include <cstddef>
#include <deque>
#include <string>

// Hand-written tokenizer for the text inside a single XMILE <eqn> element.
// Structure mirrors src/Vensim/VensimLex (also hand-written despite the
// "flex" wording in the design doc -- the codebase has no flex dependency).
// XMILE equations are much simpler than full Vensim MDL: no inline comments,
// no :KEYWORD: syntax, no tabbed-array literal, no equation-end marker -- the
// input buffer IS one equation, and yylex() returns 0 at end-of-buffer for
// bison's accept signal.
//
// The lexer is exercised by the XmileEqYacc grammar. Token codes
// (XPTT_*) come from XmileEqYacc.tab.hpp; the lexer #includes that header.
class XmileEqLex {
public:
  XmileEqLex();

  // Bind the lexer to a UTF-8 equation buffer. The buffer must outlive the
  // lexer; we keep a borrowed pointer. len is the byte length; the buffer
  // need not be NUL-terminated.
  void Initialize(const char *text, std::size_t len);

  // Return the next XPTT_* token, populating xpyylval as a side effect.
  // Returns 0 at end of buffer (the bison accept signal).
  int yylex();

  // Short snippet around the current position, for use in error messages.
  std::string Snippet() const;

private:
  // GetNextChar returns the next raw byte (0 at end-of-buffer).
  unsigned char GetNextChar();
  // PushBack restores a previously-returned byte to the input (LIFO: the most
  // recently pushed byte is returned first). Most lookahead needs one byte
  // (`<=` vs `<`, `//` vs `/`), but MaybeColonNa restores up to three on a
  // ':' that turns out not to start an `:na:` literal.
  void PushBack(unsigned char c);

  // Token scanners. Each leaves the consumed text in sToken and populates
  // xpyylval as appropriate before returning the token code.
  int ScanNumber(unsigned char first);
  int ScanIdentOrKeyword(unsigned char first);
  int ScanQuotedSymbol();
  int MaybeColonNa();

  // Intern the current sToken so xpyylval.lit can point at a stable C
  // string for the duration of the parse. Bison reads xpyylval at reduction
  // time, which can be one or more tokens after we set it, so we cannot use
  // sToken's storage directly (the next NextToken would overwrite it).
  const char *InternToken();

  const char *pText;
  std::size_t iCurPos;
  std::size_t iLength;

  std::string sToken;
  // Interned token strings live here for the duration of the lexer. A
  // std::vector<std::string> would invalidate prior c_str() pointers on
  // reallocation; std::deque never invalidates references to existing
  // elements, so back().c_str() stays stable across later interns.
  std::deque<std::string> mInterns;

  // LIFO pushback stack; sized for the deepest lookahead in the lexer
  // (MaybeColonNa's three bytes).
  unsigned char aPushBack[4];
  int iPushBackCount;
};

#endif
