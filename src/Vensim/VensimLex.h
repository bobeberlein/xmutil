#ifndef _XMUTIL_VENSIM_VENSIMLEX_H
#define _XMUTIL_VENSIM_VENSIMLEX_H

/* a tokenizer for Vensim files - it is called by VensimPare both
  indirectly throught the Bison generated parser and directly for 
  comments and group defs */
#include <string>
#include "../Symbol/Parse.h"

class VensimLex
{
public:
   VensimLex(void) ;
   ~VensimLex(void);
   void Initialize(const char *content,off_t length);
   std::string *CurToken(void) ;
   void GetReady(void) ;
   int yylex(void) ;
   int GetEndToken(void) ;
   int LineNumber(void) { return iLineNumber ; }
   int Position(void) { return iCurPos - iLineStart ; }
   std::string GetComment(const char *tok);
   bool FindToken(const char *tok) ;
   bool ReadLine(char *buf, size_t buflen); // read a line if enough room otherwise part of it
private :
   char GetNextChar(bool store) ;
   void PushBack(char c,bool store) ;
   void SyncBuffers(void) ;
   bool TestTokenMatch(const char *tok,bool update) ;
   std::string sToken ;
   std::string sBuffer ;
   const char *ucContent ;
   off_t iCurPos,iHoldPos ;
   off_t iLineStart,iHoldStart ;
   void MarkPosition(void) {iHoldPos=iCurPos;iHoldStart=iLineStart;}
   void ReturnToMark(void) {iCurPos=iHoldPos;iLineStart=iHoldStart;sBuffer.empty();}
   int iLineNumber ;
   off_t iFileLength ;
   int NextToken(void) ;
   bool KeywordMatch(const char *target) ;
   void GetDigits(void) ;
   int iInUnitsComment ; // 0 no, 1 units, 2 comment
   bool bInGroup ;
   int TestColonKeyword(void) ;
   int ReadTabbedArray(void) ;
   bool bInUnits;


};

#endif
