// Phase 7 view round-trip tests for the XMILE reader.
//
// Covers the sketch-geometry path produced by XmileView: variables, flows,
// cloud endpoints, aliases, connectors (including polarity), groups, and the
// multi-view soft-warning path. The corpus tests exercise the full surface
// against the simlin fishbanks and reliability XMILE files; three hand-written
// fixtures pin down the cloud, polarity, and multi-view shapes that are hard
// to isolate in the corpora. The final test is the AC3.4 guard: a deliberate
// rename after re-parse must be flagged by the comparator, proving the
// round-trip tests can fail when content actually drifts.
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/Symbol/Variable.h"
#include "../../src/Vensim/VensimView.h"
#include "../TestHarness.h"
#include "../mdl/ModelComparator.h"
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

// A stock with one inflow and one outflow whose pipe pts on the outer ends are
// far enough from any variable position to be classified as clouds; the inner
// ends sit at the stock's boundary. After ProcessView the view should hold
// exactly two VensimCommentElement nodes (the two clouds).
const char *kClouds = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables>
      <stock name="s"><eqn>0</eqn><inflow>in</inflow><outflow>out</outflow></stock>
      <flow name="in"><eqn>1</eqn></flow>
      <flow name="out"><eqn>1</eqn></flow>
    </variables>
    <views><view view_type="stock_flow">
      <stock name="s" x="200" y="200"/>
      <flow name="in" x="120" y="200">
        <pts><pt x="50" y="200"/><pt x="190" y="200"/></pts>
      </flow>
      <flow name="out" x="280" y="200">
        <pts><pt x="210" y="200"/><pt x="350" y="200"/></pts>
      </flow>
    </view></views>
  </model>
</xmile>
)";

// Two auxes with a single connector carrying polarity="+". The XMILE -> MDL
// -> Vensim re-parse round-trip must preserve the '+' polarity byte on the
// emitted MDL connector record.
const char *kPolarityConnector = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables>
      <aux name="a"><eqn>1</eqn></aux>
      <aux name="b"><eqn>a</eqn></aux>
    </variables>
    <views><view view_type="stock_flow">
      <aux name="a" x="100" y="100"/>
      <aux name="b" x="200" y="100"/>
      <connector polarity="+"><from>a</from><to>b</to></connector>
    </view></views>
  </model>
</xmile>
)";

// A flow whose left pipe endpoint sits 10px from an unrelated aux ("decoy")
// and far from any stock. Endpoint resolution must not attach the pipe to the
// decoy just because it is geometrically close: only stocks that list the
// flow in an <inflow>/<outflow> are valid anchors, and with none in range the
// endpoint is a cloud.
const char *kDecoyAux = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables>
      <stock name="s"><eqn>0</eqn><inflow>in</inflow></stock>
      <flow name="in"><eqn>1</eqn></flow>
      <aux name="decoy"><eqn>1</eqn></aux>
    </variables>
    <views><view view_type="stock_flow">
      <stock name="s" x="200" y="200"/>
      <aux name="decoy" x="60" y="200"/>
      <flow name="in" x="120" y="200">
        <pts><pt x="50" y="200"/><pt x="190" y="200"/></pts>
      </flow>
    </view></views>
  </model>
</xmile>
)";

// A connector that references its endpoint in Stella's canonical form
// (underscores for spaces) AND a different letter case than the view's own
// <aux name="..."> spelling. The namespace folds case and '_'<->' ' when
// resolving variables, so the view-layer name maps must fold the same way --
// otherwise the connector resolves in the model but silently drops from the
// sketch.
const char *kCaseFoldedConnector = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables>
      <aux name="My Var"><eqn>1</eqn></aux>
      <aux name="b"><eqn>My_Var</eqn></aux>
    </variables>
    <views><view view_type="stock_flow">
      <aux name="My Var" x="100" y="100"/>
      <aux name="b" x="200" y="100"/>
      <connector><from>MY_VAR</from><to>b</to></connector>
    </view></views>
  </model>
</xmile>
)";

// Two <view> elements both declaring aux "a". v1 takes the first and emits a
// soft warning containing "multi-view" for each additional view.
const char *kMultiView = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables><aux name="a"><eqn>1</eqn></aux></variables>
    <views>
      <view view_type="stock_flow"><aux name="a" x="100" y="100"/></view>
      <view view_type="stock_flow"><aux name="a" x="200" y="200"/></view>
    </views>
  </model>
</xmile>
)";

}  // namespace

TEST(View_corpus_fishbanks_to_mdl) {
  std::string xmile = ReadFile(std::string(XMUTIL_SRC_ROOT) + "/test/fixtures/simlin/fishbanks.xmile");
  CHECK(!xmile.empty());
  if (xmile.empty())
    return;
  std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(xmile);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(View_corpus_reliability_to_mdl) {
  std::string xmile = ReadFile(std::string(XMUTIL_SRC_ROOT) + "/test/fixtures/simlin/reliability.xmile");
  CHECK(!xmile.empty());
  if (xmile.empty())
    return;
  std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(xmile);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(View_clouds_produce_VensimCommentElement) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kClouds, errs);
  CHECK(m != nullptr);
  CHECK(errs.empty());
  if (!m)
    return;
  CHECK(m->Views().size() == 1);
  VensimView *view = m->Views().empty() ? nullptr : dynamic_cast<VensimView *>(m->Views()[0]);
  CHECK(view != nullptr);
  if (view) {
    int cloudCount = 0;
    for (VensimViewElement *e : view->Elements()) {
      if (e && e->Type() == VensimViewElement::ElementTypeCOMMENT)
        ++cloudCount;
    }
    CHECK(cloudCount == 2);
  }
  delete m;
}

TEST(View_connector_polarity_plus_round_trips) {
  // The comparator (CompareViewElements) explicitly compares the connector
  // Polarity() bytes, so an empty diff vector proves '+' survived
  // XMILE -> MDL -> Vensim re-parse.
  std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(kPolarityConnector);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(View_flow_endpoint_ignores_structurally_unrelated_neighbor) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kDecoyAux, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  VensimView *view = m->Views().empty() ? nullptr : dynamic_cast<VensimView *>(m->Views()[0]);
  CHECK(view != nullptr);
  if (view) {
    // The left endpoint must be a cloud (one VensimCommentElement), not a
    // connector to the decoy aux; the right endpoint still resolves to the
    // structurally-connected stock "s".
    int cloudCount = 0;
    for (VensimViewElement *e : view->Elements()) {
      if (e && e->Type() == VensimViewElement::ElementTypeCOMMENT)
        ++cloudCount;
    }
    CHECK(cloudCount == 1);
  }
  delete m;
}

TEST(View_connector_endpoint_folds_case_and_underscores) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kCaseFoldedConnector, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  bool unresolved = false;
  for (const std::string &e : errs) {
    if (e.find("unresolved") != std::string::npos) {
      unresolved = true;
      printf("  err: %s\n", e.c_str());
    }
  }
  CHECK(!unresolved);
  VensimView *view = m->Views().empty() ? nullptr : dynamic_cast<VensimView *>(m->Views()[0]);
  CHECK(view != nullptr);
  if (view) {
    int connectors = 0;
    for (VensimViewElement *e : view->Elements()) {
      if (e && e->Type() == VensimViewElement::ElementTypeCONNECTOR)
        ++connectors;
    }
    CHECK(connectors == 1);
  }
  delete m;
}

TEST(View_multi_view_takes_first_warns_second) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kMultiView, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  bool sawWarning = false;
  for (const std::string &e : errs) {
    if (e.find("multi-view") != std::string::npos)
      sawWarning = true;
  }
  CHECK(sawWarning);
  CHECK(m->Views().size() == 1);
  delete m;
}

// AC3.4 guard: parse, regen via PrintXMILE, re-parse, then rename a variable in
// the re-parsed model. The comparator MUST report a difference -- if it does
// not, the round-trip tests above could be passing vacuously (the comparator
// would be blind to a real divergence). A deliberately corrupted re-parse is a
// negative control: it tests that the test itself can fail.
TEST(View_comparator_guard_detects_corruption) {
  std::vector<std::string> errs;
  Model *m0 = xmileroundtrip::ParseXMILE(kPolarityConnector, errs);
  CHECK(m0 != nullptr);
  if (!m0)
    return;
  std::vector<std::string> regenErrs;
  std::string regen = m0->PrintXMILE(/*isCompact=*/false, regenErrs, 1.0, 1.0);
  CHECK(regenErrs.empty());
  Model *m1 = xmileroundtrip::ParseXMILE(regen, errs);
  CHECK(m1 != nullptr);
  if (!m1) {
    delete m0;
    return;
  }
  Variable *a = static_cast<Variable *>(m1->GetNameSpace()->Find("a"));
  CHECK(a != nullptr);
  if (a) {
    bool renamed = m1->RenameVariable(a, "z");
    CHECK(renamed);
  }
  std::vector<std::string> diffs = ModelComparator::Compare(m0, m1);
  CHECK(!diffs.empty());
  delete m0;
  delete m1;
}
