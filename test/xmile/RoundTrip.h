#pragma once
#include <string>
#include <vector>
class Model;

namespace xmileroundtrip {
// Parse XMILE text in-memory and run the same post-parse pipeline the extern-C
// drivers use (MarkVariableTypes main + macros, AdjustGroupNames,
// CheckGhostOwners) -- the same sequence convert_xmile_to_xmile uses
// (src/XMUtil.cpp). Returns a heap Model the caller owns, or nullptr on parse
// failure; failure messages are pushed into errs.
Model *ParseXMILE(const std::string &text, std::vector<std::string> &errs);

// Parse -> PrintXMILE -> re-parse; returns the differences from
// ModelComparator. On any parse or generation failure, returns a single-element
// vector describing the failure.
std::vector<std::string> RoundTripDiffs(const std::string &xmileText);

// Run one XMILE -> Model -> XMILE normalization: parse, run the post-parse
// pipeline, and PrintXMILE. Returns the emitted document, or an empty string
// with the reason pushed into errs.
std::string NormalizeXMILE(const std::string &xmileText, std::vector<std::string> &errs);

// The normalizer's fixpoint property: normalizing a document the normalizer
// itself produced must reproduce it BYTE FOR BYTE. Structural round-trip
// equality (RoundTripDiffs) does not imply this -- a writer that adds a
// redundant paren layer or permutes its variable order each pass still compares
// structurally equal every time, yet never converges. Returns an empty string
// when the property holds, otherwise a description naming the first line that
// differs between the first and second emission.
std::string FixpointFailure(const std::string &xmileText);

// XMILE -> Vensim cross-conversion check: parse XMILE text (M0), emit it as
// Vensim .mdl via PrintMDL, re-parse the emission with VensimParse (M1), and
// return the ModelComparator differences. On any parse or generation failure,
// returns a single-element vector describing the failure.
std::vector<std::string> XmileToMdlDiffs(const std::string &xmileText);
}  // namespace xmileroundtrip
