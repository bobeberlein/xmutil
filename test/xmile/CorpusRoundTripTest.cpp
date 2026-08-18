// Phase 8 corpus-scale validation for the XMILE reader.
//
// Exercises all three simlin default-project corpus models -- fishbanks,
// logistic-growth, reliability -- in both directions (XMILE -> XMILE and
// XMILE -> MDL). The Phase 4 and Phase 7 test files cover individual cells
// of this matrix; this file's role is to make the full matrix one cohesive
// pass so a future regression in any cell surfaces here even if the
// phase-scoped test files are reorganized.
//
// The XMILE -> XMILE direction exercises the reader -> Model::PrintXMILE ->
// reader round trip; the XMILE -> MDL direction exercises reader ->
// Model::PrintMDL -> VensimParse re-parse. Both compare via ModelComparator
// and pass iff the diff list is empty.

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/Model.h"
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

const char *kFishbanksPath = "/test/fixtures/simlin/fishbanks.xmile";
const char *kLogisticGrowthPath = "/test/fixtures/simlin/logistic-growth.xmile";
const char *kReliabilityPath = "/test/fixtures/simlin/reliability.xmile";

}  // namespace

TEST(Corpus_fishbanks_xmile_to_xmile) {
  std::string xmile = ReadFile(std::string(XMUTIL_SRC_ROOT) + kFishbanksPath);
  CHECK(!xmile.empty());
  if (xmile.empty())
    return;
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(xmile);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Corpus_logistic_growth_xmile_to_xmile) {
  std::string xmile = ReadFile(std::string(XMUTIL_SRC_ROOT) + kLogisticGrowthPath);
  CHECK(!xmile.empty());
  if (xmile.empty())
    return;
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(xmile);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Corpus_reliability_xmile_to_xmile) {
  std::string xmile = ReadFile(std::string(XMUTIL_SRC_ROOT) + kReliabilityPath);
  CHECK(!xmile.empty());
  if (xmile.empty())
    return;
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(xmile);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Corpus_fishbanks_xmile_to_mdl) {
  std::string xmile = ReadFile(std::string(XMUTIL_SRC_ROOT) + kFishbanksPath);
  CHECK(!xmile.empty());
  if (xmile.empty())
    return;
  std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(xmile);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Corpus_logistic_growth_xmile_to_mdl) {
  std::string xmile = ReadFile(std::string(XMUTIL_SRC_ROOT) + kLogisticGrowthPath);
  CHECK(!xmile.empty());
  if (xmile.empty())
    return;
  std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(xmile);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Corpus_reliability_xmile_to_mdl) {
  std::string xmile = ReadFile(std::string(XMUTIL_SRC_ROOT) + kReliabilityPath);
  CHECK(!xmile.empty());
  if (xmile.empty())
    return;
  std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(xmile);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}
