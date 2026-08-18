// Coverage for the "silent wrong results" hardening on the XMILE reader:
//   - unknown multi-argument function calls fail loudly (not lowered to 0);
//   - a single-argument lookup application on a never-defined target fails as a
//     phantom, while a forward-referenced graphical function still parses;
//   - a fixed-arity builtin called outside its accepted argument-count range
//     fails loudly and names the function (an emitted wrong-arity call would be
//     rejected by Vensim);
//   - a dropped <non_negative> clamp is reported as a one-line advisory that
//     does not fail the conversion.
//
// These exercise the equation-diagnostics classification (a "warning: " prefix
// separates non-fatal advisories from hard errors) and the post-parse
// lookup-target validation added to XmileReader.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

// Parse expecting FAILURE: m == nullptr and some diagnostic contains substr.
void ExpectParseFails(const char *xmile, const char *substr) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(xmile, errs);
  CHECK(m == nullptr);
  bool matched = false;
  for (const std::string &e : errs)
    if (e.find(substr) != std::string::npos)
      matched = true;
  if (!matched) {
    printf("  expected substring not found: %s\n", substr);
    for (const std::string &e : errs)
      printf("  err: %s\n", e.c_str());
  }
  CHECK(matched);
  delete m;
}

bool AnyContains(const std::vector<std::string> &errs, const char *substr) {
  for (const std::string &e : errs)
    if (e.find(substr) != std::string::npos)
      return true;
  return false;
}

const char *kUnknownMultiArg = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="y"><eqn>weirdfunc(1, 2)</eqn></aux>
  </variables></model>
</xmile>
)";

const char *kPhantomSingleArg = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="x"><eqn>5</eqn></aux>
    <aux name="y"><eqn>undefined_table(x)</eqn></aux>
  </variables></model>
</xmile>
)";

// A graphical function referenced (as name(x)) BEFORE it is declared later in
// the document: a legal forward reference that must still parse. This pins the
// contract that the phantom-lookup validation does not over-reject.
const char *kForwardGf = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="x"><eqn>5</eqn></aux>
    <aux name="y"><eqn>my_gf(x)</eqn></aux>
    <gf name="my_gf"><xpts>0, 10</xpts><ypts>0, 100</ypts></gf>
  </variables></model>
</xmile>
)";

// A fixed-arity builtin called with too many arguments. This is now a hard
// error: the emitted call would be rejected by Vensim, so the conversion must
// fail loudly rather than convert with exit 0.
const char *kArityTooMany = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="y"><eqn>sin(1, 2)</eqn></aux>
  </variables></model>
</xmile>
)";

// A non-negative flow and stock, plus a flow whose clamp is explicitly off.
const char *kNonNegative = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <flow name="clamped flow"><eqn>1</eqn><non_negative/></flow>
    <flow name="open flow"><eqn>1</eqn><non_negative>false</non_negative></flow>
    <stock name="clamped stock"><inflow>clamped flow</inflow><inflow>open flow</inflow>
      <eqn>0</eqn><non_negative/></stock>
  </variables></model>
</xmile>
)";

}  // namespace

// Fix 1a: an unknown multi-argument call must abort the conversion and name the
// function, never silently lower to a 0 literal.
TEST(Diagnostics_unknown_multi_arg_function_fails_and_names_it) {
  ExpectParseFails(kUnknownMultiArg, "unknown function 'weirdfunc'");
}

// Fix 1b: a single-argument application of a never-defined target is a phantom
// lookup and must fail, naming the offending variable.
TEST(Diagnostics_phantom_single_arg_lookup_fails_and_names_it) {
  ExpectParseFails(kPhantomSingleArg, "undefined_table");
}

// Fix 1b (positive): a graphical function applied before its <gf> declaration is
// a legal forward reference and must still parse cleanly.
TEST(Diagnostics_forward_referenced_gf_applied_before_definition_parses) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kForwardGf, errs);
  CHECK(m != nullptr);
  delete m;
}

// A wrong-arity fixed-arity builtin now aborts the conversion and names the
// function plus the got-count, rather than converting with a swallowed advisory.
TEST(Diagnostics_wrong_arity_fails_and_names_function) {
  ExpectParseFails(kArityTooMany, "SIN");
}

// Fix 3: dropped <non_negative> clamps produce one advisory per clamped variable
// (naming it) and never fail the conversion; a clamp turned off emits nothing.
TEST(Diagnostics_non_negative_clamp_advisories) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kNonNegative, errs);
  CHECK(m != nullptr);
  CHECK(AnyContains(errs, "non_negative"));
  CHECK(AnyContains(errs, "clamped flow"));
  CHECK(AnyContains(errs, "clamped stock"));
  // The clamp explicitly turned off must not produce an advisory.
  CHECK(!AnyContains(errs, "'open flow'"));
  delete m;
}
