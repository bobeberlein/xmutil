// Phase 8 rejection-path coverage for the XMILE reader.
//
// Pins down AC5.1 (<module> rejected), AC5.2 (<macro> rejected), AC5.3
// (multi-<model> rejected), AC2.9 (postfix ' rejected at document scope), and
// AC1.5 (extern-C entries return NULL on failure). The Phase 4
// AuxRoundTrip_module_is_rejected and the Phase 2 equation-parser apostrophe
// test cover the same ground at lower levels; the duplicate coverage here is
// deliberate so a future refactor of the equation parser or envelope walker
// cannot silently let a previously-rejected shape through.

#include <cstring>
#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/XMUtil.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

const char *kWithModule = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="x"><eqn>1</eqn></aux>
    <module name="m1"/>
  </variables></model>
</xmile>
)";

const char *kWithMacro = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <macro name="my_macro"><eqn>x*2</eqn></macro>
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables><aux name="x"><eqn>1</eqn></aux></variables></model>
</xmile>
)";

const char *kMultiModel = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables><aux name="x"><eqn>1</eqn></aux></variables></model>
  <model><variables><aux name="y"><eqn>2</eqn></aux></variables></model>
</xmile>
)";

const char *kPostfixApostrophe = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="x"><eqn>a'</eqn></aux>
  </variables></model>
</xmile>
)";

// A bare `*` wildcard on a scalar (non-arrayed) target: the reference parses,
// but there is no dimension family to bind the `*` to, so neither writer can
// render valid output. ResolveWildcardSubscripts (run inside the post-parse
// pipeline) must record the failure and both Print paths must reject.
const char *kWildcardOnScalar = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="x"><eqn>1</eqn></aux>
    <aux name="y"><eqn>SUM(x[*])</eqn></aux>
  </variables></model>
</xmile>
)";

void CheckRejected(const char *xmile, const char *expectedSubstring) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(xmile, errs);
  CHECK(m == nullptr);
  CHECK(!errs.empty());
  if (!expectedSubstring || !*expectedSubstring)
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
}

}  // namespace

TEST(ErrorReject_module_with_clear_message) {
  CheckRejected(kWithModule, "modules are not supported");
}

TEST(ErrorReject_macro_with_clear_message) {
  CheckRejected(kWithMacro, "macro");
}

TEST(ErrorReject_multiple_models) {
  CheckRejected(kMultiModel, "multiple <model>");
}

// AC2.9 regression check at the document-level entry point. Phase 2 covers the
// same rejection at the equation parser layer (XmileParse_postfix_apostrophe_-
// errors in EquationParseTest.cpp); the duplicate here guards against a future
// refactor that bypasses the grammar from the XMILE side and silently lets the
// postfix-apostrophe shape through.
TEST(ErrorReject_postfix_apostrophe_in_equation) {
  CheckRejected(kPostfixApostrophe, "");
}

// A wildcard that cannot be bound parses cleanly but must fail at write time
// rather than shipping a literal `*` (invalid MDL, un-reparseable XMILE).
TEST(ErrorReject_unbound_wildcard_fails_both_writers) {
  std::vector<std::string> parseErrs;
  Model *m = xmileroundtrip::ParseXMILE(kWildcardOnScalar, parseErrs);
  CHECK(m != nullptr);  // parse + pipeline succeed; the defect surfaces at write time
  CHECK(!m->UnresolvedWildcards().empty());

  std::vector<std::string> mdlErrs;
  std::string mdl = m->PrintMDL(mdlErrs);
  CHECK(mdl.empty());
  CHECK(!mdlErrs.empty());

  std::vector<std::string> xmileErrs;
  std::string xmile = m->PrintXMILE(/*isCompact=*/false, xmileErrs, 1.0, 1.0);
  CHECK(xmile.empty());
  CHECK(!xmileErrs.empty());

  delete m;
}

// The extern-C entries must return NULL (not invalid text) for the same model.
TEST(ErrorReject_unbound_wildcard_extern_c_returns_null) {
  char *mdl = convert_xmile_to_mdl(kWildcardOnScalar, std::strlen(kWildcardOnScalar), "test.xmile", -1);
  CHECK(mdl == nullptr);
  char *xmile = convert_xmile_to_xmile(kWildcardOnScalar, std::strlen(kWildcardOnScalar), "test.xmile", -1, false);
  CHECK(xmile == nullptr);
}

TEST(ErrorReject_convert_xmile_to_xmile_returns_null_on_error) {
  char *out = convert_xmile_to_xmile(kWithModule, std::strlen(kWithModule), "test.xmile", -1, false);
  CHECK(out == nullptr);
}

TEST(ErrorReject_convert_xmile_to_mdl_returns_null_on_error) {
  char *out = convert_xmile_to_mdl(kWithModule, std::strlen(kWithModule), "test.xmile", -1);
  CHECK(out == nullptr);
}
