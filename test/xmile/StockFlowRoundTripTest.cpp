// Phase 4 round-trip tests for the XMILE reader: stocks and flows.
//
// Verifies that <stock> + <flow> elements synthesize the INTEG expression the
// downstream pipeline (MarkVariableTypes, MarkStockFlows) expects, so a model
// with stocks round-trips XMILE -> Model -> XMILE -> Model with no
// ModelComparator diffs and converts cleanly XMILE -> Model -> MDL -> Model.
//
// The headline fixture is the simlin logistic-growth corpus model. Three
// hand-written fixtures exercise specific shapes the corpus does not: a single
// inflow with no outflow, multiple inflows + multiple outflows, and a stock
// that references its inflow by name before the <flow> element is declared
// (forward-reference; InsertVariable's lookup-or-create handles this).

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/Symbol/Variable.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

std::string ReadFile(const std::string &path) {
  std::ifstream in(path, std::ios::in | std::ios::binary);
  if (!in.is_open())
    return std::string();
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

const char *kSingleInflowNoOutflow = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <stock name="s"><eqn>100</eqn><inflow>in</inflow></stock>
    <flow name="in"><eqn>5</eqn></flow>
  </variables></model>
</xmile>
)";

const char *kMultipleInflowsAndOutflows = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <stock name="s">
      <eqn>0</eqn>
      <inflow>in1</inflow>
      <inflow>in2</inflow>
      <outflow>out1</outflow>
    </stock>
    <flow name="in1"><eqn>1</eqn></flow>
    <flow name="in2"><eqn>2</eqn></flow>
    <flow name="out1"><eqn>3</eqn></flow>
  </variables></model>
</xmile>
)";

// The stock declares its inflow before the corresponding <flow> appears in the
// document. InsertVariable creates an empty placeholder when ProcessStock first
// sees "in"; ProcessAuxOrFlow then attaches the equation when it reaches the
// <flow>.
const char *kStockReferencedBeforeFlowDeclared = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <stock name="s"><eqn>10</eqn><inflow>in</inflow></stock>
    <aux name="rate"><eqn>2</eqn></aux>
    <flow name="in"><eqn>rate</eqn></flow>
  </variables></model>
</xmile>
)";

}  // namespace

TEST(StockFlow_corpus_logistic_growth_round_trip) {
  std::string xmile = ReadFile(std::string(XMUTIL_SRC_ROOT) + "/test/fixtures/simlin/logistic-growth.xmile");
  CHECK(!xmile.empty());
  if (xmile.empty())
    return;
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(xmile);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(StockFlow_corpus_logistic_growth_to_mdl) {
  std::string xmile = ReadFile(std::string(XMUTIL_SRC_ROOT) + "/test/fixtures/simlin/logistic-growth.xmile");
  CHECK(!xmile.empty());
  if (xmile.empty())
    return;
  std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(xmile);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(StockFlow_single_inflow_no_outflow_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kSingleInflowNoOutflow);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(StockFlow_multiple_in_and_out_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kMultipleInflowsAndOutflows);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(StockFlow_forward_reference_to_flow_ok) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kStockReferencedBeforeFlowDeclared);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(StockFlow_stock_classified_as_STOCK_after_pipeline) {
  // Direct shape check: after parsing + post-parse pipeline, the stock variable
  // is classified XMILE_Type_STOCK and MarkStockFlows has populated its
  // Inflows()/Outflows() vectors from the synthesized INTEG expression.
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kMultipleInflowsAndOutflows, errs);
  CHECK(m != nullptr);
  CHECK(errs.empty());
  if (!m)
    return;
  Variable *stock = static_cast<Variable *>(m->GetNameSpace()->Find("s"));
  CHECK(stock != nullptr);
  if (stock) {
    CHECK(stock->VariableType() == XMILE_Type_STOCK);
    CHECK(stock->Inflows().size() == 2);
    CHECK(stock->Outflows().size() == 1);
  }
  delete m;
}
