#ifndef _XMUTIL_SYMBOL_FUNCTION_H
#define _XMUTIL_SYMBOL_FUNCTION_H
#include "../Symbol/Symbol.h"
#include <boost/utility.hpp>
#include "State.h"

class Expression ; /* forward declaration */
class ExpressionList ; // forward
class UnitExpression;

/* abstract class - every function has its own subclass
   defined here with bodies in Function.cpp or in their own 
   files depending on complexity */

class Function :
   public Symbol
{
public:
   Function(SymbolNameSpace *sns,const std::string &name,int narg) ;
   virtual ~Function(void) = 0 ;
   SYMTYPE isType(void) { return Symtype_Function ; }
   virtual bool AsKeyword(void) { return false ; } // for the parser - treats name as keyword not function
   virtual bool IsMemoryless(void) { return true ; }
   virtual bool IsTimeDependent(void) { return false ; }
   virtual double Eval(Expression *ex, ExpressionList *arg, ContextInfo *info) { return 0; } // make this pure virtual if finishing engine is required
   virtual bool CheckComputed(ContextInfo *info,ExpressionList *arg) ;
   virtual void OutputComputable(ContextInfo *info,ExpressionList *arg) ;
   virtual std::string ComputableName(void) { return "" ; }
   virtual std::string ComputableNameInit(void) { return "" ; }
   int NumberArgs(void) { return iNumberArgs ; }
protected :
   int iNumberArgs ;
};


class FunctionMemoryBase : public Function {
public:
   FunctionMemoryBase(SymbolNameSpace *sns,const std::string &name,int narg,unsigned actarg,unsigned iniarg) : Function(sns,name,narg) {iActiveArgMark=BitFlip(actarg) ; iInitArgMark = BitFlip(iniarg) ;}
   ~FunctionMemoryBase(void) {}
   unsigned BitFlip(unsigned bits) ;
   bool IsMemoryless(void) { return false ; }
   bool CheckComputed(ContextInfo *info,ExpressionList *arg) ;
   void OutputComputable(ContextInfo *info,ExpressionList *arg) ;
private :
   unsigned iInitArgMark ;
   unsigned iActiveArgMark ;
} ;

class MacroFunction : public Function {
public:
	class EqUnitPair {
	public:
		EqUnitPair(Equation* eq, UnitExpression* un) : equation(eq), units(un) {}
		Equation* equation;
		UnitExpression* units;
	};
	MacroFunction(SymbolNameSpace *sns, SymbolNameSpace* local, const std::string& name, ExpressionList *margs);
	~MacroFunction() { delete pSymbolNameSpace; }
	void AddEq(Equation* equation, UnitExpression* units) { mEquations.push_back(EqUnitPair(equation, units)); }
	virtual std::string ComputableName(void);
private:
	SymbolNameSpace *pSymbolNameSpace; // local
	MacroFunction(const MacroFunction& other);
	ExpressionList* mArgs;
	std::vector<EqUnitPair> mEquations;
};

#define FSubclassKeyword(name,xname,narg) \
class name : public Function {\
public :\
   name(SymbolNameSpace *sns) : Function(sns,xname,narg) { ; }\
   ~name(void) {}\
    std::string ComputableName(void) { return " ?? " ; }\
    bool AsKeyword(void) { return true ; } \
} ;




#define FSubclassStart(name,xname,narg,cname) \
class name : public Function {\
public :\
   name(SymbolNameSpace *sns) : Function(sns,xname,narg) { ; }\
   ~name(void) {}\
    std::string ComputableName(void) { return cname ; }\
private :

#define FSubclass(name,xname,narg,cname) FSubclassStart(name,xname,narg,cname) };




#define FSubclassMemoryStart(name,xname,narg,actarg,iniarg,cnamea,cnamei) \
class name : public FunctionMemoryBase {\
public :\
   name(SymbolNameSpace *sns) : FunctionMemoryBase(sns,xname,narg,actarg,iniarg) { }\
   ~name(void) {}\
   std::string ComputableName(void) { return cnamea ; }\
   std::string ComputableNameInit(void) { return cnamei ; }\
private :

#define FSubclassMemory(name,xname,narg,actarg,iniarg,cnamea,cnamei) FSubclassMemoryStart(name,xname,narg,actarg,iniarg,cnamea,cnamei) };


#define FSubclassTimeStart(name,xname,narg,cname) \
class name : public Function {\
public :\
   name(SymbolNameSpace *sns) : Function(sns,xname,narg) { ; }\
   ~name(void) {}\
   bool IsTimeDependent(void) { return true ; }\
   std::string ComputableName(void) { return cname ; }\
private :

#define FSubclassTime(name,xname,narg,cname) FSubclassTimeStart(name,xname,narg,cname) };

FSubclass(FunctionAbs, "ABS", 1, "ABS")
FSubclass(FunctionExp, "EXP", 1, "EXP")
FSubclass(FunctionSqrt, "SQRT", 1, "SQRT")

FSubclass(FunctionCosine, "COS", 1, "COS")
FSubclass(FunctionTangent, "TAN", 1, "TAN")
FSubclass(FunctionSine, "SIN", 1, "SIN")
FSubclass(FunctionArcCosine, "ARCCOS", 1, "ARCCOS")
FSubclass(FunctionArcSine, "ARCSIN", 1, "ARCSIN")
FSubclass(FunctionArcTangent, "ARCTAN", 1, "ARCTAN")
FSubclass(FunctionInterger, "INTEGER", 1, "INT")

FSubclass(FunctionMax, "MAX", 2, "MAX")
FSubclass(FunctionMin, "MIN", 2, "MIN")
FSubclass(FunctionZidz, "ZIDZ", 2, "SAFEDIV")
FSubclass(FunctionXidz, "XIDZ", 3, "SAFEDIV")
FSubclass(FunctionWithLookup, "WITH LOOKUP", 3, "WITH_LOOKUP")
FSubclass(FunctionSum, "SUM", 1, "SUM");
FSubclass(FunctionVectorSelect, "VECTOR SELECT", 5, "VECTOR SELECT");
FSubclass(FunctionVectorElmMap, "VECTOR ELM MAP", 2, "VECTOR ELM MAP");
FSubclass(FunctionVectorSortOrder, "VECTOR SORT ORDER", 2, "VECTOR SORT ORDER");
FSubclass(FunctionGame, "GAME", 1, ""); // don't need this 
FSubclass(FunctionRandom01, "RANDOM 0 1", 0, "UNIFORM(0,1)");

// actually memory but no init - or init - does not matter for translation
FSubclass(FunctionSmooth, "SMOOTH", 2, "SMTH1")
FSubclass(FunctionSmoothI, "SMOOTHI", 3, "SMTH1")
FSubclass(FunctionSmooth3, "SMOOTH3", 2, "SMTH3")
FSubclass(FunctionTrend,"TREND", 3, "TREND")
FSubclass(FunctionDelay3, "DELAY3", 2, "DELAY3")
FSubclass(FunctionDelay, "DELAY FIXED", 3, "DELAY")
FSubclass(FunctionNPV,"NPV",4,"NPV")

// done as macros
FSubclass(FunctionDelayConveyor,"DELAY CONVEYOR", 6, "DELAY_CONVEYOR")
// - this one is fake - return NaN
FSubclass(FunctionVectorReorder,"VECTOR REORDER", 2, "VECTOR_REORDER")
FSubclass(FunctionModulo,"MODULO",2,"MODULO")
FSubclass(FunctionGetDataAtTime, "GET DATA AT TIME", 2, "GET_DATA_AT_TIME")
FSubclass(FunctionGetDataLastTime, "GET DATA LAST TIME", 1, "GET_DATA_LAST_TIME")
FSubclass(FunctionLookupArea, "LOOKUP AREA", 3, "LOOKUP_AREA")
FSubclass(FunctionRandomNormal, "RANDOM NORMAL", 5, "RANDOM_NORMAL")

FSubclassStart(FunctionDelayN, "DELAY N", 4, "DELAYN")
public:
virtual void OutputComputable(ContextInfo *info, ExpressionList *arg);
};
FSubclassStart(FunctionSmoothN, "SMOOTH N", 4, "SMTHN")
public:
	virtual void OutputComputable(ContextInfo *info, ExpressionList *arg);
};


FSubclassMemory(FunctionInteg, "INTEG", 2, BOOST_BINARY(10), BOOST_BINARY(01), "integ_active", "integ_init")
FSubclassMemory(FunctionActiveInitial, "ACTIVE INITIAL", 2, BOOST_BINARY(10), BOOST_BINARY(01), "ai_active", "ai_init")
FSubclassMemory(FunctionInitial, "INITIAL", 1, BOOST_BINARY(0), BOOST_BINARY(1), "no active equation only init", "INITIAL")

FSubclassTime(FunctionRamp, "RAMP", 3, "RAMP")
FSubclass(FunctionLn, "LN", 1, "LN")
FSubclassTime(FunctionPulse, "PULSE", 2, "pulse")
FSubclassTime(FunctionStep, "STEP", 2, "step")

FSubclassKeyword(FunctionTabbedArray, "TABBED ARRAY", 1)

// functions that will never translate - but easier to catch error on other side
FSubclass(FunctionGetDirectData, "GET DIRECT DATA", 4, "GET_DIRECT_DATA")

/*
class FunctionMin :
   public Function
{
public :
   FunctionMin(SymbolNameSpace *sns) : Function(sal,"MIN",2) { ; }
   ~FunctionMin(void) { }
   inline double Eval(Expression *,ExpressionList *arg) ;
} ;

*/


class FunctionSampleIfTrue : public Function
{
public:
	FunctionSampleIfTrue(SymbolNameSpace *sns) : Function(sns, "SAMPLE IF TRUE", 3) {}
	~FunctionSampleIfTrue(void) {}
	std::string ComputableName(void) { return "SampleIfTrue"; }
	virtual void OutputComputable(ContextInfo *info, ExpressionList *arg);
private:
};


class FunctionIfThenElse : public Function
{
public:
	FunctionIfThenElse(SymbolNameSpace *sns) : Function(sns, "IF THEN ELSE", 3) { ; }\
		~FunctionIfThenElse(void) {}
	std::string ComputableName(void) { return "IF"; }
	virtual void OutputComputable(ContextInfo *info, ExpressionList *arg);
private:
};

class FunctionLog : public Function
{
public:
	FunctionLog(SymbolNameSpace *sns) : Function(sns, "LOG", 2) { ; }\
		~FunctionLog(void) {}
	std::string ComputableName(void) { return "LOG10"; }
	virtual void OutputComputable(ContextInfo *info, ExpressionList *arg);
private:
};
#endif
