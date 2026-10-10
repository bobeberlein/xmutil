#include "Function.h"

#include "../ContextInfo.h"
#include "../Log.h"
#include "../Symbol/ExpressionList.h"
#include "../XMUtil.h"

// model symbol - most variables including subscript ranges but not
// subscript elements which are just
Function::Function(SymbolNameSpace *sns, const std::string &name, int nargs) : Symbol(sns, name) {
  assert(nargs < 32);  // the way initial argument identification is implemented requires this
  iNumberArgs = nargs;
  iMinArgs = nargs;
  iMaxArgs = nargs;
}

Function::~Function(void) {
}

MacroFunction::MacroFunction(SymbolNameSpace *sns, SymbolNameSpace *local, const std::string &name,
                             ExpressionList *margs)
    : Function(sns, name, margs->Length()), pSymbolNameSpace(local), mArgs(margs) {
}

std::string MacroFunction::ComputableName(void) {
  return SpaceToUnderBar(this->GetName());
}

bool Function::CheckComputedList(ContextInfo *info, ExpressionList *arg) {
  return arg->CheckComputed(info, 0xffffffff);
}
void Function::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  *info << ComputableName();
  if (arg) {
    *info << "(";
    arg->OutputComputable(info, 0xffffffff);
    *info << ")";
  }
}

void UnknownFunction::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  *info << "{Untranslated function used}";
  *info << sName;
  Function::OutputComputable(info, arg);
}

void FunctionVectorLookup::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  // could try to figure this one out -
  *info << "{try INTERPORATE or just used arrays with variable arguments for indices}";
  Function::OutputComputable(info, arg);
}

void FunctionVectorSelect::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  // VECTOR SELECT(sel[..d!..], expr[..d!..], missing, action, error) reduces,
  // over the bang (!) dimensions, the elements whose selection is nonzero, and
  // gives `missing` when there are none. XMILE spells the same thing with its
  // array builtins over the `*` the bang subscripts are written as:
  //
  //   ( IF SUM(IF sel <> 0 THEN 1 ELSE 0) = 0 THEN missing ELSE reduction )
  //
  // where the weighted actions (0-4) reduce sel * expr and the unweighted ones
  // (6-10) expr, each over the selected elements only -- an unselected element
  // contributes the reduction's identity (0 to a sum, 1 to a product, +/-1e38
  // to a min/max). The error action only decides when Vensim raises a run-time
  // error, which XMILE cannot do, so it is dropped. Action 5 (a weighted
  // power product) and an action that is not a constant are passed through
  // unchanged, with a warning: there is nothing XMILE to write.
  //
  // The two arrays are paired only through the dimensions they iterate, so
  // their bang subscripts are written `Dim.*`, naming them, rather than the
  // bare `*` used elsewhere (for the whole call, restored on the way out).
  struct BangGuard {
    ContextInfo *info;
    bool was;
    ~BangGuard() {
      info->SetBangAsDimStar(was);
    }
  } bangGuard{info, info->BangAsDimStar()};
  info->SetBangAsDimStar(true);
  if (!arg || arg->Length() != 5 || arg->GetExp(3)->GetType() != EXPTYPE_Number) {
    log("warning: VECTOR SELECT with a non-constant numerical action has no XMILE equivalent; written as is\n");
    Function::OutputComputable(info, arg);
    return;
  }
  const double actionValue = static_cast<ExpressionNumber *>(arg->GetExp(3))->GetValue();
  const int action = static_cast<int>(actionValue);
  if (action != actionValue || action < 0 || action > 10 || action == 5) {
    log("warning: VECTOR SELECT numerical action %g has no XMILE equivalent; written as is\n", actionValue);
    Function::OutputComputable(info, arg);
    return;
  }
  Expression *sel = arg->GetExp(0);
  Expression *expr = arg->GetExp(1);
  // An operand is parenthesized unless it is a lone variable or number, which
  // needs none.
  auto operand = [&](Expression *e) {
    const bool atom = e->GetType() == EXPTYPE_Variable || e->GetType() == EXPTYPE_Number;
    if (!atom)
      *info << "(";
    e->OutputComputable(info);
    if (!atom)
      *info << ")";
  };
  auto selected = [&]() {
    operand(sel);
    *info << " <> 0";
  };
  auto value = [&]() {
    if (action <= 4) {
      operand(sel);
      *info << "*";
    }
    operand(expr);
  };
  auto count = [&]() {
    *info << "SUM(IF ";
    selected();
    *info << " THEN 1 ELSE 0)";
  };
  // The reduction over the selected elements, with the identity for the rest.
  auto reduce = [&](const char *fn, const char *identity) {
    *info << fn << "(IF ";
    selected();
    *info << " THEN ";
    value();
    *info << " ELSE " << identity << ")";
  };

  *info << "( IF ";
  count();
  *info << " = 0 THEN ";
  arg->GetExp(2)->OutputComputable(info);
  *info << " ELSE ";
  switch (action) {
  case 0:
  case 6:
    reduce("SUM", "0");
    break;
  case 1:
  case 7:
    reduce("PROD", "1");
    break;
  case 2:
  case 8:
    reduce("MIN", "1e+38");
    break;
  case 3:
  case 9:
    reduce("MAX", "-1e+38");
    break;
  case 4:
  case 10:
    reduce("SUM", "0");
    *info << "/";
    count();
    break;
  }
  *info << " )";
}

void FunctionElmCount::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg && arg->Length() == 1 && arg->GetExp(0)->GetType() == EXPTYPE_Variable) {
    *info << "SIZE(";
    Variable *var = static_cast<ExpressionVariable *>(arg->GetExp(0))->GetVariable();
    *info << var->GetName();
    *info << ")";
    return;
  }
  Function::OutputComputable(info, arg);
}

void FunctionTimeBase::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 2) {
    Expression *exp1 = arg->GetExp(0);
    exp1->OutputComputable(info);
    *info << " + (";
    Expression *exp2 = arg->GetExp(1);
    exp2->OutputComputable(info);
    *info << ") * Time";
  } else {
    Function::OutputComputable(info, arg);
  }
}

bool FunctionMemoryBase::CheckComputedList(ContextInfo *info, ExpressionList *arg) {
  if (info->GetComputeType() == CF_initial)
    return arg->CheckComputed(info, iInitArgMark);
  else
    return arg->CheckComputed(info, iActiveArgMark);
}
void FunctionMemoryBase::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (info->GetComputeType() == CF_xmile_output) {
    const std::string &fname = this->GetName();
    if (fname == "INITIAL") {
      *info << "INIT(";
      arg->OutputComputable(info, iInitArgMark);
      *info << ")";
    } else if (this->IsIntegrator() || info->InitEqn())
      arg->OutputComputable(info, iInitArgMark);
    else
      arg->OutputComputable(info, iActiveArgMark);
    return;
  }

  if (info->GetComputeType() == CF_initial) {
    *info << ComputableNameInit() << "(";
    arg->OutputComputable(info, iInitArgMark);
  } else {
    *info << ComputableName() << "(";
    arg->OutputComputable(info, iActiveArgMark);
  }
  *info << ")";
}
/* a simple utility function to flip the bits so we can use BOOST_BINARY
  in the intuitive right to left manner for initial value arguments */
unsigned FunctionMemoryBase::BitFlip(unsigned bits) {
  unsigned newbits = 0;
  int i;
  for (i = 0; i < iNumberArgs; i++) {
    if (bits & (1 << i))
      newbits |= (1 << (iNumberArgs - i - 1));
  }
  return newbits;
}

void FunctionSampleIfTrue::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 3) {
    *info << "( IF ";
    info->SetSelfIsPrevious(true);
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);  // OutputComputable should really be const
    info->SetSelfIsPrevious(false);
    *info << " THEN ";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);  // OutputComputable should really be const
    *info << " ELSE PREVIOUS(SELF, ";
    const_cast<Expression *>((*arg)[2])->OutputComputable(info);  // OutputComputable should really be const
    *info << ") )";
    return;
  }
  Function::OutputComputable(info, arg);
}

void FunctionPulse::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 2) {
    *info << "( IF TIME >= (";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);  // OutputComputable should really be const
    *info << ") AND TIME < ((";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);  // OutputComputable should really be const
    *info << ") + MAX(DT,";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);  // OutputComputable should really be const
    *info << ")) THEN 1 ELSE 0 )";
    return;
  }
  Function::OutputComputable(info, arg);
}

void FunctionPulseTrain::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 4) {
    *info << "( IF TIME >= (";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);  // OutputComputable should really be const
    *info << ") AND TIME <= (";
    const_cast<Expression *>((*arg)[3])->OutputComputable(info);  // OutputComputable should really be const
    *info << ") AND (TIME - (";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);  // OutputComputable should really be const
    *info << ")) MOD (";
    const_cast<Expression *>((*arg)[2])->OutputComputable(info);  // OutputComputable should really be const
    *info << ") < (";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);  // OutputComputable should really be const
    *info << ") THEN 1 ELSE 0 )";
    return;
  }
  Function::OutputComputable(info, arg);
}

void FunctionQuantum::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 2) {
    *info << "(";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);  // OutputComputable should really be const
    *info << ")*INT((";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);  // OutputComputable should really be const
    *info << ")/(";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);  // OutputComputable should really be const
    *info << "))";
    return;
  }
  Function::OutputComputable(info, arg);
}

void FunctionRandomBinomial::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 7) {
    *info << "((";
    const_cast<Expression *>((*arg)[4])->OutputComputable(info);
    *info << ")+(";
    const_cast<Expression *>((*arg)[5])->OutputComputable(info);
    *info << ")*";
    *info << "BINOMIAL(";
    const_cast<Expression *>((*arg)[3])->OutputComputable(info);  // OutputComputable should really be const
    *info << ", ";
    const_cast<Expression *>((*arg)[2])->OutputComputable(info);  // OutputComputable should really be const
    *info << ", ";
    const_cast<Expression *>((*arg)[6])->OutputComputable(info);  // OutputComputable should really be const
    *info << ", ";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);  // OutputComputable should really be const
    *info << ", ";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);  // OutputComputable should really be const
    *info << "))";
    return;
  }
  Function::OutputComputable(info, arg);
}

void FunctionRandomNormal::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 5) {
    *info << "NORMAL(";
    const_cast<Expression *>((*arg)[2])->OutputComputable(info);  // OutputComputable should really be const
    *info << ", ";
    const_cast<Expression *>((*arg)[3])->OutputComputable(info);  // OutputComputable should really be const
    *info << ", ";
    const_cast<Expression *>((*arg)[4])->OutputComputable(info);  // OutputComputable should really be const
    *info << ", ";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);  // OutputComputable should really be const
    *info << ", ";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);  // OutputComputable should really be const
    *info << ")";
    return;
  }
  Function::OutputComputable(info, arg);
}

void FunctionRandomPoisson::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 6) {
    *info << "POISSON((";
    const_cast<Expression *>((*arg)[2])->OutputComputable(info);  // OutputComputable should really be const
    *info << ")/DT, ";
    const_cast<Expression *>((*arg)[5])->OutputComputable(info);  // OutputComputable should really be const
    *info << ", ";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);  // OutputComputable should really be const
    *info << ", ";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);  // OutputComputable should really be const
    *info << ") * ";
    const_cast<Expression *>((*arg)[4])->OutputComputable(info);  // OutputComputable should really be const
    *info << " + ";
    const_cast<Expression *>((*arg)[3])->OutputComputable(info);  // OutputComputable should really be const
    return;
  }
  Function::OutputComputable(info, arg);
}

void FunctionIfThenElse::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 3) {
    *info << "( IF ";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);  // OutputComputable should really be const
    *info << " THEN ";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);  // OutputComputable should really be const
    *info << " ELSE ";
    const_cast<Expression *>((*arg)[2])->OutputComputable(info);  // OutputComputable should really be const
    *info << " )";
    return;
  }
  Function::OutputComputable(info, arg);
}

void FunctionDelayN::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 4) {
    *info << "DELAYN(";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);
    *info << ",";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);
    *info << ",";
    const_cast<Expression *>((*arg)[3])->OutputComputable(info);
    *info << ",";
    const_cast<Expression *>((*arg)[2])->OutputComputable(info);
    *info << ")";
  }
}
void FunctionSmoothN::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 4) {
    *info << "SMTHN(";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);
    *info << ",";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);
    *info << ",";
    const_cast<Expression *>((*arg)[3])->OutputComputable(info);
    *info << ",";
    const_cast<Expression *>((*arg)[2])->OutputComputable(info);
    *info << ")";
  }
}

void FunctionLog::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 2) {
    *info << "(LN(";
    const_cast<Expression *>((*arg)[0])->OutputComputable(info);  // OutputComputable should really be const
    *info << ") / LN(";
    const_cast<Expression *>((*arg)[1])->OutputComputable(info);  // OutputComputable should really be const
    *info << "))";
    return;
  }
  Function::OutputComputable(info, arg);
}

void FunctionAllocateByPriority::OutputComputable(ContextInfo *info, ExpressionList *arg) {
  if (arg->Length() == 5) {
    *info << "ALLOCATE(";
    Expression *supply = arg->GetExp(4);
    supply->OutputComputable(info);
    *info << ", ";
    Expression *demand = arg->GetExp(0);
    if (demand->GetType() == EXPTYPE_Variable) {
      ExpressionVariable *expvar = static_cast<ExpressionVariable *>(demand);
      SymbolList *subs = expvar->GetSubs();
      if (subs) {
        int len = subs->Length();
        if (len > 0) {
          const SymbolList::SymbolListEntry &blah = (*subs)[len - 1];
          *info << blah.u.pSymbol->GetName();
        }
      }
    }
    *info << ", ";
    info->SetWantFinalStar(true);
    demand->OutputComputable(info);
    *info << ", ";
    Expression *priority = arg->GetExp(1);
    priority->OutputComputable(info);
    *info << ", ";
    info->SetWantFinalStar(false);
    Expression *width = arg->GetExp(3);
    width->OutputComputable(info);
    *info << ")";
  } else {
    Function::OutputComputable(info, arg);
  }
}

#ifdef WANT_EVAL_STUFF
double FunctionMax::Eval(Expression *from, ExpressionList *arg, ContextInfo *info) {
  double a1 = arg->GetExp(0)->Eval(info);
  double a2 = arg->GetExp(1)->Eval(info);
  return a1 > a2 ? a1 : a2;
}
double FunctionMin::Eval(Expression *from, ExpressionList *arg, ContextInfo *info) {
  double a1 = arg->GetExp(0)->Eval(info);
  double a2 = arg->GetExp(1)->Eval(info);
  return a1 < a2 ? a1 : a2;
}
double FunctionInteg::Eval(Expression *from, ExpressionList *arg, ContextInfo *info) {
  if (info->GetComputeType() == CF_initial)
    return arg->GetExp(1)->Eval(info);  // initialization
  return arg->GetExp(0)->Eval(info);
  return 0;
}
double FunctionPulse::Eval(Expression *from, ExpressionList *arg, ContextInfo *info) {
  double s = arg->GetExp(0)->Eval(info);
  double dt = info->GetDT();
  double w = arg->GetExp(1)->Eval(info);
  if (w < dt)
    w = dt;
  double t = info->GetTime();
  if (t > s - dt / 4.0 && t < s + w - dt / 4.0)
    return 1.0;
  return 0;
}
#endif
