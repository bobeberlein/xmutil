// Regression coverage for XMILE names that collide with a registered builtin.
//
// Every function the XmileReader ctor registers (STEP, SUM, TREND, ...)
// occupies its name in the same namespace variables live in. XMILE keeps the
// two apart (a call is a name followed by '('), so a .xmile carrying such a
// name is ordinary user input: nothing in XMILE reserves `Step` as a variable
// name, and Stella happily exports one (FRIDA has a macro parm `initial`).
// XmileReader::RenameAwayFromBuiltin enters the variable as `Step 1` and says
// so. These sites once handed the NULL InsertVariable then answered to code
// that dereferences it; each case below has to convert, with the warning.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

// A <view> element naming a builtin, for a variable that is never declared.
// InsertVariable once answered NULL here, and the three XmileView allocators
// fed it straight to VensimVariableElement's ctor, whose first act is
// `var->GetView()`.
const char *kViewAuxNamesBuiltin = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables><aux name="x"><eqn>1</eqn></aux></variables>
    <views><view>
      <aux name="x" x="10" y="10"/>
      <aux name="Step" x="200" y="100"/>
    </view></views>
  </model>
</xmile>
)";

const char *kViewFlowNamesBuiltin = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables><aux name="x"><eqn>1</eqn></aux></variables>
    <views><view>
      <aux name="x" x="10" y="10"/>
      <flow name="Trend" x="200" y="100"><pts><pt x="180" y="100"/><pt x="220" y="100"/></pts></flow>
    </view></views>
  </model>
</xmile>
)";

const char *kViewAliasNamesBuiltin = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables><aux name="x"><eqn>1</eqn></aux></variables>
    <views><view>
      <aux name="x" x="10" y="10"/>
      <alias x="200" y="100"><of>Sum</of></alias>
    </view></views>
  </model>
</xmile>
)";

// A stock whose <inflow>/<outflow> names a builtin. Unlike a sketch element,
// this claim is structural: the net-flow expression BuildNetFlowSubscripted
// synthesizes IS the stock's equation, so the reference has to reach the same
// renamed variable a <flow name="Step"> declaration would.
const char *kStockInflowNamesBuiltin = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <stock name="s"><eqn>0</eqn><inflow>Step</inflow></stock>
  </variables></model>
</xmile>
)";

const char *kStockOutflowNamesBuiltin = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <stock name="s"><eqn>0</eqn><outflow>Sum</outflow></stock>
  </variables></model>
</xmile>
)";

// The outflow-only branch takes a different path through the builder (the
// leading unary minus), so it needs its own case.
const char *kStockSubscriptedFlowNamesBuiltin = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <dimensions><dim name="D"><elem name="a"/><elem name="b"/></dim></dimensions>
    <variables>
      <stock name="s"><dimensions><dim name="D"/></dimensions><eqn>0</eqn><inflow>Trend</inflow></stock>
    </variables>
  </model>
</xmile>
)";

// The parse survives, the colliding name is renamed, and the rename is
// explained. Everything else in the document still converts.
void CheckSurvivesWithDiagnostic(const char *xmile, const char *expectedSubstring) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(xmile, errs);
  CHECK(m != nullptr);
  if (!m)
    return;

  bool sawExpected = false;
  for (const std::string &e : errs) {
    if (e.find(expectedSubstring) != std::string::npos)
      sawExpected = true;
  }
  if (!sawExpected) {
    printf("  expected substring not found: %s\n", expectedSubstring);
    for (const std::string &e : errs)
      printf("  err: %s\n", e.c_str());
  }
  CHECK(sawExpected);

  // Both writers must still produce output -- the collision costs one drawing,
  // not the conversion.
  std::vector<std::string> mdlErrs;
  std::string mdl = m->PrintMDL(mdlErrs);
  CHECK(!mdl.empty());
  std::vector<std::string> xmileErrs;
  std::string out = m->PrintXMILE(/*isCompact=*/false, xmileErrs, 1.0, 1.0);
  CHECK(!out.empty());

  delete m;
}

}  // namespace

TEST(BuiltinNameCollision_view_aux_is_renamed) {
  CheckSurvivesWithDiagnostic(kViewAuxNamesBuiltin, "is the name of the built-in function");
}

TEST(BuiltinNameCollision_view_flow_is_renamed) {
  CheckSurvivesWithDiagnostic(kViewFlowNamesBuiltin, "is the name of the built-in function");
}

TEST(BuiltinNameCollision_view_alias_is_renamed) {
  CheckSurvivesWithDiagnostic(kViewAliasNamesBuiltin, "is the name of the built-in function");
}

TEST(BuiltinNameCollision_stock_inflow_is_renamed) {
  CheckSurvivesWithDiagnostic(kStockInflowNamesBuiltin, "is the name of the built-in function");
}

TEST(BuiltinNameCollision_stock_outflow_is_renamed) {
  CheckSurvivesWithDiagnostic(kStockOutflowNamesBuiltin, "is the name of the built-in function");
}

TEST(BuiltinNameCollision_subscripted_stock_flow_is_renamed) {
  CheckSurvivesWithDiagnostic(kStockSubscriptedFlowNamesBuiltin, "is the name of the built-in function");
}

// The renamed variable is the one every reference reaches, declared or used
// first, and a macro parm is renamed the same way inside the macro's own
// namespace.
TEST(BuiltinNameCollision_declared_variable_and_macro_parm_are_renamed) {
  const char *kXmile = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <macro name="GROW">
    <eqn>input*(1+initial)</eqn>
    <parm>input</parm>
    <parm>initial</parm>
  </macro>
  <model><variables>
    <aux name="y"><eqn>trend*2 + GROW(trend, 1)</eqn></aux>
    <aux name="trend"><eqn>3</eqn></aux>
  </variables></model>
</xmile>
)";
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kXmile, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  int renames = 0;
  for (const std::string &e : errs) {
    if (e.find("is the name of the built-in function") != std::string::npos)
      renames++;
  }
  CHECK(renames == 2);  // trend, and the macro's initial
  std::vector<std::string> mdlErrs;
  const std::string mdl = m->PrintMDL(mdlErrs);
  CHECK(mdl.find("y = trend 1 * 2 + GROW(trend 1, 1)") != std::string::npos);
  CHECK(mdl.find("trend 1 = 3") != std::string::npos);
  CHECK(mdl.find(":MACRO: GROW(input, initial 1)") != std::string::npos);
  delete m;
}
