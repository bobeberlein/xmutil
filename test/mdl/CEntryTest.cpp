// C-entry contract checks (Phase 7, Task 2).
//
// Exercises the stable C API the JS/native callers use -- convert_to_mdl in
// src/XMUtil.h -- and the Model::PrintMDL error channel directly, verifying the
// ownership and success/failure contract (mdl-writer.AC1.2, AC1.4).
//
// convert_to_mdl returns a strdup'd, caller-owned C string (freed with free(),
// not delete) or NULL. Model.h is included so the Model* the round-trip helper
// hands back is a COMPLETE type at its delete site (avoids -Wdelete-incomplete).

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/XMUtil.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

// A tiny but complete model: a stock fed by a flow, the flow's constant rate,
// and explicit control vars. Enough that the emitted .mdl carries real content
// (so "re-parses to something" is a meaningful assertion, not a check that an
// empty shell re-parses).
const char *const kValidModel =
    "{UTF-8}\r\n"
    "Stock = INTEG( Flow, 10)\r\n"
    "\t~\twidgets\r\n"
    "\t~\tA stock.\r\n"
    "\t|\r\n"
    "Flow = Rate\r\n"
    "\t~\twidgets/Month\r\n"
    "\t~\tThe inflow.\r\n"
    "\t|\r\n"
    "Rate = 5\r\n"
    "\t~\twidgets/Month\r\n"
    "\t~\tConstant rate.\r\n"
    "\t|\r\n"
    "INITIAL TIME = 0\r\n\t~~|\r\n"
    "FINAL TIME = 100\r\n\t~~|\r\n"
    "TIME STEP = 1\r\n\t~~|\r\n"
    "SAVEPER = 1\r\n\t~~|\r\n";

}  // namespace

// AC1.2 (success): convert_to_mdl on a valid model returns a non-null,
// caller-owned string that begins with the {UTF-8} marker and that re-parses to
// a usable Model. The returned buffer is strdup'd, so it is released with
// free() (NOT delete) -- the contract this test pins down for callers.
//
// Non-vacuity: the input has a real stock/flow/aux, and the test confirms the
// re-parsed Model actually contains them, so "the emitted .mdl re-parses" is a
// statement about a non-trivial model rather than an empty shell.
TEST(CEntry_convert_to_mdl_success) {
  std::string text = kValidModel;
  char *r = convert_to_mdl(text.c_str(), static_cast<uint32_t>(text.size()), "m.mdl", 0);
  CHECK(r != nullptr);
  if (!r)
    return;

  // Begins with the UTF-8 marker (rfind at offset 0 == 0 means "starts with").
  CHECK(std::string(r).rfind("{UTF-8}", 0) == 0);

  // The returned string re-parses to a usable Model.
  Model *reparsed = roundtrip::ParseVensim(r);
  CHECK(reparsed != nullptr);
  if (reparsed) {
    // Non-vacuity: the round-tripped model carries the variables the input
    // defined, so the success above is about a real model, not an empty shell.
    Symbol *stock = reparsed->GetNameSpace()->Find("Stock");
    Symbol *flow = reparsed->GetNameSpace()->Find("Flow");
    Symbol *rate = reparsed->GetNameSpace()->Find("Rate");
    CHECK(stock && stock->isType() == Symtype_Variable);
    CHECK(flow && flow->isType() == Symtype_Variable);
    CHECK(rate && rate->isType() == Symtype_Variable);
    delete reparsed;
  }

  // The C entry hands ownership to the caller; it was allocated with strdup, so
  // it is released with free(), not delete.
  free(r);
}

// Input-format dispatch is by file extension, and the C API documents that a
// bare file name is acceptable. The shortest legal Dynamo name ("x.dyn", five
// characters) must still route to the Dynamo reader -- a length-based guard
// (`strlen > 5`) silently fed Dynamo source to the Vensim parser, which
// "recovers" into a degenerate empty model instead of failing loudly.
TEST(CEntry_short_dyn_filename_routes_to_dynamo_parser) {
  std::string absolute = std::string(XMUTIL_SRC_ROOT) + "/test/mdl/fixtures/minimal.dyn";
  std::ifstream in(absolute, std::ios::binary);
  CHECK(static_cast<bool>(in));
  if (!in)
    return;
  std::ostringstream ss;
  ss << in.rdbuf();
  std::string dyn = ss.str();

  char *r = convert_to_mdl(dyn.c_str(), static_cast<uint32_t>(dyn.size()), "x.dyn", 0);
  CHECK(r != nullptr);
  if (!r)
    return;

  // Population only appears in the output if the Dynamo reader actually ran;
  // the misrouted Vensim parse produced an empty {UTF-8} shell.
  CHECK(std::string(r).find("Population") != std::string::npos);
  free(r);
}

// AC1.4 (error channel): Model::PrintMDL(errs) is the generation entry point the
// C entry routes through. For a well-formed parsed Model it returns a non-empty
// string and leaves `errs` empty.
//
// VERIFIED LIMITATION (documented, not asserted-away): the writer does not
// currently surface any generation error for a parsed Model -- MDLGenerator::Print
// takes no errs at all (src/Mdl/MDLGenerator.h); Model::PrintMDL populates errs
// only from its pre-generation checks (unresolved wildcards). The `errs` channel
// exists for forward-compatibility, mirroring convert_mdl_to_xmile's identical
// errs handling (src/XMUtil.cpp). This test pins the present contract: errs stays
// empty and the output is real, so a future change that started populating errs
// (and thus making convert_to_mdl return NULL) would be a deliberate, visible
// break of this test rather than a silent behavior change.
TEST(CEntry_print_mdl_error_channel_empty_for_wellformed) {
  Model *m = roundtrip::ParseVensim(kValidModel);
  CHECK(m != nullptr);
  if (!m)
    return;

  std::vector<std::string> errs;
  std::string mdl = m->PrintMDL(errs);

  // Non-vacuous: real, non-empty output that begins with the UTF-8 marker.
  CHECK(!mdl.empty());
  CHECK(mdl.rfind("{UTF-8}", 0) == 0);
  // The forward-compat error channel is empty for a well-formed model.
  CHECK(errs.empty());

  delete m;
}

// AC1.4 (NULL-return contract): convert_to_mdl returns NULL iff the parse step
// reports failure (`if (!ProcessFile(...)) return nullptr;`, src/XMUtil.cpp) or
// PrintMDL populates `errs`. The success path is the contrapositive of that
// contract and is the non-vacuous lever this test pulls: a model that parses
// MUST yield non-null.
//
// VERIFIED GAP vs. the phase file's example: the phase file suggested
// `convert_to_mdl("@#$ not a model %^&")` returns NULL. It does NOT. The Vensim
// reader is recovery-oriented -- VensimParse::ProcessFile logs a syntax error,
// discards the unparseable equation, and still returns true (the `return false`
// branch sits under an `if (true)` and is unreachable). So garbage input yields
// a degenerate-but-valid {UTF-8} shell (the .Control group at default sim specs),
// not NULL. This test asserts that VERIFIED behavior rather than the incorrect
// guess: the call returns non-null and a parseable shell. The genuine NULL path
// is therefore unreachable from input alone today; it is reached only if a future
// parser/writer change starts reporting failure, which this and the test above
// would then catch.
TEST(CEntry_recovery_oriented_reader_does_not_null_on_garbage) {
  const char *garbage = "@#$ not a model %^&";
  char *r = convert_to_mdl(garbage, static_cast<uint32_t>(strlen(garbage)), "m.mdl", 0);

  // Verified behavior: non-null degenerate shell, not NULL.
  CHECK(r != nullptr);
  if (!r)
    return;
  CHECK(std::string(r).rfind("{UTF-8}", 0) == 0);

  // Non-vacuity: the degenerate output still parses (it is a real, if empty,
  // model), so the non-null result is a usable .mdl, not random bytes.
  Model *reparsed = roundtrip::ParseVensim(r);
  CHECK(reparsed != nullptr);
  if (reparsed)
    delete reparsed;

  free(r);
}
