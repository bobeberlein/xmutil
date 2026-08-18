// Regression coverage for XMILE names that collide with a registered builtin.
//
// XmileReader::InsertVariable answers NULL for a name already held by a
// non-Variable symbol -- every function the XmileReader ctor registers (STEP,
// SUM, TREND, ...) occupies its name in the same namespace variables live in.
// Declaration sites have always checked that (DeclareVariable reports "name
// collides with a non-variable symbol" and fails the conversion); the sites
// exercised here did not, and handed the NULL to code that dereferences it.
//
// A .xmile carrying such a name is ordinary user input, not a malformed
// document: nothing in XMILE reserves `Step` as a variable name, and Stella
// happily exports one. So each case below has to reach a diagnostic rather
// than a signal.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

// A <view> element naming a builtin, for a variable that is never declared.
// InsertVariable answers NULL, and the three XmileView allocators fed it
// straight to VensimVariableElement's ctor, whose first act is
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
// synthesizes IS the stock's equation, so there is nothing to drop and still
// have the same model. A <flow name="Step"> declaration already fails the
// conversion (DeclareVariable), and the reference form has to agree.
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

void CheckRejected(const char *xmile, const char *expectedSubstring) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(xmile, errs);
  CHECK(m == nullptr);
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
  delete m;
}

// The parse survives, the colliding sketch element is dropped, and the drop is
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

TEST(BuiltinNameCollision_view_aux_is_dropped_not_dereferenced) {
  CheckSurvivesWithDiagnostic(kViewAuxNamesBuiltin, "collides with a non-variable symbol");
}

TEST(BuiltinNameCollision_view_flow_is_dropped_not_dereferenced) {
  CheckSurvivesWithDiagnostic(kViewFlowNamesBuiltin, "collides with a non-variable symbol");
}

TEST(BuiltinNameCollision_view_alias_is_dropped_not_dereferenced) {
  CheckSurvivesWithDiagnostic(kViewAliasNamesBuiltin, "collides with a non-variable symbol");
}

TEST(BuiltinNameCollision_stock_inflow_is_rejected) {
  CheckRejected(kStockInflowNamesBuiltin, "collides with a non-variable symbol");
}

TEST(BuiltinNameCollision_stock_outflow_is_rejected) {
  CheckRejected(kStockOutflowNamesBuiltin, "collides with a non-variable symbol");
}

TEST(BuiltinNameCollision_subscripted_stock_flow_is_rejected) {
  CheckRejected(kStockSubscriptedFlowNamesBuiltin, "collides with a non-variable symbol");
}
