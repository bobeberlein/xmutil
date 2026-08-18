#include "Expression.h"

#include <algorithm>

#include "../Model.h"
#include "../Symbol/Parse.h"
#include "../XMUtil.h"
#include "Equation.h"
#include "ExpressionList.h"
#include "LeftHandSide.h"
#define YYSTYPE ParseUnion
#include "../Dynamo/DynamoFunction.h"
#include "../Vensim/VYacc.tab.hpp"

Expression::Expression(SymbolNameSpace *sns) : SymbolTableBase(sns) {
}

Expression::~Expression(void) {
}

double Expression::Eval(ContextInfo *info) {
  return FLT_MAX;
}

void Expression::OutputComputable(ContextInfo *info) {
}

Expression *ExpressionFunction::Clone(SymbolNameSpace *sns) {
  // pFunction is shared (registered in the namespace, not owned here); the
  // argument list is owned, so it is deep-copied.
  return new ExpressionFunction(sns, pFunction, pArgs ? pArgs->Clone(sns) : nullptr);
}

Expression *ExpressionFunctionMemory::Clone(SymbolNameSpace *sns) {
  // pPlacholderEquation is wired up later in the post-parse pipeline; at clone
  // time (during parsing) it is always null, so the copy needs only the shared
  // function and a cloned argument list. GetFunction()/GetArgs() reach the
  // base-class members, which are private to ExpressionFunction.
  return new ExpressionFunctionMemory(sns, GetFunction(), GetArgs() ? GetArgs()->Clone(sns) : nullptr);
}

Expression *ExpressionLookup::Clone(SymbolNameSpace *sns) {
  if (pExpressionVariable) {
    ExpressionVariable *var = static_cast<ExpressionVariable *>(pExpressionVariable->Clone(sns));
    return new ExpressionLookup(sns, var, pExpression ? pExpression->Clone(sns) : nullptr);
  }
  // WITH LOOKUP form (embedded table): the XMILE reader never emits this as a
  // scalar sub-expression, so a faithful clone is unnecessary -- signal
  // "not cloneable" per the Expression::Clone contract.
  return nullptr;
}

void ExpressionNumber::OutputComputable(ContextInfo *info) {
  // Emit the shortest decimal that reparses to the exact same double, via the
  // same ShortestDouble that backs mdl::FormatMDLNumber and the XMILE writer's
  // lookup-sample formatting. The ostream default (6 significant digits) would
  // drop precision on constants like pi(), so an XMILE->XMILE round trip would
  // drift past the comparator's tolerance.
  *info << ShortestDouble(value);
}

ExpressionFunction::~ExpressionFunction() {
  if (HasGoodAlloc())
    delete pArgs;
}

void ExpressionFunction::CheckPlaceholderVars(Model *m, bool isfirst) {
  pArgs->CheckPlaceholderVars(m);
}

void ExpressionFunction::CheckTableUses(Variable *var) {
  if (!pArgs) {
    return;
  }
  // dynamo only this same place in the call logic lets us fill in the table function info
  if (pFunction && pFunction->IsTableCall()) {
    std::string name = pFunction->GetName();
    if (!static_cast<const DFunctionTable *>(pFunction)->SetTableXAxis(pArgs))
      log("ERROR TABLE call in equation for %s not correctly formmatted.\n", var->GetName().c_str());

    if (name == "TABXL") {
      // if we get a LOOKUP_EXTRAPOLATE then try to mark the associated lookup - assume all will extrapolate
      std::vector<Variable *> vars;
      const_cast<Expression *>((*pArgs)[0])->GetVarsUsed(vars);
      // the first should be a graphical
      std::vector<Equation *> eqs = vars[0]->GetAllEquations();
      for (Equation *eq : eqs) {
        Expression *exp = eq->GetExpression();
        if (exp->GetType() == EXPTYPE_Table)
          static_cast<ExpressionTable *>(exp)->SetExtrapolate(true);
      }
    }
  }

  int n = pArgs->Length();
  for (int i = 0; i < n; i++) {
    pArgs->GetExp(i)->CheckTableUses(var);
  }
}

void ExpressionLookup::CheckTableUses(Variable *var) {
  // Run for the input sub-expression (it may itself contain lookup/table calls).
  if (pExpression)
    pExpression->CheckTableUses(var);
  // A TABXL(table, x) call (parsed into this lookup with bExtrapolate set) marks
  // the referenced graphical function as extrapolating. This runs in the
  // post-parse MarkVariableTypes pass, so it is independent of whether the <gf>
  // was declared before or after the call site. Mirrors the Dynamo TABXL branch
  // in ExpressionFunction::CheckTableUses.
  if (bExtrapolate && pExpressionVariable) {
    Variable *lookupVar = pExpressionVariable->GetVariable();
    if (lookupVar) {
      for (Equation *eq : lookupVar->GetAllEquations()) {
        Expression *exp = eq->GetExpression();
        if (exp && exp->GetType() == EXPTYPE_Table)
          static_cast<ExpressionTable *>(exp)->SetExtrapolate(true);
      }
    }
  }
}

void ExpressionFunction::GetVarsUsed(std::vector<Variable *> &vars) {
  if (!pArgs) {
    return;
  }

  int n = pArgs->Length();
  for (int i = 0; i < n; i++) {
    pArgs->GetExp(i)->GetVarsUsed(vars);
  }
}

void ExpressionFunctionMemory::CheckPlaceholderVars(Model *m, bool isfirst) {
  if (isfirst || !m) {
    pPlacholderEquation = NULL;  // deletion is handled by Model
  } else {
    pPlacholderEquation = m->AddUnnamedVariable(this);
  }
}

static void is_all_plus_minus(Expression *e, FlowList *fl, bool neg) {
  if (!e)
    fl->SetValid(false);
  else if (e->GetType() == EXPTYPE_Variable) {
    ExpressionVariable *ev = static_cast<ExpressionVariable *>(e);
    Variable *var = ev->GetVariable();
    if (neg) {
      if (var->HasUpstream())
        fl->SetValid(false);
      else {
        if (var->VariableType() == XMILE_Type_STOCK)
          fl->SetValid(false);
        fl->AddOutflow(var);
        var->SetHasUpstream(true);
      }
    } else {
      if (var->HasDownstream())
        fl->SetValid(false);
      else {
        if (var->VariableType() == XMILE_Type_STOCK)
          fl->SetValid(false);
        fl->AddInflow(var);
        var->SetHasDownstream(true);
      }
    }
  } else if (e->GetType() == EXPTYPE_Operator) {
    const char *op = e->GetOperator();
    if (op) {
      if (*op == '\0')  // could be () or unary +/- if - we need to flip neg
      {
        const char *before = e->GetBefore();
        if (before && *before == '-')
          neg = !neg;
        is_all_plus_minus(e->GetArg(0), fl, neg);
      } else if ((*op == '-' || *op == '+') && op[1] == '\0') {
        if (e->GetArg(0) != NULL)  // unary plus or leading - still okay
          is_all_plus_minus(e->GetArg(0), fl, neg);
        if (*op == '-')
          neg = !neg;
        is_all_plus_minus(e->GetArg(1), fl, neg);
      } else
        fl->SetValid(false);
    } else
      fl->SetValid(false);
  } else
    fl->SetValid(false);
}
void ExpressionVariable::GetVarsUsed(std::vector<Variable *> &vars) {
  for (Variable *var : vars) {
    if (var == pVariable)
      return;
  }
  vars.push_back(pVariable);
}  // list of variables used

bool ExpressionFunctionMemory::TestMarkFlows(SymbolNameSpace *sns, FlowList *fl, Equation *eq) {
  if (!this->GetFunction()->IsIntegrator()) {
    return false;
  }
  // only care about active part here - if it all a+b+c-d-e or similar then we are good to go otherwise
  // we need to make up a new variable and then use that as the equation in place of what was here
  Expression *e = this->GetArgs()->GetExp(0);
  if (eq)  // make a change
  {
    assert(fl);
    // we set the first argument for the variable in the flow list
    ExpressionVariable *ev = new ExpressionVariable(sns, fl->NewVariable(), eq->GetLeft()->GetSubs());
    this->GetArgs()->SetExp(0, ev);

  } else if (fl)  // populat flow list
  {
    is_all_plus_minus(e, fl, false);
    fl->SetActiveExpression(e);
  }
  return true;
}

void FlowList::AddInflow(Variable *v) {
  if (std::find(vInflows.begin(), vInflows.end(), v) != vInflows.end())
    bValid = false;
  else if (std::find(vOutflows.begin(), vOutflows.end(), v) != vOutflows.end())
    bValid = false;
  else
    vInflows.push_back(v);
}

void FlowList::AddOutflow(Variable *v) {
  if (std::find(vInflows.begin(), vInflows.end(), v) != vInflows.end())
    bValid = false;
  else if (std::find(vOutflows.begin(), vOutflows.end(), v) != vOutflows.end())
    bValid = false;
  else
    vOutflows.push_back(v);
}

bool FlowList::operator==(const FlowList &rhs) {
  if (!bValid || !rhs.bValid || vInflows.size() != rhs.vInflows.size() || vOutflows.size() != rhs.vOutflows.size())
    return false;
  for (const Variable *v : rhs.vInflows) {
    if (std::find(vInflows.begin(), vInflows.end(), v) == vInflows.end())
      return false;
  }
  for (const Variable *v : rhs.vOutflows) {
    if (std::find(vOutflows.begin(), vOutflows.end(), v) == vOutflows.end())
      return false;
  }
  return true;
}

namespace {

// True when `s` is one parenthesized group spanning the whole string: "(a+b)"
// or "( IF c THEN a ELSE b )", but NOT "(a)*INT((a)/(b))" (the leading group
// closes before the end) and not "POISSON(x)*2" (it does not open with one).
// Deliberately a scan rather than a first/last character test -- the closing
// paren of the opening group has to be the last character for the pair to be
// the outermost grouping.
//
// Only the parens the RE-PARSER would read as grouping may be counted. A
// quoted identifier carries its own: `"x("` is one name, and its paren is a
// name character, not a group. `XmileEqLex::yylex` opens a quoted symbol at
// any '"' and `ScanQuotedSymbol` ends it at the next one, so this scan skips
// exactly those spans. Counting them was wrong in BOTH directions -- an
// unmatched '(' in one name and an unmatched ')' in another let the depth hit
// zero at the last character of a string that is not one group (dropping a
// needed paren and silently reassociating the expression), while a lone
// unmatched paren in a name made every string look unbalanced (so the
// redundant layer this predicate exists to suppress accumulated forever).
//
// Where the scan cannot be sure what the lexer will see, it answers false and
// the caller keeps its parentheses: a redundant pair is merely verbose, a
// dropped one changes the model.
//
// The apostrophe is the character with more than one reading. It delimits a
// Vensim string literal (`ExpressionLiteral` renders the token with its
// quotes, so `GET DIRECT DATA('f(x).xlsx',...)` puts parens inside one) and it
// is also an ordinary character in a bare Vensim name (`don't`). The two
// readings disagree about which parens are grouping, so the scan runs BOTH and
// suppresses only on agreement -- rather than refusing the character outright,
// which answered "keep" for every literal-bearing rendering and left the
// self-delimiting ones carrying a spare layer.

// How one scan reads an apostrophe.
enum class ApostropheReading {
  NameChar,  // an ordinary character; the parens around it are counted
  Literal,   // opens a Vensim string literal that ends at the next apostrophe
};

enum class GroupScan {
  Whole,         // one parenthesized group spanning the whole string
  NotWhole,      // definitely not
  Inapplicable,  // the text cannot be lexed this way at all, so this is not a
                 // reading OF it (an unterminated literal, an unknowable name)
};

GroupScan ScanForWholeParenGroup(const std::string &s, ApostropheReading reading) {
  int depth = 0;
  for (size_t i = 0; i < s.size(); i++) {
    const char c = s[i];
    if (c == '"') {
      // Where a quoted name ends is not agreed on by the two lexers that read
      // this text: XmileEqLex recognizes no escapes, while the Vensim reader
      // stores an interior quote as \" (mdl::EscapeMDLQuotedIdent) and skips
      // the escaped character. A backslash inside the span therefore makes the
      // name's extent unknowable here, under either reading.
      size_t end = i + 1;
      while (end < s.size() && s[end] != '"') {
        if (s[end] == '\\')
          return GroupScan::Inapplicable;
        end++;
      }
      if (end == s.size())
        return GroupScan::Inapplicable;  // unterminated quote
      i = end;
      continue;
    }
    if (c == '\'' && reading == ApostropheReading::Literal) {
      const size_t end = s.find('\'', i + 1);
      if (end == std::string::npos)
        return GroupScan::Inapplicable;  // nothing closes it, so it is no literal
      i = end;
      continue;
    }
    if (c == '(')
      depth++;
    else if (c == ')') {
      depth--;
      if (depth == 0)
        return (i + 1 == s.size()) ? GroupScan::Whole : GroupScan::NotWhole;
    }
  }
  return GroupScan::NotWhole;  // unbalanced; leave the grouping alone
}

bool IsWholeParenGroup(const std::string &s) {
  if (s.size() < 2 || s.front() != '(' || s.back() != ')')
    return false;
  const GroupScan readings[] = {
      ScanForWholeParenGroup(s, ApostropheReading::NameChar),
      ScanForWholeParenGroup(s, ApostropheReading::Literal),
  };
  // Suppress only when every reading the text admits agrees. A text with no
  // apostrophe admits both readings identically; one with a lone apostrophe
  // admits only NameChar (nothing closes a literal), which is what settles
  // `don't`; and one where a literal's parens would change the count leaves the
  // two disagreeing, so the paren is kept.
  bool anyApplicable = false;
  for (GroupScan r : readings) {
    if (r == GroupScan::Inapplicable)
      continue;
    if (r != GroupScan::Whole)
      return false;
    anyApplicable = true;
  }
  return anyApplicable;
}

}  // namespace

void ExpressionParen::OutputComputable(ContextInfo *info) {
  if (!pE1) {
    *info << "()";
    return;
  }
  // A paren node is grouping, and grouping is the ONLY thing that keeps this
  // serializer's output faithful: the binary-operator nodes emit "e1 op e2"
  // with no parentheses of their own, so dropping a paren can reassociate the
  // expression. But some renderings already delimit themselves -- IF THEN ELSE,
  // SAMPLE IF TRUE and the expanded PULSE forms all emit "( ... )" around their
  // whole output -- and re-wrapping those is what made XMILE -> XMILE grow one
  // paren layer per conversion without bound: the writer emitted "(( IF ... ))"
  // where it had read "( IF ... )", the reader turned the extra pair into
  // another paren node, and nothing ever removed one.
  //
  // So the emission is suppressed exactly when the child's own text is already
  // a single enclosing group, which is decided from the rendered text rather
  // than from the node kind on purpose: whether a rendering is self-delimiting
  // depends on the function, on its arity, and (for the memory functions) on
  // which arguments the current ContextInfo selects, and a node-kind table that
  // missed one case would leave that construct growing forever. The cost of
  // reading the text instead is that the reading has to agree with the lexer
  // that will read it back -- see IsWholeParenGroup, which is why quoted names
  // are skipped rather than scanned.
  const std::ostringstream::pos_type start = info->tellp();
  if (static_cast<std::streamoff>(start) < 0) {
    // A failed stream reports -1 and there is no position to rewind to, so the
    // grouping goes out unconditionally rather than through the substr below.
    *info << '(';
    pE1->OutputComputable(info);
    *info << ')';
    return;
  }
  pE1->OutputComputable(info);
  // ostringstream::str() copies the whole buffer and pre-C++20 there is no
  // view() to ask for the tail instead, so drop the prefix in place rather than
  // allocating a second string for it. The buffer is one equation's rendering
  // (Equation::RHSFormattedXMILE constructs a fresh ContextInfo per equation),
  // which is what bounds this: the copy is per paren node, over that equation's
  // text, not over the document.
  std::string inner = info->str();
  inner.erase(0, static_cast<size_t>(start));
  if (IsWholeParenGroup(inner))
    return;
  // Rewind over the child's text and re-emit it wrapped. ContextInfo is an
  // ostringstream, so seeking back and writing overwrites in place; the
  // replacement is strictly longer than what it covers, so no stale tail
  // survives and the put position ends up at the end of the buffer again.
  info->seekp(start);
  *info << '(' << inner << ')';
}

void ExpressionLogical::OutputComputable(ContextInfo *info) {
  // The separator ahead of the operator belongs to the left operand, so it is
  // written only when there is one. Unary NOT parses with an empty left slot
  // (its operand lives in pE2), and the unconditional leading space doubled up
  // against the space the caller had already written -- "( IF " + " not x".
  if (pE1) {
    pE1->OutputComputable(info);
    *info << ' ';
  }
  switch (mOper) {
  case VPTT_le:
    *info << "<=";
    break;
  case VPTT_ge:
    *info << ">=";
    break;
  case VPTT_ne:
    *info << "<>";
    break;
  case VPTT_and:
    *info << "and";
    break;
  case VPTT_or:
    *info << "or";
    break;
  case VPTT_not:
    *info << "not";
    break;

  default:
    assert(mOper < 128);
    *info << (char)mOper;
    break;
  }
  *info << ' ';
  if (pE2)
    pE2->OutputComputable(info);
}

void ExpressionTable::SetXAxis(Variable *var, double xmin, double xmax, double increment) {
  if (increment > 0) {
    if (xmax > xmin) {
      int count = std::round((xmax - xmin) / increment + 1);
      if (count != vYVals.size())
        log("Error the table function %s has %d entries but its usage suggests %d\n", var->GetName().c_str(),
            (int)vYVals.size(), count);
    } else
      log("Error the table function %s is used in a table without a proper min/max\n", var->GetName().c_str());
  } else {
    log("Error the table function %s is used in a table without an increment\n", var->GetName().c_str());
    increment = 1;
  }
  for (int i = 0; i < vYVals.size(); i++, xmin += increment)
    vXVals.push_back(xmin);
}
void ExpressionTable::TransformLegacy() {
  assert(!(vXVals.size() % 2));
  size_t n = vXVals.size() / 2;
  for (size_t i = 0; i < n; i++)
    vYVals[i] = vXVals[n + i];
  vXVals.resize(n);
  vYVals.resize(n);
}

void ExpressionLookup::OutputComputable(ContextInfo *info) {
  if (pExpressionVariable) {
      // curious that Claude believes the below
    // WRONG  Applying a named graphical function to an input is written as direct
    // WRONG  application -- "table(input)" -- in both XMILE and Vensim. (The XMILE
    // WRONG  reader also accepts the explicit "LOOKUP(table, input)" spelling some
    // WRONG  producers emit; both parse back to this node.) The only consumer of
    // WRONG  this path is the XMILE writer; the MDL writer renders lookups through
    // WRONG  MDLGenerator::RenderTableLike.
      *info << "LOOKUP(";
    pExpressionVariable->OutputComputable(info);
    *info << ", ";
    pExpression->OutputComputable(info);
    *info << ")";
  } else {
    pExpression->OutputComputable(info);
  }
}
