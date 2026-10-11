#ifndef _XMUTIL_SYMBOL_EQUATION_H
#define _XMUTIL_SYMBOL_EQUATION_H
#include "../ContextInfo.h"
#include "SymbolTableBase.h"

class Model;
class ExpressionTable;

class Equation : public SymbolTableBase {
public:
  Equation(SymbolNameSpace *sns, LeftHandSide *lhs, Expression *ex, int token /* = ( dataequals not standard */);
  ~Equation(void);
  LeftHandSide *GetLeft(void) {
    return pLeftHandSide;
  }
  Variable *GetVariable(void);  // LeftHandSide forward declared can't put function here
  void GetVarsUsed(std::vector<Variable *> &vars);
  inline Expression *GetExpression(void) {
    return pExpression;
  }
  ExpressionTable *GetTable(void);
  int SubscriptCount(std::vector<Variable *> &elmlist);
  static void GetSubscriptElements(std::vector<Symbol *> &vals, Symbol *s);  // if nested defs
  bool SubscriptExpand(std::vector<std::vector<Symbol *>> &elms,
                       std::vector<Symbol *> &subs);  // can be one or many depending on the subs
  void Execute(ContextInfo *info);
  void OutputComputable(ContextInfo *info);
  void CheckPlaceholderVars(Model *m);
  // droppedInputs / referenced, when given, receive the variables a translation
  // read as literals and the variables the written equation refers to (see
  // ContextInfo::NoteDroppedInput); an input the variable no longer depends on
  // is dropped in one of its equations and referenced in none.
  std::string RHSFormattedXMILE(Variable *lhs, const std::vector<Symbol *> &subs, const std::vector<Symbol *> &dims,
                                bool init, std::set<Variable *> *droppedInputs = nullptr,
                                std::set<Variable *> *referenced = nullptr);  // need a_b*c
  bool IsActiveInit();                                                        // true only for active
private:
  LeftHandSide *pLeftHandSide;
  Expression *pExpression;
  int iEqType;
};

#endif