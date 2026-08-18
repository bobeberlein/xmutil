// Dynamo -> Vensim conversion round-trip (Phase 7, Task 3; verifies
// mdl-writer.AC5.1).
//
// A hand-authored classic-Dynamo model (test/mdl/fixtures/minimal.dyn: a level,
// a rate, an aux, a constant, and a SPEC card) is parsed with DynamoParse (M0),
// emitted as Vensim .mdl via Model::PrintMDL, and re-parsed with VensimParse
// (M1). The two models must compare equal -- same variables, equation ASTs,
// types, and sim specs. DynamoParse synthesizes no sketch, so there must be no
// view-related diffs either (the comparator ignores the minimal empty view the
// Vensim re-parse adds).
//
// The .dyn fixture is read from disk via XMUTIL_SRC_ROOT (the repo root injected
// by XMUtil.gyp) so the fixture file genuinely exists and is exercised, rather
// than being inlined. Model.h is included so each Model* the round-trip helper
// hands back is a COMPLETE type at its delete site (avoids -Wdelete-incomplete).

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/Symbol/Variable.h"
#include "../TestHarness.h"
#include "ModelComparator.h"
#include "RoundTrip.h"

namespace {

std::string ReadFixture(const std::string &relative_path, bool &found) {
  std::string absolute = std::string(XMUTIL_SRC_ROOT) + "/" + relative_path;
  std::ifstream in(absolute, std::ios::binary);
  if (!in) {
    found = false;
    return std::string();
  }
  found = true;
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

Variable *FindVariable(Model *m, const char *name) {
  Symbol *s = m->GetNameSpace()->Find(name);
  if (s && s->isType() == Symtype_Variable)
    return static_cast<Variable *>(s);
  return nullptr;
}

}  // namespace

// AC5.1: the minimal .dyn fixture converts to a valid .mdl that re-parses to an
// equivalent Model. The conversion is verified non-vacuously: M0 is first
// confirmed to actually contain a STOCK (Population) fed by a FLOW (Births)
// driven by an AUX (frac), so the equivalence assertion is about a real
// stock/flow/aux model rather than an empty parse.
TEST(DynamoToMdl_minimal_round_trips) {
  bool found = false;
  std::string dyn = ReadFixture("test/mdl/fixtures/minimal.dyn", found);
  CHECK(found);
  if (!found)
    return;

  // Non-vacuity: parse the .dyn and confirm the model the conversion will act on
  // is the intended stock/flow/aux, not a derailed parse.
  Model *m0 = roundtrip::ParseDynamo(dyn);
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  Variable *population = FindVariable(m0, "Population");
  Variable *births = FindVariable(m0, "Births");
  Variable *frac = FindVariable(m0, "frac");
  CHECK(population != nullptr);
  CHECK(births != nullptr);
  CHECK(frac != nullptr);
  if (population) {
    // The level became a stock. The Dynamo `level = level + DT*rate` form is not
    // a clean +/- of named flows, so the reader synthesizes a single
    // "Population net flow" inflow (the rate `Births` stays an aux referenced
    // inside that synthetic flow), leaving Population's inflow set non-empty.
    CHECK(population->VariableType() == XMILE_Type_STOCK);
    CHECK(population->Inflows().size() == 1);
    // The init value lives in a separate `N` init equation (the Dynamo
    // convention), distinct from the active INTEGRATE equation -- this is the
    // structure the writer must read the initial value back out of.
    CHECK(!population->GetAllInitEquations().empty());
  }
  // The rate and the constant are auxiliaries (Births is referenced inside the
  // synthetic net flow rather than being a flow itself).
  if (births)
    CHECK(births->VariableType() == XMILE_Type_AUX);
  if (frac)
    CHECK(frac->VariableType() == XMILE_Type_AUX);

  // The emitted .mdl is a real, non-empty Vensim document.
  std::vector<std::string> errs;
  std::string mdl = m0->PrintMDL(errs);
  CHECK(errs.empty());
  CHECK(!mdl.empty());
  CHECK(mdl.rfind("{UTF-8}", 0) == 0);

  // The emission re-parses with the Vensim reader (the .mdl is valid).
  Model *m1 = roundtrip::ParseVensim(mdl);
  CHECK(m1 != nullptr);

  // No view diffs despite Dynamo having no sketch: the comparator drops empty
  // views, so the minimal frame the Vensim re-parse adds does not register.
  if (m1) {
    std::vector<std::string> view_diffs;
    for (const std::string &d : ModelComparator::Compare(m0, m1)) {
      if (d.rfind("view", 0) == 0)
        view_diffs.push_back(d);
    }
    for (const std::string &d : view_diffs)
      printf("  unexpected view diff: %s\n", d.c_str());
    CHECK(view_diffs.empty());
    delete m1;
  }

  delete m0;

  // The decisive AC5.1 assertion: the full Dynamo->Vensim->re-parse comparison is
  // clean (same variables, equation ASTs, types, sim specs; no sketch diffs).
  std::vector<std::string> diffs = roundtrip::DynamoToMdlDiffs(dyn);
  for (const std::string &d : diffs)
    printf("  dynamo->mdl diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}
