#include <string>

#include "../../src/Function/Function.h"
#include "../../src/Mdl/MDLGenerator.h"
#include "../../src/Model.h"
#include "../../src/Symbol/Equation.h"
#include "../../src/Symbol/Expression.h"
#include "../../src/Symbol/ExpressionList.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

// The comparator compares MacroFunctions(), so a dropped/renamed/corrupted
// macro is a non-empty diff in ExpectCleanRoundTrip (it would otherwise pass
// vacuously).
using roundtrip::ExpectCleanRoundTrip;

// The macro of the given name in a model, or nullptr if absent. Used by the
// focused survival assertion to fetch the same macro from the original and the
// re-parsed model.
MacroFunction *FindMacro(Model *m, const std::string &name) {
  for (MacroFunction *mf : m->MacroFunctions()) {
    if (mf->GetName() == name)
      return mf;
  }
  return nullptr;
}

// test_macro_expression.mdl, inlined verbatim from
// tests/macro_expression/test_macro_expression.mdl in the SDXorg test-models corpus.
// Inlined (rather than read from disk) so the test does not depend on the process
// working directory, matching the EquationRoundTripTest / SketchRoundTripTest
// convention. A macro with two formal parameters whose body is a single
// expression aux (no stock), exercising :MACRO: header, the arg list, a body
// equation with units/comment, and :END OF MACRO:.
const char *kMacroExpressionMdl =
    R"MDL({UTF-8}
:MACRO: EXPRESSION MACRO(input, parameter)
EXPRESSION MACRO = input * parameter
	~	input
	~	tests basic macro containing no stocks and having no output
	|

:END OF MACRO:
macro input=
	5
	~
	~		|

macro output=
	EXPRESSION MACRO(macro input,macro parameter)
	~
	~		|

macro parameter=
	1.1
	~
	~		|

********************************************************
	.Control
********************************************************~
		Simulation Control Parameters
	|

FINAL TIME  = 1
	~	Month
	~	The final time for the simulation.
	|

INITIAL TIME  = 0
	~	Month
	~	The initial time for the simulation.
	|

SAVEPER  =
        TIME STEP
	~	Month [0,?]
	~	The frequency with which output is stored.
	|

TIME STEP  = 1
	~	Month [0,?]
	~	The time step for the simulation.
	|

\\\---/// Sketch information - do not modify anything except names
V300  Do not put anything below this section - it will be ignored
*View 1
$192-192-192,0,Times New Roman|12||0-0-0|0-0-0|0-0-255|-1--1--1|-1--1--1|72,72,100,0
10,1,macro input,255,165,31,8,8,3,0,0,0,0,0,0
10,2,macro output,383,163,34,8,8,3,0,0,0,0,0,0
1,3,1,2,0,0,0,0,0,128,0,-1--1--1,,1|(310,164)|
10,4,macro parameter,351,107,43,8,8,3,0,0,0,0,0,0
1,5,4,2,0,0,0,0,0,128,0,-1--1--1,,1|(363,128)|
///---\\\
:L<%^E!@
1:Current.vdf
9:Current
15:0,0,0,0,0,0
19:100,0
27:2,
34:0,
4:Time
5:macro output
35:Date
36:YYYY-MM-DD
37:2000
38:1
39:1
40:2
41:0
42:0
24:0
25:1
26:1
)MDL";

// test_macro_stock.mdl, inlined verbatim from
// tests/macro_stock/test_macro_stock.mdl in the SDXorg test-models corpus.
// Identical to the expression fixture except the macro body is a stock
// (EXPRESSION MACRO = INTEG(input, parameter)), exercising the Phase 3 stock
// emission path inside a :MACRO: block.
const char *kMacroStockMdl =
    R"MDL({UTF-8}
:MACRO: EXPRESSION MACRO(input, parameter)
EXPRESSION MACRO = INTEG(input, parameter)
	~	input
	~	tests basic macro containing a stock but no output
	|

:END OF MACRO:
macro input=
	5
	~
	~		|

macro output=
	EXPRESSION MACRO(macro input,macro parameter)
	~
	~		|

macro parameter=
	1.1
	~
	~		|

********************************************************
	.Control
********************************************************~
		Simulation Control Parameters
	|

FINAL TIME  = 10
	~	Month
	~	The final time for the simulation.
	|

INITIAL TIME  = 0
	~	Month
	~	The initial time for the simulation.
	|

SAVEPER  =
        TIME STEP
	~	Month [0,?]
	~	The frequency with which output is stored.
	|

TIME STEP  = 1
	~	Month [0,?]
	~	The time step for the simulation.
	|

\\\---/// Sketch information - do not modify anything except names
V300  Do not put anything below this section - it will be ignored
*View 1
$192-192-192,0,Times New Roman|12||0-0-0|0-0-0|0-0-255|-1--1--1|-1--1--1|72,72,100,0
10,1,macro input,255,165,31,8,8,3,0,0,0,0,0,0
10,2,macro output,383,163,34,8,8,3,0,0,0,0,0,0
1,3,1,2,0,0,0,0,0,128,0,-1--1--1,,1|(310,164)|
10,4,macro parameter,351,107,43,8,8,3,0,0,0,0,0,0
1,5,4,2,0,0,0,0,0,128,0,-1--1--1,,1|(363,128)|
///---\\\
:L<%^E!@
1:Current.vdf
9:Current
15:0,0,0,0,0,0
19:100,0
27:2,
34:0,
4:Time
5:macro output
35:Date
36:YYYY-MM-DD
37:2000
38:1
39:1
40:2
41:0
42:0
24:0
25:10
26:10
)MDL";

}  // namespace

// AC6.1: a macro with no stock -- a single-expression body output -- emits a
// :MACRO: ... :END OF MACRO: block that re-parses to an equivalent macro. An
// empty diff list means the macro's name, parameter list, and body (the
// EXPRESSION MACRO aux, its RHS, units, and comment) all matched after
// parse -> write -> re-parse.
TEST(MacroRoundTrip_expression_macro) {
  ExpectCleanRoundTrip(kMacroExpressionMdl);
}

// AC6.1: a macro whose body is a stock (= INTEG(...)) round-trips. This exercises
// the Phase 3 stock emission path inside the macro block. The stock's net flow
// here is the bare parameter `input`, which decomposes to a flow rather than a
// reader-synthesized "<stock> net flow"; the dedicated synthetic-flow case is
// covered by MacroRoundTrip_synthetic_net_flow_in_macro_body below.
TEST(MacroRoundTrip_stock_macro) {
  ExpectCleanRoundTrip(kMacroStockMdl);
}

// AC6.1 (regression): a macro body containing a stock whose net flow is NOT a
// clean +/- of named flows (here `a * 2`) makes MarkVariableTypes synthesize a
// "<stock> net flow" variable in the MACRO's namespace. The writer must inline
// that synthetic flow's expression back into the INTEG and suppress it as a
// standalone body variable -- exactly as the main-model loop does -- or the macro
// body would double-emit the synthetic flow and re-synthesize a duplicate on
// re-parse. This is the case the macro_stock fixture does NOT trigger, so it is
// constructed explicitly to guard the macro-loop suppression.
TEST(MacroRoundTrip_synthetic_net_flow_in_macro_body) {
  const std::string mdl =
      "{UTF-8}\r\n"
      ":MACRO: SYNTH MACRO(in)\r\n"
      "a = in * 2\r\n\t~~|\r\n"
      "lvl = INTEG(a * 2, 0)\r\n\t~~|\r\n"
      ":END OF MACRO:\r\n"
      "out = SYNTH MACRO(5)\r\n\t~~|\r\n"
      "INITIAL TIME = 0\r\n\t~~|\r\n"
      "FINAL TIME = 10\r\n\t~~|\r\n"
      "TIME STEP = 1\r\n\t~~|\r\n"
      "SAVEPER = TIME STEP\r\n\t~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// AC6.1 (focused, non-vacuous): the macro name, its parameter list, and its body
// equation's RHS survive the round trip. This guards specifically against a
// writer that dropped or renamed the macro, mangled its parameters, or corrupted
// its body expression. The assertion is non-vacuous because it FIRST confirms the
// ORIGINAL model genuinely has the macro with the expected two parameters and a
// body equation (a model lacking these would make the survival checks pass
// trivially), then confirms the re-parsed model carries an equivalent macro.
TEST(MacroRoundTrip_name_params_body_survive) {
  Model *m0 = roundtrip::ParseVensim(kMacroExpressionMdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  // Non-vacuity precondition: the source model must actually have the macro,
  // with exactly two parameters and a body equation, before we assert survival.
  MacroFunction *mf0 = FindMacro(m0, "EXPRESSION MACRO");
  CHECK(mf0 != nullptr);
  if (!mf0) {
    delete m0;
    return;
  }
  ExpressionList *args0 = mf0->Args();
  CHECK(args0 != nullptr);
  CHECK(args0 && args0->Length() == 2);

  std::vector<Variable *> body0 = m0->GetVariables(mf0->NameSpace());
  Variable *bodyVar0 = nullptr;
  for (Variable *v : body0) {
    if (v->GetName() == "EXPRESSION MACRO")
      bodyVar0 = v;
  }
  CHECK(bodyVar0 != nullptr);
  Equation *bodyEq0 = bodyVar0 ? bodyVar0->GetEquation(0) : nullptr;
  CHECK(bodyEq0 != nullptr);

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  CHECK(errs.empty());

  Model *m1 = roundtrip::ParseVensim(regen);
  CHECK(m1 != nullptr);
  if (!m1) {
    delete m0;
    return;
  }

  // Survival: the re-parsed model has a macro of the same name, the same number
  // of parameters, and a body equation whose RHS is structurally equal.
  MacroFunction *mf1 = FindMacro(m1, "EXPRESSION MACRO");
  CHECK(mf1 != nullptr);
  if (mf1) {
    ExpressionList *args1 = mf1->Args();
    CHECK(args1 != nullptr);
    CHECK(args1 && args0 && args1->Length() == args0->Length());

    std::vector<Variable *> body1 = m1->GetVariables(mf1->NameSpace());
    Variable *bodyVar1 = nullptr;
    for (Variable *v : body1) {
      if (v->GetName() == "EXPRESSION MACRO")
        bodyVar1 = v;
    }
    CHECK(bodyVar1 != nullptr);
    Equation *bodyEq1 = bodyVar1 ? bodyVar1->GetEquation(0) : nullptr;
    CHECK(bodyEq1 != nullptr);

    // Render both body RHSs through the writer's expression walker and compare.
    // The walker drops parser paren nodes and re-derives parentheses from
    // precedence, so the rendered strings are a stable, paren-insensitive
    // signature of the expression -- a corrupted body (a dropped factor, a
    // mangled operator) would render differently. Non-vacuity is established
    // above: bodyEq0 is confirmed present before this runs.
    if (bodyEq0 && bodyEq1) {
      MDLGenerator gen(nullptr);
      std::string rhs0 = gen.RenderExpression(bodyEq0->GetExpression());
      std::string rhs1 = gen.RenderExpression(bodyEq1->GetExpression());
      CHECK(!rhs0.empty());  // a real expression renders to non-empty text
      CHECK_EQ_STR(rhs0, rhs1);
    }
  }

  delete m0;
  delete m1;
}
