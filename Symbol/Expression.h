/* Abstract class for expression - the different expression types
   are all tiny extensions and included in the same files */
#ifndef _XMUTIL_SYMBOL_EXPRESSION_H
#define _XMUTIL_SYMBOL_EXPRESSION_H
#include "SymbolTableBase.h"
#include "SymbolList.h" 
#include "Variable.h"
#include "../Function/Function.h"
#include "../Function/State.h"
#include "../Function/TableFunction.h"
#include "../ContextInfo.h" 
#include <math.h>
#include <float.h>

class ExpressionList ; // forward declaration
class Model ;


// probably don't need these after all
enum EXPTYPE {
   EXPTYPE_None,
   EXPTYPE_Variable,
   EXPTYPE_Symlist,
   EXPTYPE_Number,
   EXPTYPE_NumberTable,
   EXPTYPE_Function,
   EXPTYPE_FunctionMemory,
   EXPTYPE_Lookup,
   EXPTYPE_Table,
   EXPTYPE_Operator
} ;


class Expression :
   public SymbolTableBase
{
public:
   Expression(SymbolNameSpace *sns);
   virtual ~Expression(void) = 0;
   virtual EXPTYPE GetType(void) { return EXPTYPE_None ; }
   virtual double Eval(ContextInfo *info) = 0 ;
   virtual void FlipSign(void) {}
   virtual Function *GetFunction(void) { return '\0' ; }
   virtual void CheckPlaceholderVars(Model *m,bool isfirst) = 0 ;// generally do nothing, but big error to skip
   virtual bool CheckComputed(ContextInfo *info) { return true ; }
   virtual void RemoveFunctionArgs(void) {} // only 1 subclass does anything
   virtual void OutputComputable(ContextInfo *info) = 0 ; // again don't skip

};

class ExpressionVariable :
   public Expression 
{
public :
   inline ExpressionVariable(SymbolNameSpace *sns,Variable *var,SymbolList *subs) : Expression(sns) {pVariable = var;pSubList = subs;}
   inline ~ExpressionVariable(void) {if(HasGoodAlloc()) {if(pSubList) delete pSubList ;/* leave pVariable alone */}}
   inline EXPTYPE GetType(void) { return EXPTYPE_Variable ; }
   inline Variable *GetVariable(void) { return pVariable ; }
   inline void CheckPlaceholderVars(Model *m,bool isfirst) {}
   bool CheckComputed(ContextInfo *info) { return pVariable->CheckComputed(info,false) ; }
   double Eval(ContextInfo *info) { return pVariable->Eval(info) ; } 
   void OutputComputable(ContextInfo *info) { *info << pVariable->GetAlternateName() ; }
private :
   Variable *pVariable ; // pointer back to the model variable - not allocated by this object
   SymbolList *pSubList ; // subscripts - allocated by this object
} ;

class ExpressionSymbolList :
   public Expression 
{
public :
   inline ExpressionSymbolList(SymbolNameSpace *sns,SymbolList *subs,SymbolList *map) : Expression(sns) {pSymList = subs;pMap = map ;}
   inline ~ExpressionSymbolList(void) {if(HasGoodAlloc()) {if(pSymList) delete pSymList ; if(pMap) delete pMap ;}}
   inline EXPTYPE GetType(void) { return EXPTYPE_Symlist ; }
   inline void CheckPlaceholderVars(Model *m,bool isfirst) {}
   bool CheckComputed(ContextInfo *info) { return true ; }
   double Eval(ContextInfo *info) { return -FLT_MAX ; } 
   void OutputComputable(ContextInfo *info) { assert(0) ; }
private :
   SymbolList *pSymList ; // subscript elements - should be deleted by this object
   SymbolList *pMap ; // 
} ;

class ExpressionNumber :
   public Expression
{
public :
   inline ExpressionNumber(SymbolNameSpace *sns,double num) : Expression(sns) {value = num ;}
   inline ~ExpressionNumber(void) {}
   inline EXPTYPE GetType(void) { return EXPTYPE_Number ; }
   void FlipSign(void) { value = -value ; }
   inline double Eval(ContextInfo *info) { return value ; }
   inline void CheckPlaceholderVars(Model *m,bool isfirst) {}
   void OutputComputable(ContextInfo *info) { *info << value ; }
private :
   double value ;
} ;
class ExpressionNumberTable :
   public Expression
{
public :
   inline ExpressionNumberTable(SymbolNameSpace *sns) : Expression(sns) {}
   inline ~ExpressionNumberTable(void) {}
   inline EXPTYPE GetType(void) { return EXPTYPE_NumberTable ; }
   inline double Eval(ContextInfo *info) { return -FLT_MAX ; }
   inline void CheckPlaceholderVars(Model *m,bool isfirst) {}
   void OutputComputable(ContextInfo *info) { *info << " ??? " ; }
   void AddValue(unsigned row,double num) {if(row+1 > vRow.size()) vRow.resize(row+1) ; vRow[row].push_back(num) ; }
   int Count(unsigned row) { if(row < vRow.size()) return 0 ; return vRow[row].size() ; }
   typedef  std::vector<double> ColVals ;
private :
   std::vector<ColVals> vRow ;
} ;

class ExpressionFunction :
   public Expression
{
public :
   ExpressionFunction(SymbolNameSpace *sns,Function *f,ExpressionList *args) : Expression(sns) {pFunction =f;pArgs = args;}
   ~ExpressionFunction(void) ;
   inline EXPTYPE GetType(void) { return EXPTYPE_Function ; }
   inline double Eval(ContextInfo *info) { return pFunction->Eval(this,pArgs,info) ; }
   inline Function *GetFunction(void) { return pFunction ; }
   inline ExpressionList *GetArgs(void) { return pArgs ; }
   void CheckPlaceholderVars(Model *m,bool isfirst) ;
   bool CheckComputed(ContextInfo *info) { return pFunction->CheckComputed(info,pArgs) ; }
   void RemoveFunctionArgs(void) { pArgs = '\0' ; }
   void OutputComputable(ContextInfo *info)  { pFunction->OutputComputable(info,pArgs) ; }

private :
   Function *pFunction ; // not allocated here
   ExpressionList *pArgs ;
} ;
class ExpressionFunctionMemory :
   public ExpressionFunction
{
public :
   ExpressionFunctionMemory(SymbolNameSpace *sns,Function *f,ExpressionList *args) : ExpressionFunction(sns,f,args) {pPlacholderEquation = '\0' ;}
   ~ExpressionFunctionMemory(void) { }
   inline EXPTYPE GetType(void) { return EXPTYPE_FunctionMemory ; }
   inline double Eval(ContextInfo *info) {if(pPlacholderEquation) return pPlacholderEquation->GetVariable()->Eval(info);return ExpressionFunction::Eval(info) ; }
   void CheckPlaceholderVars(Model *m,bool isfirst) ;
   bool CheckComputed(ContextInfo *info) {if(pPlacholderEquation) return pPlacholderEquation->GetVariable()->CheckComputed(info,false);return ExpressionFunction::CheckComputed(info) ; }
   void OutputComputable(ContextInfo *info)  {if(pPlacholderEquation) return pPlacholderEquation->GetVariable()->OutputComputable(info);return ExpressionFunction::OutputComputable(info) ; }
private :
   Equation *pPlacholderEquation ; // used in computation (null if function defines LHS)
} ;


class ExpressionLookup :
   public Expression 
{
public:
   ExpressionLookup(SymbolNameSpace *sns,ExpressionVariable *var,Expression *e) : Expression(sns) {pExpressionVariable =var;pExpression = e ; }
   ~ExpressionLookup(void) {if(HasGoodAlloc()) { delete pExpressionVariable;delete pExpression;}}
   inline EXPTYPE GetType(void) { return EXPTYPE_Lookup ; }
   void CheckPlaceholderVars(Model *m,bool isfirst) { pExpression->CheckPlaceholderVars(m,false) ; }
   bool CheckComputed(ContextInfo *info) { return pExpression->CheckComputed(info) ; }
   double Eval(ContextInfo *info) { return TableFunction::Eval(pExpressionVariable,pExpression,info)  ; } 
   void OutputComputable(ContextInfo *info) { *info << "fLookup(" << pExpressionVariable->GetVariable()->GetAlternateName() << "," ; pExpression->OutputComputable(info);*info<<")";}
private:
   ExpressionVariable *pExpressionVariable ;
   Expression *pExpression ;
} ;

class ExpressionTable :
   public Expression
{
public :
   ExpressionTable(SymbolNameSpace *sns) : Expression(sns) { bHasRange=false;}
   ~ExpressionTable(void) {/* vector destructors only*/ }
   inline EXPTYPE GetType(void) { return EXPTYPE_Table ; }
   void AddPair(double x,double y) { vXVals.push_back(x);vYVals.push_back(y);}
   void AddRange(double x1,double y1,double x2,double y2) { bHasRange=true;dX2=x2;dY1=y1;dX2=x2;dY2=y2;}
   inline void CheckPlaceholderVars(Model *m,bool isfirst) {}
   double Eval(ContextInfo *info) { assert(0) ; return FLT_MAX ; } 
   std::vector<double>*GetXVals(void) { return &vXVals ; }
   std::vector<double>*GetYVals(void) { return &vYVals ; }
   void OutputComputable(ContextInfo *info) { *info << "??" ; }


private :
   std::vector<double> vXVals ;
   std::vector<double> vYVals ;
   bool bHasRange ;
   double dX1,dY1,dX2,dY2 ;
} ;



class ExpressionOperator2 :
   public Expression
{
public :
   ExpressionOperator2(SymbolNameSpace *sns,Expression *e1,Expression *e2) : Expression(sns) {pE1=e1;pE2=e2;}
   ~ExpressionOperator2(void) {if(HasGoodAlloc()){if(pE1) delete pE1;if(pE2) delete pE2;}}
   inline EXPTYPE GetType(void) { return EXPTYPE_Operator ; }
   void CheckPlaceholderVars(Model *m,bool isfirst) { if(pE1) pE1->CheckPlaceholderVars(m,false);if(pE2)pE2->CheckPlaceholderVars(m,false);}
   bool CheckComputed(ContextInfo *info) { if(pE1 && !pE1->CheckComputed(info))return false ;if(pE2 && !pE2->CheckComputed(info)) return false; return true ; }
   void OutputComputable(ContextInfo *info) { }
protected :
   Expression *pE1 ;
   Expression *pE2 ;
} ;

#define EO2SubClassRaw(name,evaleq,before,middle,after) \
class name : public ExpressionOperator2{\
public : \
   name(SymbolNameSpace *sns,Expression *e1,Expression *e2) : ExpressionOperator2(sns,e1,e2) {}\
   ~name(void) {}\
   inline double Eval(ContextInfo *info) { return (evaleq); }\
   void OutputComputable(ContextInfo *info) { *info << before;if(pE1)pE1->OutputComputable(info);*info<<middle;if(pE2)pE2->OutputComputable(info);*info<<after; }\
} ;
#define EO2SubClass(name,evaleq,compsym) EO2SubClassRaw(name,evaleq,"",compsym,"") ;

EO2SubClass(ExpressionMultiply,pE1->Eval(info)*pE2->Eval(info),"*")
EO2SubClass(ExpressionDivide,pE1->Eval(info)/pE2->Eval(info),"/")
EO2SubClass(ExpressionAdd,pE1->Eval(info)+pE2->Eval(info),"+")
EO2SubClass(ExpressionSubtract,pE1->Eval(info)-pE2->Eval(info),"-")
EO2SubClass(ExpressionPower,exp(log(pE1->Eval(info))*pE2->Eval(info)),"^")
EO2SubClassRaw(ExpressionParen,pE1->Eval(info),"(","",")")
EO2SubClassRaw(ExpressionUnaryMinus,(-pE1->Eval(info)),"-","","")

/*
{

class ExpressionMultiply :
   public Expression
{
public :
   ExpressionMultiply(SymbolNameSpace *sns,Expression *exp1,Expression *exp2) : Expression(sal) {pE1 = exp1;pE2=exp2;}
   ~ExpressionMultiply(void) {if(HasGoodAlloc()){delete pE1;delete pE2;}}
   inline double Eval(void) { return pE1->Eval() * pE2->Eval() ; }
private :
   Expression *pE1 ;
   Expression *pE2 ;
} ;

*/

#endif
