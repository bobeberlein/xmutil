// Envelope-level smoke tests for the XMILE reader. These exercise only the
// document envelope: a well-formed empty <xmile> parses successfully, malformed
// XML and a non-<xmile> root both report errors. Variable population is
// exercised by the populated-model test fixtures.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {
const char *kEmptyEnvelope =
    "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\">\n"
    "  <header><name>empty</name></header>\n"
    "  <sim_specs/>\n"
    "  <model/>\n"
    "</xmile>\n";

const char *kMissingRoot = "not even xml";

const char *kWrongRoot = "<not-xmile/>\n";
}  // namespace

TEST(xmile_envelope_parses) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kEmptyEnvelope, errs);
  CHECK(m != nullptr);
  CHECK(errs.empty());
  delete m;
}

TEST(xmile_non_xml_input_errors) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kMissingRoot, errs);
  CHECK(m == nullptr);
  CHECK(!errs.empty());
}

TEST(xmile_wrong_root_errors) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kWrongRoot, errs);
  CHECK(m == nullptr);
  CHECK(!errs.empty());
}

TEST(xmile_empty_envelope_round_trip) {
  // Verifies the parse->print->parse round-trip on an empty envelope does not
  // crash and surfaces no errors; structural checks are exercised by
  // populated-model tests.
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kEmptyEnvelope);
  for (const std::string &d : diffs)
    printf("  unexpected diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}
