// VensimParse.cpp : Read an mdl file into an XModel object
// we use a mapped file via boost to simplify look ahead/back
// we include the tokenizer here because it is as easy as settinig
// up regular expressions for Flex and more easily understood

#include "VensimParse.h"
#include "../Symbol/Variable.h"
#include "../Symbol/LeftHandSide.h"
#include "../Symbol/ExpressionList.h"
#include <boost/lexical_cast.hpp>
#define YYSTYPE VensimParse
#include "VYacc.tab.hpp"
#include "../XMUtil.h"
#include "VensimView.h"


VensimParse *VPObject = NULL ;


VensimParse::VensimParse(Model* model)
{
#if YYDEBUG
	vpyydebug = 0;
#endif
   assert(!VPObject) ;
   VPObject = this ;
   _model = model;
   pSymbolNameSpace = model->GetNameSpace() ;
   bLongName = false;
   ReadyFunctions();

}
VensimParse:: ~VensimParse(void)
{
   VPObject = NULL ;
}

void VensimParse::ReadyFunctions()
{
	// initialize functions - actually need to split this up for common functions
	// and those specific to Vensim - later
	try {
		new FunctionMin(pSymbolNameSpace);
		new FunctionMax(pSymbolNameSpace);
		new FunctionInteg(pSymbolNameSpace);
		new FunctionActiveInitial(pSymbolNameSpace);
		new FunctionInitial(pSymbolNameSpace);
		new FunctionReInitial(pSymbolNameSpace);
		new FunctionSampleIfTrue(pSymbolNameSpace);
		new FunctionPulse(pSymbolNameSpace);
		new FunctionPulseTrain(pSymbolNameSpace);
		new FunctionQuantum(pSymbolNameSpace);
		new FunctionIfThenElse(pSymbolNameSpace);
		new FunctionLog(pSymbolNameSpace);
		new FunctionZidz(pSymbolNameSpace);
		new FunctionXidz(pSymbolNameSpace);
		new FunctionWithLookup(pSymbolNameSpace); // but WITH_LOOKUP is treated specially by parser
		new FunctionStep(pSymbolNameSpace);
		new FunctionTabbedArray(pSymbolNameSpace);
		new FunctionRamp(pSymbolNameSpace);
		new FunctionLn(pSymbolNameSpace);
		new FunctionSmooth(pSymbolNameSpace);
		new FunctionSmoothI(pSymbolNameSpace);
		new FunctionSmooth3(pSymbolNameSpace);
		new FunctionTrend(pSymbolNameSpace);
		new FunctionDelay1(pSymbolNameSpace);
		new FunctionDelay3(pSymbolNameSpace);
		new FunctionDelay(pSymbolNameSpace);
		new FunctionDelayN(pSymbolNameSpace);
		new FunctionSmoothN(pSymbolNameSpace);
		new FunctionDelayConveyor(pSymbolNameSpace);
		new FunctionVectorReorder(pSymbolNameSpace);
		new FunctionRandomNormal(pSymbolNameSpace);
		new FunctionRandomPoisson(pSymbolNameSpace);
		new FunctionLookupArea(pSymbolNameSpace);
		new FunctionLookupExtrapolate(pSymbolNameSpace);
		new FunctionGetDataAtTime(pSymbolNameSpace);
		new FunctionGetDataLastTime(pSymbolNameSpace);
		new FunctionModulo(pSymbolNameSpace);
		new FunctionNPV(pSymbolNameSpace);
		new FunctionSum(pSymbolNameSpace);
		new FunctionProd(pSymbolNameSpace);
		new FunctionTimeBase(pSymbolNameSpace);
		new FunctionVectorSelect(pSymbolNameSpace);
		new FunctionVectorElmMap(pSymbolNameSpace);
		new FunctionVectorSortOrder(pSymbolNameSpace);
		new FunctionGame(pSymbolNameSpace);
		new FunctionRandom01(pSymbolNameSpace);
		new FunctionRandomUniform(pSymbolNameSpace);
		new FunctionAbs(pSymbolNameSpace);
        new FunctionExp(pSymbolNameSpace);
        new FunctionSqrt(pSymbolNameSpace);
        
        new FunctionCosine(pSymbolNameSpace);
        new FunctionSine(pSymbolNameSpace);
        new FunctionTangent(pSymbolNameSpace);
        new FunctionArcCosine(pSymbolNameSpace);
        new FunctionArcSine(pSymbolNameSpace);
        new FunctionArcTangent(pSymbolNameSpace);
		new FunctionInterger(pSymbolNameSpace);

		new FunctionGetDirectData(pSymbolNameSpace);
		new FunctionGetDataMean(pSymbolNameSpace);

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
			 if (ex->GetType() == EXPTYPE_Operator && ex->GetArg(0) == NULL && *ex->GetOperator() == '-')
			 {
				 // alloe unary minus here
				 ent->AddValue(0, -ex->GetArg(1)->Eval(NULL)); // note eval does not need context for number
			 }
             else if(ex->GetType() != EXPTYPE_Number) {
                mSyntaxError.str = "Expecting only comma delimited numbers " ;
                throw mSyntaxError ;
             }
			 else 
	             ent->AddValue(0,ex->Eval(NULL)) ; // note eval does not need context for number
             delete ex ;
          }
          delete exl ;
          ex = ent ;
       }
    }

    return new Equation(pSymbolNameSpace,lhs,ex,tok) ; 
}
Equation *VensimParse::AddTable(LeftHandSide *lhs, Expression *ex, ExpressionTable* tbl, bool legacy)
{
	if (!tbl)
	{
		// this is an exogenous data entry - treat as a table on time and give the table value 1 at all times if we have that info
		tbl = new ExpressionTable(this->pSymbolNameSpace);
		tbl->AddPair(0, 1);
		tbl->AddPair(1, 1);
		Variable* var = this->FindVariable("TIME");
		if(!var)
			var = new Variable(this->pSymbolNameSpace, "TIME");
		ex = new ExpressionVariable(this->pSymbolNameSpace, var, NULL);
	}
	if (legacy)
		tbl->TransformLegacy();
	if (!ex)
		return new Equation(pSymbolNameSpace, lhs, tbl, '(');
	// with lookup is just a table embedded in a variable - actually the norm for XMILE
	//Function* wl = static_cast<Function*>(pSymbolNameSpace->Find("WITH LOOKUP"));
	Expression* rhs =  new ExpressionLookup(pSymbolNameSpace, ex, tbl);
	return new Equation(pSymbolNameSpace, lhs, rhs, '=');

}

/* a full eq cleans up the temporary memory space and 
  also assigns the equation to the Variable  - not that inside
  of macros there is a separate symbol space this is going
  against */
void VensimParse::AddFullEq(Equation *eq,UnitExpression *un)
{
   pSymbolNameSpace->ConfirmAllAllocations() ; // now independently allocated
   pActiveVar = eq->GetVariable() ;
   if (!_model->Groups().empty() && pActiveVar->GetAllEquations().empty() && !mInMacro)
	   _model->Groups().back().vVariables.push_back(pActiveVar);
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

static std::string compress_whitespace(const std::string& s)
{
	std::string rval;
	const char *tv = s.c_str();
	for (; *tv; tv++)
	{
		if (*tv != ' ' && *tv != '\t' && *tv != '\n' && *tv != '\r')
			break;
	}
	for (; *tv; tv++)
	{
		if (*tv == '~')
			break; // some comments have supplementary in them
		if (*tv == ' ' || *tv == '\t' || *tv == '\n' || *tv == '\r')
		{
			rval.push_back('_');
			for (; tv[1]; tv++)
			{
				if (tv[1] != ' ' && tv[1] != '\t' && tv[1] != '\n' && tv[1] != '\r')
					break;
			}
		}
		else if( (*tv >= 'A' && *tv <= 'Z') || (*tv >= 'a' && *tv <= 'z'))
			rval.push_back(*tv); // otherwise ignore
	}
	while (rval.back() == '_')
		rval.pop_back();
	return rval;
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
                if(!FindNextEq(true))
                   break ;
             } 
             else if(rval == '|') {
             }
             else if(rval == VPTT_groupstar) {
				//printf("%s\n", mVensimLex.CurToken()->c_str());
				// only change this if a new number
				 std::string group_owner;
				 char c = mVensimLex.CurToken()->at(0);
				 if (_model->Groups().empty() ||
					 (_model->Groups().back().sName[0] != c && c >= '0' && c <= '9'))
					 group_owner = *mVensimLex.CurToken();
				 else
					 group_owner = _model->Groups().back().sOwner;
				 {
					 _model->Groups().push_back(ModelGroup(*mVensimLex.CurToken(),group_owner));
				 }
             }
             else if(rval != endtok) {
                std::cout << "Unknown terminal token " << rval << std::endl ;
                if(!FindNextEq(false))
                   break ;
             }

          }
          catch(VensimParseSyntaxError& e) {
             std::cout << e.str << std::endl ;
             std::cout << "Error at line " << mVensimLex.LineNumber() << " position " << mVensimLex.Position() 
                << " in file " << sFilename << std::endl ;
			 std::cout << ".... skipping the associated variable and looking for the next usable content.";
             pSymbolNameSpace->DeleteAllUnconfirmedAllocations() ;
             noerr = false ;
             if(!FindNextEq(false)) 
                break ;

          }
          catch(...) {
             pSymbolNameSpace->DeleteAllUnconfirmedAllocations() ;
             noerr = false ;
             if(!FindNextEq(false))
                break ;

          }
       } while(rval != endtok) ;
	   char buf[BUFLEN]; // plenty big for sketch info
	   if (rval == endtok)
		this->mVensimLex.BufferReadLine(buf, BUFLEN); // get the marker line
	   while (true)
	   { // read in the sketch information
		   if (strncmp(buf, "\\\\\\---///", 9) != 0)
			   break;
		   this->mVensimLex.ReadLine(buf, BUFLEN); // version line
		   if (strncmp(buf, "V300 ", 5))
		   {
			   printf("Unrecognized version - can't read sketch info\n");
			   noerr = false;
			   break;
		   }
		   VensimView* view = new VensimView;
		   VensimViewElements& elements = view->Elements(); // we populate this directly
		   
		   _model->AddView(view);
		   // next the title
		   this->mVensimLex.ReadLine(buf, BUFLEN); 
		   view->SetTitle(buf + 1); // skip the star - we can try to name modules with this eventually subject to name collisions
		   this->mVensimLex.ReadLine(buf, BUFLEN); // default font info - we can try to grab this later
		   view->ReadView(this, buf); // will return with buf populated at next view
	   }
	   // there may be options at the end
	   if (strncmp(buf, "///---\\\\\\", 9) == 0)
	   {
		   while (this->mVensimLex.ReadLine(buf, BUFLEN)) // looking for settings maker
		   {
			   if (strncmp(buf, ":L\177<%^E!@", 9) == 0)
			   {
				   while (this->mVensimLex.ReadLine(buf, BUFLEN))
				   {
					   int type;
					   char *curpos = GetIntChar(buf, type, ':');
					   if (type == 15) // fourth entry is integration type
					   {
						   int im;
						   for (int i = 0; i < 4; i++)
							   curpos = GetInt(curpos, im);
						   Integration_Type it = Integration_Type_EULER;
						   switch (im)
						   {
						   case 0:
						   case 2:
						   default:
							   it = Integration_Type_EULER;
							   break;
						   case 1:
						   case 5:
							   it = Integration_Type_RK4;
							   break;
						   case 3:
						   case 4:
							   it = Integration_Type_RK2;
							   break;
						   }
						   _model->SetIntegrationType(it);
					   }
					   else if (type == 22) // units equialences
					   {
						   _model->UnitEquivs().push_back(curpos);
					   }

				   }
				   break;
			   }
		   }
	   }
       mfSource.close() ;
	   _model->SetMacroFunctions(mMacroFunctions);

	   if (bLongName)
	   {
		   // try to replace variable names with long names from the documentaion
		   std::vector<Variable*> vars = _model->GetVariables(NULL); // all symbols that are variables
		   BOOST_FOREACH(Variable* var, vars)
		   {
			   std::string alt = compress_whitespace(var->Comment());
			   if (alt == "Backlog")
			   {
				   bLongName = true;
			   }
			   if (!alt.empty() && alt.size() < 80 &&
				   pSymbolNameSpace->Rename(var,alt))
			   {
				   var->SetAlternateName(alt);
			   }
		   }
	   }
       return true ; // got something - try to put something out
    }
    else
       return false ;
}

char *VensimParse::GetIntChar(char *s, int& val, char c)
{
	char* tv;
	for (tv = s; *tv; tv++)
	{
		if (*tv == c)
		{
			*tv++ = NULL;
			break;
		}
	}
	val = atoi(s);
	return tv;
}
char *VensimParse::GetInt(char *s, int& val)
{
	char* tv;
	for (tv = s; *tv; tv++)
	{
		if (*tv == ',')
		{
			*tv++ = NULL;
			break;
		}
	}
	val = atoi(s);
	return tv;
}
char *VensimParse::GetString(char *s, std::string& name)
{
	char* tv = s;
	if (*tv == '\"')
	{
		for (tv++; *tv; tv++)
		{
			if (*tv == '\"')
			{
				tv++;
				assert(*tv == ','); 
				*tv++ = NULL;
				break;
			}
			else if (*tv == '\\' && tv[1] == '\"')
				tv++;
		}
	}
	else
	{
		for (tv = s; *tv; tv++)
		{
			if (*tv == ',')
			{
				*tv++ = NULL;
				break;
			}
		}
	}
	name = s;
	return tv;
}

Variable *VensimParse::FindVariable(const std::string &name)
{
	Variable *var = static_cast<Variable *>(pSymbolNameSpace->Find(name));
	if (var && var->isType() == Symtype_Variable)
		return var;
	return NULL;
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
	std::string uname = ">" + name; // an illegal variable name since we allow the same names to be used for vars and units - could use a separate namespace
   Units *u =  static_cast<Units *>(pSymbolNameSpace->Find(uname)) ;
   if(u && u->isType() != Symtype_Units) {
      mSyntaxError.str = "Type meaning mismatch for " + name ;
      throw mSyntaxError ;
   }
   if(!u) {
      u = new Units(pSymbolNameSpace,uname) ;
   }
   return u ;
}

UnitExpression *VensimParse::InsertUnitExpression(Units *u) 
{
   UnitExpression *uni = new UnitExpression(pSymbolNameSpace,u) ;
   return uni ;
}

// find the beginning of the next equation - for error recovery
bool VensimParse::FindNextEq(bool want_comment)
{
	if (want_comment && this->pActiveVar)
	{
		std::string comment = mVensimLex.GetComment("|");
		if (! comment.empty()) // multile appearances okay - take last non empty
			this->pActiveVar->SetComment(comment);
	}
   // just zip through to the first | then whatever follows is it
   return mVensimLex.FindToken("|") ;
}


LeftHandSide *VensimParse::AddExceptInterp(ExpressionVariable *var,SymbolListList *except,int interpmode) 
{ 
   return new LeftHandSide(pSymbolNameSpace,var,NULL, except,interpmode) ;
}
SymbolList *VensimParse::SymList(SymbolList *in,Variable *add,bool bang,Variable *end) 
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

SymbolList *VensimParse::MapSymList(SymbolList* in, Variable* range, SymbolList *list)
{
	list->SetMapRange(range);
	if (in) {
		in->Append(list);
		return in;
	}
	return list;
}
UnitExpression *VensimParse::UnitsDiv(UnitExpression *num,UnitExpression *denom) 
{
   return num->Divide(denom) ;
}
UnitExpression *VensimParse::UnitsMult(UnitExpression *f,UnitExpression *s) 
{ 
   return f->Multiply(s) ;
}
UnitExpression *VensimParse::UnitsRange(UnitExpression *e,double minval,double maxval,double increment) 
{ 
	if (e == NULL)	{
		e = VPObject->InsertUnitExpression(VPObject->InsertUnits("1"));
	}
   e->SetRange(minval,maxval,increment) ;
   return e ;
}

SymbolListList *VensimParse::ChainSublist(SymbolListList *sll,SymbolList *nsl) 
{ 
	if (!sll)
		sll = new SymbolListList(pSymbolNameSpace, nsl);
	else
		sll->Append(nsl);
	return sll;
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
Expression *VensimParse::LiteralExpression(const char* lit)
{
	return new ExpressionLiteral(pSymbolNameSpace, lit);
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
         return new ExpressionUnaryMinus(pSymbolNameSpace,exp1,NULL) ; 
      }
      return new ExpressionSubtract(pSymbolNameSpace,exp1,exp2) ;
   case '^' :
      return new ExpressionPower(pSymbolNameSpace,exp1,exp2) ;
   case '(' :
      assert(!exp2) ;
      return new ExpressionParen(pSymbolNameSpace,exp1,NULL) ;
   case '<':
   case '>':
   case VPTT_le:
   case VPTT_ge:
   case VPTT_and:
   case VPTT_or:
   case '=':
	   return new ExpressionLogical(pSymbolNameSpace, exp1, exp2, oper);
   case VPTT_not:
	   assert(exp2 == NULL);
	   return new ExpressionLogical(pSymbolNameSpace, NULL, exp1, oper);
   default :
     mSyntaxError.str = "Unknown operator internal error "  ;
     throw mSyntaxError ;
   }
}
Expression *VensimParse::FunctionExpression(Function *func,ExpressionList *eargs) 
{  
   if((!eargs && func->NumberArgs() > 0)  
	   || (eargs && func->NumberArgs() != eargs->Length())) {
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

ExpressionTable *VensimParse::XYTableVec(ExpressionTable *table, double val)
{
	if (!table)
		table = new ExpressionTable(pSymbolNameSpace);
	table->AddPair(val, 0); // fix these after reducing
	return table;
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
