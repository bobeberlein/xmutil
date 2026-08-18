// Phase 5 round-trip tests for the XMILE reader: dimension definitions and
// subscripted variables (apply-to-all and per-element shapes).
//
// Each fixture is a hand-written XMILE document so the wire shape under test
// is visible at the call site. The XMILE corpus has no arrayed models, so
// dimension coverage rides entirely on these fixtures.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/Symbol/Variable.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

const char *kApplyToAllAux = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <dimensions><dim name="Dim"><elem name="a"/><elem name="b"/><elem name="c"/></dim></dimensions>
    <variables>
      <aux name="inflow">
        <eqn>5</eqn>
        <dimensions><dim name="Dim"/></dimensions>
      </aux>
    </variables>
  </model>
</xmile>
)";

const char *kPerElementAux = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <dimensions><dim name="Dim"><elem name="a"/><elem name="b"/></dim></dimensions>
    <variables>
      <aux name="inflow">
        <element subscript="a"><eqn>1</eqn></element>
        <element subscript="b"><eqn>2</eqn></element>
        <dimensions><dim name="Dim"/></dimensions>
      </aux>
    </variables>
  </model>
</xmile>
)";

const char *kApplyToAllStock = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <dimensions><dim name="Dim"><elem name="a"/><elem name="b"/><elem name="c"/></dim></dimensions>
    <variables>
      <stock name="s">
        <eqn>10</eqn>
        <dimensions><dim name="Dim"/></dimensions>
        <inflow>in</inflow>
        <outflow>out</outflow>
      </stock>
      <flow name="in"><eqn>5</eqn><dimensions><dim name="Dim"/></dimensions></flow>
      <flow name="out"><eqn>2</eqn><dimensions><dim name="Dim"/></dimensions></flow>
    </variables>
  </model>
</xmile>
)";

const char *kPerElementStock = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <dimensions><dim name="Dim"><elem name="a"/><elem name="b"/></dim></dimensions>
    <variables>
      <stock name="s">
        <element subscript="a"><eqn>10</eqn></element>
        <element subscript="b"><eqn>20</eqn></element>
        <inflow>in</inflow>
        <dimensions><dim name="Dim"/></dimensions>
      </stock>
      <flow name="in"><eqn>5</eqn><dimensions><dim name="Dim"/></dimensions></flow>
    </variables>
  </model>
</xmile>
)";

const char *kIndexedDim = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <dimensions><dim name="Dim" size="3"/></dimensions>
    <variables>
      <aux name="inflow"><eqn>5</eqn><dimensions><dim name="Dim"/></dimensions></aux>
    </variables>
  </model>
</xmile>
)";

}  // namespace

TEST(Array_apply_to_all_aux_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kApplyToAllAux);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Array_per_element_aux_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kPerElementAux);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Array_apply_to_all_stock_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kApplyToAllStock);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Array_per_element_stock_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kPerElementStock);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Array_dim_classified_as_ARRAY_after_pipeline) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kApplyToAllAux, errs);
  CHECK(m != nullptr);
  CHECK(errs.empty());
  if (!m)
    return;
  Variable *dim = static_cast<Variable *>(m->GetNameSpace()->Find("Dim"));
  CHECK(dim != nullptr);
  if (dim)
    CHECK(dim->VariableType() == XMILE_Type_ARRAY);
  Variable *a = static_cast<Variable *>(m->GetNameSpace()->Find("a"));
  CHECK(a != nullptr);
  if (a)
    CHECK(a->VariableType() == XMILE_Type_ARRAY_ELM);
  delete m;
}

TEST(Array_indexed_dim_expands_to_numeric_elements) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kIndexedDim, errs);
  CHECK(m != nullptr);
  CHECK(errs.empty());
  if (!m)
    return;
  // Synthesized elements "1", "2", "3" should exist in the namespace.
  CHECK(m->GetNameSpace()->Find("1") != nullptr);
  CHECK(m->GetNameSpace()->Find("2") != nullptr);
  CHECK(m->GetNameSpace()->Find("3") != nullptr);
  delete m;
}

TEST(Array_apply_to_all_to_mdl) {
  std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(kApplyToAllAux);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}
