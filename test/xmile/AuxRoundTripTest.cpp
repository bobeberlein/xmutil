// Phase 3 round-trip tests for the XMILE reader: auxes-only models, sim specs
// (including dt reciprocal resolution), unit definitions, view-level groups,
// foreign-namespace skipping, and <module> rejection. Each fixture is a hand-
// written XMILE document so the wire shape under test is visible at the call
// site.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

const char *kAuxesOnly = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <header><name>aux-only</name></header>
  <sim_specs method="euler">
    <start>0</start>
    <stop>100</stop>
    <dt>1</dt>
  </sim_specs>
  <model>
    <variables>
      <aux name="a"><eqn>2</eqn></aux>
      <aux name="b"><eqn>3</eqn></aux>
      <aux name="c"><eqn>a * b</eqn><units>widgets</units><doc>a comment</doc></aux>
    </variables>
  </model>
</xmile>
)";

const char *kWithModule = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <header><name>with-module</name></header>
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables>
      <aux name="x"><eqn>1</eqn></aux>
      <module name="m1"/>
    </variables>
  </model>
</xmile>
)";

const char *kForeignNamespace = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0"
       xmlns:isee="http://iseesystems.com/XMILE">
  <header><name>with-isee</name></header>
  <isee:prefs show_module_prefix="false"/>
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables>
      <aux name="x"><eqn>1</eqn></aux>
      <isee:loop_indicator/>
    </variables>
  </model>
</xmile>
)";

const char *kWithUnits = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <header><name>with-units</name></header>
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model_units>
    <unit name="Dollar"><eqn>$</eqn><alias>Dollars</alias><alias>$s</alias></unit>
    <unit name="Widget"><eqn>w</eqn></unit>
  </model_units>
  <model><variables><aux name="x"><eqn>1</eqn></aux></variables></model>
</xmile>
)";

const char *kSimSpecsReciprocalDt = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <header><name>recip-dt</name></header>
  <sim_specs method="rk4">
    <start>0</start>
    <stop>10</stop>
    <dt reciprocal="true">4</dt>
  </sim_specs>
  <model><variables><aux name="x"><eqn>1</eqn></aux></variables></model>
</xmile>
)";

const char *kWithGroup = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <header><name>with-group</name></header>
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables>
      <aux name="x"><eqn>1</eqn></aux>
      <aux name="y"><eqn>x + 1</eqn></aux>
    </variables>
    <views>
      <view view_type="stock_flow">
        <group name="MyGroup"><var>x</var><var>y</var></group>
      </view>
    </views>
  </model>
</xmile>
)";

// The writer emits "a/(b*c)" when a unit expression has one numerator and two
// denominator terms. This fixture verifies that the reader correctly parses the
// parenthesized denominator group back into the same UnitExpression shape so the
// round-trip comparison sees identical structures on both sides.
const char *kCompoundUnits = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <header><name>compound-units</name></header>
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables>
      <aux name="density"><eqn>1</eqn><units>Persons/(Day*SqMeter)</units></aux>
    </variables>
  </model>
</xmile>
)";

}  // namespace

TEST(AuxRoundTrip_auxes_only_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kAuxesOnly);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(AuxRoundTrip_module_is_rejected) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kWithModule, errs);
  CHECK(m == nullptr);
  CHECK(!errs.empty());
  bool sawModuleError = false;
  for (const std::string &e : errs) {
    if (e.find("modules are not supported") != std::string::npos)
      sawModuleError = true;
  }
  CHECK(sawModuleError);
}

TEST(AuxRoundTrip_foreign_namespace_silently_skipped) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kForeignNamespace, errs);
  CHECK(m != nullptr);
  CHECK(errs.empty());
  if (m) {
    CHECK(m->GetNameSpace()->Find("x") != nullptr);
    delete m;
  }
}

TEST(AuxRoundTrip_units_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kWithUnits);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(AuxRoundTrip_dt_reciprocal_resolves) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kSimSpecsReciprocalDt, errs);
  CHECK(m != nullptr);
  CHECK(errs.empty());
  if (m) {
    CHECK(m->dt() == 0.25);
    CHECK(m->IntegrationType() == Integration_Type_RK4);
    delete m;
  }
}

TEST(AuxRoundTrip_group_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kWithGroup);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(AuxRoundTrip_xmile_to_mdl) {
  std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(kAuxesOnly);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(AuxRoundTrip_compound_units_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kCompoundUnits);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}
