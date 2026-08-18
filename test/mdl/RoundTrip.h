#pragma once
#include <string>
#include <vector>
class Model;

namespace roundtrip {
// Parse Vensim .mdl text in-memory and run the full post-parse pipeline
// (MarkVariableTypes main + macros, AdjustGroupNames, CheckGhostOwners) -- the
// same sequence convert_to_mdl uses (src/XMUtil.cpp). Returns a heap Model the
// caller owns, or nullptr on parse failure.
Model *ParseVensim(const std::string &text);

// Parse Dynamo .dyn text in-memory and run the same post-parse pipeline.
Model *ParseDynamo(const std::string &text);

// Parse -> PrintMDL -> re-parse; returns the differences from ModelComparator.
// On any parse or generation failure, returns a single-element vector
// describing the failure.
std::vector<std::string> RoundTripDiffs(const std::string &mdlText);

// Run RoundTripDiffs and assert (via the test-harness CHECK) that the diff
// list is empty, printing any diffs first so a failure is self-explanatory.
// RoundTripDiffs also reports parse/generation failures as a non-empty list,
// so this single assertion covers those too.
void ExpectCleanRoundTrip(const std::string &mdl);

// Dynamo -> Vensim cross-conversion check (mdl-writer.AC5.1): parse .dyn text
// (M0, no views), emit it as Vensim .mdl via PrintMDL, re-parse the emission
// (M1), and return the ModelComparator differences. M0 has no sketch (DynamoParse
// synthesizes none) and the comparator ignores the minimal empty view the Vensim
// re-parse adds, so a clean conversion yields an empty diff list. On any parse or
// generation failure, returns a single-element vector describing the failure.
std::vector<std::string> DynamoToMdlDiffs(const std::string &dynText);
}  // namespace roundtrip
