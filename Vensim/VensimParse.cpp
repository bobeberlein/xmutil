// VensimParse.cpp : Read an mdl file into an XModel object
// we use a mapped file via boost to simplify look ahead/back
// we include the tokenizer here because it is as easy as settinig
// up regular expressions for Flex and more easily understood

#include "VensimParse.h"
#include "../Symbol/Variable.h"
#include "../Symbol/LeftHandSide.h"
#include "../Symbol/ExpressionList.h"
#include <boost/lexical_cast.hpp>
extern "C" {
/* VYacc.tab.h is C and - most importantly vpyyparse appears
  without extern "C" so by prefacing everything we can include
  it hear and not cause linke issues */
#define YYSTYPE VensimParse
#include "VYacc.tab.h"
}
#include "../XMUtil.h"


VensimParse *VPObject = '\0' ;


VensimParse::VensimParse(SymbolNameSpace *sns)
{
	vpyydebug = 1;
   assert(!VPObject) ;
   VPObject = this ;
   pSymbolNameSpace = sns ;

   ReadyFunctions();

}
VensimParse:: ~VensimParse(void)
{
   VPObject = '\0' ;
}

void VensimParse::ReadyFunctions()
{
	// initialize functions - actually need to split this up for common functions
	// and those specific to Vensim - later
	try {
		new FunctionMin(pSymbolNameSpace);
		new FunctionMax(pSymbolNameSpace);
		new FunctionInteg(pSymbolNameSpace);
		new FunctionPulse(pSymbolNameSpace);
		new FunctionIfThenElse(pSymbolNameSpace);
		new FunctionStep(pSymbolNameSpace);
		new FunctionTabbedArray(pSymbolNameSpace);
		pSymbolNameSpace->ConfirmAllAllocations();
	}
	catch (...) {
		std::cout << "Failed to initialize symbol table" << std::endl;
	}

}
Equation *VensimParse::AddEq(LeftHandSide *lhs,Expression *ex,ExpressionList *exl,int tok) 
 { 
    if(exl) {
       if(exl->Length() == 1) {
          ex = exl->GetExp(0) ;
          delete exl ;
       } else { /* only a list of numbers is valid here - throw exception for anything else */
          ExpressionNumberTable *ent = new ExpressionNumberTable(pSymbolNameSpace) ;
          int n = exl->Length() ;
          int i ;
          for(i=0;i<n;i++) {
             ex = exl->GetExp(i) ;
             if(ex->GetType() != EXPTYPE_Number) {
                mSyntaxError.str = "Expecting only comma delimited numbers " ;
                throw mSyntaxError ;
             }
             ent->AddValue(0,ex->Eval('\0')) ; // note eval does not need context for number
             delete ex ;
          }
          delete exl ;
          ex = ent ;
       }
    }

    return new Equation(pSymbolNameSpace,lhs,ex,tok) ; 
}

/* a full eq cleans up the temporary memory space and 
  also assigns the equation to the Variable */
void VensimParse::AddFullEq(Equation *eq,UnitExpression *un)
{
	if (mInMacro) {
		mMacroFunctions.back()->AddEq(eq, un);
		return;
	}
   pSymbolNameSpace->ConfirmAllAllocations() ; // now independently allocated
   pActiveVar = eq->GetVariable() ;
   pActiveVar->AddEq(eq) ;
   if(un) {
      if(!pActiveVar->AddUnits(un))
         delete un ;
   }
}

int VensimParse::yyerror(const char *str)
{
   mSyntaxError.str = str ;
   throw mSyntaxError ;
}

bool VensimParse::ProcessFile(const std::string &filename)
{
   sFilename = filename ;
    try {
      mfSource.open(filename);
    }
    catch(...) {
       return false ;
    }
    if(mfSource.is_open()) { 
       bool noerr = true ;
       mVensimLex.Initialize((const char *)mfSource.data(), mfSource.size()) ;
       int endtok = mVensimLex.GetEndToken() ;
       // now we call the bison built parser which will call back to VensimLex
       // for the tokenizing - 
       int rval ;
       do {
          rval = 0 ;
          try {
             mVensimLex.GetReady() ;
             rval = vpyyparse() ;
             if(rval == '~') { // comment follows 
                if(!FindNextEq())
                   break ;
             } 
             else if(rval == '|') {
             }
             else if(rval == VPTT_groupstar) {
                if(yylex() == VPTT_groupname) {
                }
                if(!FindNextEq())
                   break ;
             }
             else if(rval != endtok) {
                std::cout << "Unknown terminal token " << rval << std::endl ;
                if(!FindNextEq())
                   break ;
             }

          }
          catch(VensimParseSyntaxError& e) {
             std::cout << e.str << std::endl ;
             std::cout << "Error at line " << mVensimLex.LineNumber() << " position " << mVensimLex.Position() 
                << " in file " << sFilename << std::endl ;
             pSymbolNameSpace->DeleteAllUnconfirmedAllocations() ;
             noerr = false ;
             if(!FindNextEq()) 
                break ;

          }
          catch(...) {
             pSymbolNameSpace->DeleteAllUnconfirmedAllocations() ;
             noerr = false ;
             if(!FindNextEq())
                break ;

          }
       } while(rval != endtok) ;
       mfSource.close() ;
       return noerr ;
    }
    else
       return false ;
}


Variable *VensimParse::InsertVariable(const std::string &name)
{
   Variable *var =  static_cast<Variable *>(pSymbolNameSpace->Find(name)) ;
   if(var && var->isType() != Symtype_Variable &&
      var->isType() != Symtype_Function) {
      mSyntaxError.str = "Type meaning mismatch for " + name ;
      throw mSyntaxError ;
   }
   if(!var) {
      var = new Variable(pSymbolNameSpace,name) ;
      // this will insert it into the name space for hash lookup as well
   }
   return var ;
}
Units *VensimParse::InsertUnits(const std::string &name)
{
   Units *u =  static_cast<Units *>(pSymbolNameSpace->Find(name)) ;
   if(u && u->isType() != Symtype_Units && ( !mInMacro || u->isType() != Symtype_Variable)) {
      mSyntaxError.str = "Type meaning mismatch for " + name ;
      throw mSyntaxError ;
   }
   if(!u) {
      u = new Units(pSymbolNameSpace,name) ;
   }
   return u ;
}

UnitExpression *VensimParse::InsertUnitExpression(Units *u) 
{
   UnitExpression *uni = new UnitExpression(pSymbolNameSpace,u) ;
   return uni ;
}

// find the beginning of the next equation - for error recovery
bool VensimParse::FindNextEq(void)
{
   // just zip through to the first | then whatever follows is it
   return mVensimLex.FindToken("|") ;
}


LeftHandSide *VensimParse::AddExceptInterp(ExpressionVariable *var,SymbolListList *except,int interpmode) 
{ 
   return new LeftHandSide(pSymbolNameSpace,var,except,interpmode) ;
}
SymbolList *VensimParse::SymList(SymbolList *in,Variable *add,int bang,Variable *end) 
{
   SymbolList *sl ;
   if(in)
      sl =  in->Append(add,bang) ;
   else
     sl =  new SymbolList(pSymbolNameSpace,add,bang) ;
   if(end) { /* actually shortcut for (axx-ayy) */
      int i,j ;
      Variable *v ;
      std::string start = add->GetName() ;
      std::string finish = end->GetName() ;
      for(i=start.length();--i > 0;) {
         if(start[i-1] < '0' || start[i-1] > '9')
            break ;
      }
      for(j=finish.length();--j > 0;) {
         if(finish[j-1] < '0' || finish[j-1] > '9')
            break ;
      }
      int low = atoi(start.c_str() + i) ;
      int high = atoi(finish.c_str() + j) ;
      if(i != j || start.compare(0,j,finish,0,j) || low >= high) {
         mSyntaxError.str = "Bad subscript range specification" ;
         throw mSyntaxError ;
      }
      start.erase(i,std::string::npos) ;
      for(i=low+1;i<high;i++) {
         finish = start + boost::lexical_cast<std::string>(i) ;
         v = static_cast<Variable *>(pSymbolNameSpace->Find(finish)) ;
         if(!v)
            v = new Variable(pSymbolNameSpace,finish) ;
         sl->Append(v,bang) ;
      }
      sl->Append(end,bang) ;
   }
   return sl ;
}
UnitExpression *VensimParse::UnitsDiv(UnitExpression *num,UnitExpression *denom) 
{
   return num->Divide(denom) ;
}
UnitExpression *VensimParse::UnitsMult(UnitExpression *f,UnitExpression *s) 
{ 
   return f->Multiply(s) ;
}
UnitExpression *VensimParse::UnitsRange(UnitExpression *e,double minval,double maxval) 
{ 
   e->SetRange(minval,maxval) ;
   return e ;
}

SymbolListList *VensimParse::ChainSublist(SymbolListList *sll,SymbolList *nsl) 
{ 
   return sll->Append(nsl) ;
}
ExpressionList *VensimParse::ChainExpressionList(ExpressionList *el,Expression *e) 
{
   if(!el)
      el = new ExpressionList(pSymbolNameSpace) ;
   return el->Append(e) ;
}
Expression *VensimParse::NumExpression(double num) 
{ 
   return new ExpressionNumber(pSymbolNameSpace,num) ;
}
ExpressionVariable *VensimParse::VarExpression(Variable *var,SymbolList *subs) 
{ 
   return new ExpressionVariable(pSymbolNameSpace,var,subs) ;
}
ExpressionSymbolList *VensimParse::SymlistExpression(SymbolList *subs,SymbolList *map) 
{ 
   return new ExpressionSymbolList(pSymbolNameSpace,subs,map) ;
}
Expression *VensimParse::OperatorExpression(int oper,Expression *exp1,Expression *exp2) 
{  
   switch(oper) {
   case '*' :
      return new ExpressionMultiply(pSymbolNameSpace,exp1,exp2) ;
   case '/' :
      return new ExpressionDivide(pSymbolNameSpace,exp1,exp2) ;
   case '+' :
      if(!exp2 && exp1 && exp1->GetType() == EXPTYPE_Number)
         return exp1 ; /* unary plus just ignore */
      return new ExpressionAdd(pSymbolNameSpace,exp1,exp2) ;
   case '-' :
      if(!exp2) {
         if(exp1 && exp1->GetType() == EXPTYPE_Number) {
            exp1->FlipSign() ;
            return exp1 ;
         }
         return new ExpressionUnaryMinus(pSymbolNameSpace,exp1,'\0') ; 
      }
      return new ExpressionSubtract(pSymbolNameSpace,exp1,exp2) ;
   case '^' :
      return new ExpressionPower(pSymbolNameSpace,exp1,exp2) ;
   case '(' :
      assert(!exp2) ;
      return new ExpressionParen(pSymbolNameSpace,exp1,'\0') ;
   case '<':
   case '>':
   case VPTT_le:
   case VPTT_ge:
   case VPTT_and:
   case VPTT_or:
   case '=':
	   return new ExpressionLogical(pSymbolNameSpace,exp1,exp2,oper);
   default :
     mSyntaxError.str = "Unknown operator internal error "  ;
     throw mSyntaxError ;
   }
}
Expression *VensimParse::FunctionExpression(Function *func,ExpressionList *eargs) 
{  
   if(func->NumberArgs() != eargs->Length()) {
      mSyntaxError.str = "Argument count mismatch for "  ;
      mSyntaxError.str.append(func->GetName()) ;
      throw mSyntaxError ;
   }
   if(func->IsMemoryless() && !func->IsTimeDependent())
       return new ExpressionFunction(pSymbolNameSpace,func,eargs) ;
   return new ExpressionFunctionMemory(pSymbolNameSpace,func,eargs) ;
}
Expression *VensimParse::LookupExpression(ExpressionVariable *var,Expression *exp) 
{
   return new ExpressionLookup(pSymbolNameSpace,var,exp) ;
}

ExpressionTable *VensimParse::TablePairs(ExpressionTable *table,double x,double y) 
{
   if(!table)
      table = new ExpressionTable(pSymbolNameSpace) ;
   table->AddPair(x,y) ;
   return table ;
}

ExpressionTable *VensimParse::TableRange(ExpressionTable *table,double x1,double y1,double x2,double y2) 
{
   table->AddRange(x1,y1,x2,y2) ;
   return table ;
}

void VensimParse::MacroStart()
{
	mInMacro = true;
	pMainSymbolNameSpace = pSymbolNameSpace;
	pSymbolNameSpace = new SymbolNameSpace(); // local name space for macro variables and macro name - macro name will go into the main name space on close
	ReadyFunctions(); // against this new name space - somewhat duplicative
}

void VensimParse::MacroExpression(Variable* name, ExpressionList *margs)
{
	// the macro functiongoes against the main name space - everything else is local
	mMacroFunctions.push_back(new MacroFunction(pMainSymbolNameSpace, pSymbolNameSpace, name->GetName(), margs));
}
void VensimParse::MacroEnd()
{
	pSymbolNameSpace = pMainSymbolNameSpace;
	mInMacro = false;
}
