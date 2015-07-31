/* VensimLex - 
*/
#include "VensimLex.h"
#include "VensimParse.h"
/* note this is the only file that includes VYacc.tab.h */
#define YYSTYPE  "C" ParseUnion
#include "VYacc.tab.h"
#include "../Symbol/Variable.h"
#include "../Symbol/Expression.h"
#include "../XMUtil.h"




VensimLex::VensimLex(void)
{
   ucContent = '\0' ;
   iCurPos = iFileLength = 0 ;
   GetReady() ;
}
VensimLex:: ~VensimLex()
{
}
void VensimLex::Initialize(const char *content,off_t length)
{
   ucContent = content ;
   iFileLength = length ;
   iLineStart = iCurPos = 0 ;
   iLineNumber = 1 ;
   GetReady() ;
}
int VensimLex::GetEndToken(void) 
{
   return VPTT_eqend ; 
}

void VensimLex::GetReady(void)
{
   bInGroup = false ;
   iInUnitsComment = 0 ;
}
std::string *VensimLex::CurToken()
{
   return &sToken ;
}

int VensimLex::yylex()
{
   int toktype = NextToken() ;
   switch(toktype) {
      case VPTT_number :
         vpyylval.num = atof(sToken.c_str()) ;
         break ;
      case VPTT_symbol :
         vpyylval.sym =   VPObject->InsertVariable(sToken) ;
         if(vpyylval.sym->isType() == Symtype_Function) {
            Function *f = static_cast<Function *>(static_cast<Symbol *>(vpyylval.sym)) ;
            if(f->AsKeyword()) {
               return ReadTabbedArray() ;// todo other keywords - this will return an ExpressionNumberTable
            }
            else
               toktype = VPTT_function ;
         }

         break ;
      case VPTT_units_symbol :
         vpyylval.uni =  VPObject->InsertUnitExpression(VPObject->InsertUnits(sToken)) ;
         break ;
      default :
         break ;
   }
   return toktype ;
   }

void VensimLex::GetDigits()
{
   char c ;
   while(1) {
      c = GetNextChar(true) ;
      if(c < '0' || c > '9') {
         PushBack(c,true) ;
         break ;
      }
   }
}

int VensimLex::ReadTabbedArray(void) 
{
   char c ;
   int row ;
   int toktype ;
   do { c = GetNextChar(false) ; }  while(c && c!= '(' && c != '~') ;
   if(c != '(') {
      this->PushBack(c,false) ;
      return c ;
   }
   // then just numbers - tab or space separated with new lines 
   ExpressionNumberTable *ent = new ExpressionNumberTable(VPObject->GetSymbolNameSpace()) ;
   row = 0 ;
   while(toktype = NextToken()) {
      if((toktype == '+' || toktype == '-')) {
         if(NextToken() == VPTT_number) {
            vpyylval.num = -vpyylval.num ;
            toktype = VPTT_number ;
         } else 
            throw "Bad numbers" ;
      }
      if(toktype == ')') { // finished 
         vpyylval.exn = ent ;
         return VPTT_tabbed_array ;
      }
      if(toktype != VPTT_number)
         throw "Bad numbers" ;
      ent->AddValue(row,vpyylval.num) ;
      // test for \n
      while(c = GetNextChar(false)) {
         if(c == '\n') {
            row++ ;
            break ;
         }
         if(c != '\t' && c != '\r' && c != ' ') {
            PushBack(c,false) ;
            break ;
        }
      }
   }
return 0 ;

}

int VensimLex::NextToken() // also sets token type 
{
   char c ;
   int toktype ;

   do { 
      c = GetNextChar(false) ;
   }  while(c == ' ' || c == '\t' || c ==  '\n'  || c == '\r') ;  // consume whitespace
   if(!c)
      return 0 ;
   sToken.clear() ;
   PushBack(c,false) ;
   c = GetNextChar(true) ;

   toktype = c ; // default for many tokens 
   switch(c)  {
      case '*' : // check for *** which toggles bInGroup
         if(TestTokenMatch("**",false)) {
            while((c = GetNextChar(true)) == '*')
               ;
            PushBack(c,true) ;
            bInGroup = !bInGroup ;
            return VPTT_groupstar ;
         }
         break ; 
      // single character tokens
      case '~' :
         iInUnitsComment++ ;
      case '=' : // := is handled by :
      case '/' :
      case '^' :
      case '!' :
      case '(' :
      case ')' :
      case '}' :
      case '[' :
      case ']' :
      case '|' :
      case ',' :
      case '+' :
      case '>' :
         break ;
      case '-' :
         if(TestTokenMatch(">",true)) 
            return VPTT_map ;
         break ;
      case '<' :
         if(TestTokenMatch("->",true)) 
            return VPTT_equiv ;
         break ;
      case '1' :
         if(iInUnitsComment==1) 
            return VPTT_units_symbol ;
      case '.' : // maybe a number check next digit
      case '0' :

      case '2' :
      case '3' :
      case '4' :
      case '5' :
      case '6' :
      case '7' :
      case '8' :
      case '9' : 
         // a number follow it through till it is no longer a number 
         if(c == '.') {
            if(bInGroup) { // treat as a group definition
               while(c = GetNextChar(true)) {
                  if(c == '\n' || c == '\r')
                     break ;
               }
               PushBack(c,true) ;
               return VPTT_groupname ;
            }
            else {
               c = GetNextChar(true) ;
               if(c < '0' || c > '9') {
                  PushBack(c,true) ;
                  break ; // not a number return '.'
               }
            }
         }
         else {
            GetDigits() ;
            c = GetNextChar(true) ;
            if(c != '.')
               PushBack(c,true) ;
         }
         toktype = VPTT_number ;
         if(c == '.') 
            GetDigits() ;
         c = GetNextChar(true) ;
         if(c == 'E' || c == 'e') { // xxx.xxxE+-xx 
            c = GetNextChar(true) ;
            if(c != '+' && c != '-') 
               PushBack(c,true) ;
            GetDigits() ;
         }
         else
            PushBack(c,true) ;
         break ;
      case ':' : // := :AND:  :HOLD BACKWARD: :IMPLIES: :INTERPOLATE: :LOOK FORWARD: :OR: :NA:  :NOT: :RAW: :TEST INPUT: :THE CONDITION: 
         c = GetNextChar(true) ;
         if(c == '=') 
            return VPTT_dataequals ;
         PushBack(c,true) ;
         return TestColonKeyword() ; // might return ':' 
      case '{' : // a comment, find matching }
         { // nesting, len local in scope
         int nesting = 1 ;
         int len = 1 ;
         MarkPosition() ;
         while(c = GetNextChar(false)) {
            len++ ;
            if(len > 1028)
               break ; // excessive comments not considered valid 
            if(c == '}') {
               nesting-- ;
               if(!nesting) { // comment is in sToken right now 
                  return NextToken() ; // just skip the comment for now - as if not there
                  }
            }
         }
         ReturnToMark() ; // failed to find pair give up
         }
         break ; // give up and just return the one char - will throw error message 
      case '\"' : // a quoted variable name potentially with embedded escaped quotes
         {
            int len ;
            MarkPosition() ;
            for(len=1;c = GetNextChar(true);len++) {
               if(c == '\"') {
                  toktype = iInUnitsComment==1?VPTT_units_symbol:VPTT_symbol ;
                  return toktype ;// the returned token includes both the opening and closing quote
               }
               else if(c == '\\') { // skip what follows in case it is a " or \  
                  GetNextChar(true) ;
                  len++ ;
               }
               if(len > 1024) // apparently unmmatched
                  break ;
            }
         }
         ReturnToMark() ;
        break ; // give up and just return the one char
      case '\\' : // either \\\---/// or a continuation line or an error
         if(TestTokenMatch("\\\\---///",false)) {
            assert(!sBuffer.length()) ;
            iCurPos-- ; // back up - wont be a problem with continuation lines in this case
            return VPTT_eqend ; // finished normal parse
         }
         break ;
      default : // a variable name or an unrecognizable token
         if(isalpha(c) || c > 127 || ((iInUnitsComment == 1) && c == '$')) { // a variable
            while(c = GetNextChar(true)) {
               if(!isalnum(c) && c != ' ' && c != '_' && c != '\t' && c  < 128) {
                  PushBack(c,true) ;
                  break ;
               }
            }
            toktype = iInUnitsComment==1?VPTT_units_symbol:VPTT_symbol ;
            return toktype ;
         }
   }
   return toktype ;
}

int VensimLex::TestColonKeyword()
{
   // := :AND:  :HOLD BACKWARD: :IMPLIES: :INTERPOLATE: :LOOK FORWARD: :OR: :NA:  :NOT: :RAW: :TEST INPUT: :THE CONDITION: 
   char *keywords[] = {":AND:",":EXCEPT:",":HOLD BACKWARD:",":IMPLIES:",":INTERPOLATE:",":LOOK FORWARD:",":OR:",":NA:",":NOT:",":RAW:",":TEST INPUT:",":THE CONDITION:",'\0' } ;
   int keyvals[] = {VPTT_and,VPTT_except,VPTT_hold_backward,VPTT_implies,VPTT_interpolate,VPTT_look_forward,VPTT_or,VPTT_na,VPTT_not,VPTT_raw,VPTT_test_input,VPTT_the_condition,-1} ;
   int i ;
   char c = GetNextChar(true) ;
   for(i=0;keywords[i];i++) {
      if(toupper(c) == keywords[i][1]) {
         if(KeywordMatch(keywords[i]+2))
            return keyvals[i] ;
      }
   }
   PushBack(c,true) ;
   return ':' ;
}
// target assumed upper case 
bool VensimLex::KeywordMatch(const char *target)
{
   std::string buffer ;
   char c ;
   int i ;
   for(i=0;target[i];i++) {
      c = GetNextChar(true) ;
      if(target[i] == ' ') { /* one or more _ or space ok here */
         if(c != ' ' && c != '_' && c != '\t')
            break ;
         buffer.push_back(c) ;
         while(c = GetNextChar(true)) {
            if(c != ' ' && c != '_' && c != '\t')
               break ;
            buffer.push_back(c) ;
         }
         PushBack(c,true) ;
      }
      else if(toupper(c) != target[i])
         break ;
      else
         buffer.push_back(c) ;
   }
   if(target[i]) { // not a match 
      while(buffer.length()) {
         PushBack(buffer.back(),true) ;
         buffer.pop_back() ;
      }
      return false ;
   }
   return true ;
}

char VensimLex::GetNextChar(bool store)
{
   char c ;
   if(sBuffer.length()) {
      c = sBuffer.back() ;
      sBuffer.pop_back() ;
      if(store)
         sToken.push_back(c) ;
      return c ;
   }
   if(iCurPos >= iFileLength)
      return 0 ; // nothing to do 
   c = ucContent[iCurPos++] ;
   if(c == '\\') { // check for continuation lines
      if(iCurPos < iFileLength && ucContent[iCurPos] == '\n' || ucContent[iCurPos] == '\r') {
         iLineNumber++ ;
         iLineStart = iCurPos+1 ; // actually the next pos
         for(iCurPos++;iCurPos < iFileLength;) {
            c = ucContent[iCurPos++] ;
            if(c != '\t' && c != ' ' && c != '\r' && c != '\n')
               break ;
            // note as in vensim two \ line ends in a row just cause an error
         }
      }
   }
   else if(c == '\n') {
      iLineNumber++ ;
      iLineStart = iCurPos ; // actually the next pos
   }
   if(store)
      sToken.push_back(c) ;
   return c ;
}

// check for token match - advance position on success
bool VensimLex::TestTokenMatch(const char *tok,bool storeonsuccess)
{
   char c ;
   if(!*tok)
      return true ;
   int i ;
   for(i=0;tok[i];i++) {
      if((c=GetNextChar(storeonsuccess)) != tok[i])
         break ;
   }
   if(tok[i]) {
      PushBack(c,storeonsuccess) ;
      while(i-- > 0)
         PushBack(tok[i],storeonsuccess) ;
      return false ;
   }
   return true ;
}


bool VensimLex::FindToken(const char *tok)
{
   char c ;
   while(c = GetNextChar(false)) {
      if(c == *tok && TestTokenMatch(tok+1,true))
         return true ;
      else if(c == '\\' && TestTokenMatch("\\\\---///",false))  {
         PushBack(c,false) ;
         SyncBuffers() ;
         return false ;
      }
   }
   return false ;

}

void VensimLex::PushBack(char c,bool store)
{
   assert(c) ;
   sBuffer.push_back(c) ;
   if(store)
      sToken.pop_back() ;
}
void VensimLex::SyncBuffers(void)
{
   int i = sBuffer.length() ;
   iCurPos -= i ;
   sBuffer.empty() ;
}
