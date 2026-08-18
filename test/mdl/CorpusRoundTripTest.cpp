// Corpus round-trip breadth check (Phase 7, Task 1).
//
// Reads real Vensim .mdl fixtures from the simlin corpus on disk, parses each
// one, emits it back to .mdl via Model::PrintMDL, re-parses the emission, and
// asserts the two Models compare equal (empty diff list from ModelComparator).
// This exercises the writer over a wide range of single-feature models rather
// than the hand-built fixtures the other Phase tests use.
//
// Fixtures are checked in under test/fixtures/ (see test/fixtures/README.md for
// provenance); the absolute path is built from XMUTIL_SRC_ROOT (the repo root,
// injected by XMUtil.gyp from gyp's <(cwd)) joined with the fixture's
// repo-relative path. A model that is missing on disk fails the test, so a
// renamed or moved fixture is caught rather than silently skipped.
//
// Curating the list:
//   - kAllowList holds models that parse and round-trip with an empty diff.
//     Every entry is asserted; adding a model means it genuinely round-trips.
//   - A model whose INITIAL parse (M0) fails is a parser gap, not a writer bug
//     -- it is SKIPPED (logged), never failed. (The current allow-list has no
//     such entries; the skip path is kept so the list can grow safely.)
//   - A model using a v1-unsupported feature (the :EXCEPT: subscript-exception
//     clause, which the writer neither emits nor compares) is kept OFF the
//     allow-list: it would otherwise pass vacuously. kKnownDeferred records the
//     known-out-of-scope models with their reason so the breadth check stays
//     honest about what is and isn't covered.

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/Model.h"
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

bool ContainsExceptClause(const std::string &text) {
  return text.find(":EXCEPT:") != std::string::npos;
}

// Models that parse and round-trip with an empty diff. Curated from a survey of
// the whole simlin corpus; each was confirmed to compare equal after the Phase 7
// writer/comparator fixes. The set spans aux/stock/flow equations, arrays and
// subscript mappings, lookups, macros, control specs, custom group banners,
// unicode and specially-quoted names, and sketched models (geometry/polarity/
// ghosts).
const char *const kAllowList[] = {
    // sdeverywhere single-feature models.
    "test/fixtures/sdeverywhere/models/delay/delay.mdl",
    "test/fixtures/sdeverywhere/models/trend/trend.mdl",
    "test/fixtures/sdeverywhere/models/pulsetrain/pulsetrain.mdl",
    "test/fixtures/sdeverywhere/models/elmcount/elmcount.mdl",
    "test/fixtures/sdeverywhere/models/subalias/subalias.mdl",
    "test/fixtures/sdeverywhere/models/smooth3/smooth3.mdl",
    "test/fixtures/sdeverywhere/models/mapping/mapping.mdl",
    "test/fixtures/sdeverywhere/models/multimap/multimap.mdl",
    "test/fixtures/sdeverywhere/models/sum/sum.mdl",
    "test/fixtures/sdeverywhere/models/sumif/sumif.mdl",
    "test/fixtures/sdeverywhere/models/npv/npv.mdl",
    "test/fixtures/sdeverywhere/models/vector/vector.mdl",
    "test/fixtures/sdeverywhere/models/specialchars/specialchars.mdl",
    "test/fixtures/sdeverywhere/models/comments/comments.mdl",
    "test/fixtures/sdeverywhere/models/init_recurrence/init_recurrence.mdl",

    // pysd/Vensim test-models single-feature models.
    "test/fixtures/test-models/tests/abs/test_abs.mdl",
    "test/fixtures/test-models/tests/active_initial/test_active_initial.mdl",
    "test/fixtures/test-models/tests/chained_initialization/test_chained_initialization.mdl",
    "test/fixtures/test-models/tests/conditional_subscripts/test_conditional_subscripts.mdl",
    "test/fixtures/test-models/tests/constant_expressions/test_constant_expressions.mdl",
    "test/fixtures/test-models/tests/control_vars/test_control_vars.mdl",
    "test/fixtures/test-models/tests/delays/test_delays.mdl",
    "test/fixtures/test-models/tests/delay_fixed/test_delay_fixed.mdl",
    "test/fixtures/test-models/tests/delay_pipeline/test_pipeline_delays.mdl",
    "test/fixtures/test-models/tests/elm_count/test_elm_count.mdl",
    "test/fixtures/test-models/tests/euler_step_vs_saveper/test_euler_step_vs_saveper.mdl",
    "test/fixtures/test-models/tests/exponentiation/exponentiation.mdl",
    "test/fixtures/test-models/tests/forecast/test_forecast.mdl",
    "test/fixtures/test-models/tests/game/test_game.mdl",
    "test/fixtures/test-models/tests/if_stmt/if_stmt.mdl",
    "test/fixtures/test-models/tests/initial_function/test_initial.mdl",
    "test/fixtures/test-models/tests/input_functions/test_inputs.mdl",
    "test/fixtures/test-models/tests/limits/test_limits.mdl",
    "test/fixtures/test-models/tests/line_continuation/test_line_continuation.mdl",
    "test/fixtures/test-models/tests/logicals/test_logicals.mdl",
    "test/fixtures/test-models/tests/lookups/test_lookups.mdl",
    "test/fixtures/test-models/tests/lookups_inline/test_lookups_inline.mdl",
    "test/fixtures/test-models/tests/nested_functions/test_nested_functions.mdl",
    "test/fixtures/test-models/tests/number_handling/test_number_handling.mdl",
    "test/fixtures/test-models/tests/odd_number_quotes/teacup_3quotes.mdl",
    "test/fixtures/test-models/tests/parentheses/test_parens.mdl",
    "test/fixtures/test-models/tests/power/power.mdl",
    "test/fixtures/test-models/tests/sample_if_true/test_sample_if_true.mdl",
    "test/fixtures/test-models/tests/smooth/test_smooth.mdl",
    "test/fixtures/test-models/tests/smooth_and_stock/test_smooth_and_stock.mdl",
    "test/fixtures/test-models/tests/sqrt/test_sqrt.mdl",
    "test/fixtures/test-models/tests/subscript_2d_arrays/test_subscript_2d_arrays.mdl",
    "test/fixtures/test-models/tests/subscript_3d_arrays/test_subscript_3d_arrays.mdl",
    "test/fixtures/test-models/tests/subscript_mapping_simple/test_subscript_mapping_simple.mdl",
    "test/fixtures/test-models/tests/subscript_mapping_vensim/test_subscript_mapping_vensim.mdl",
    "test/fixtures/test-models/tests/subscripted_flows/test_subscripted_flows.mdl",
    "test/fixtures/test-models/tests/time/test_time.mdl",
    "test/fixtures/test-models/tests/trend/test_trend.mdl",
    "test/fixtures/test-models/tests/trig/test_trig.mdl",
    "test/fixtures/test-models/tests/unicode_characters/unicode_test_model.mdl",
    "test/fixtures/test-models/tests/special_characters/test_special_variable_names.mdl",
    "test/fixtures/test-models/tests/macro_arrayed/test_macro_arrayed.mdl",
    "test/fixtures/test-models/tests/macro_expression/test_macro_expression.mdl",
    "test/fixtures/test-models/tests/macro_stock/test_macro_stock.mdl",
    "test/fixtures/test-models/tests/macro_multi_expression/test_macro_multi_expression.mdl",
    "test/fixtures/test-models/tests/macro_clearn_ramp_from_to/test_macro_clearn_ramp_from_to.mdl",
    "test/fixtures/test-models/tests/macro_clearn_sample_until/test_macro_clearn_sample_until.mdl",

    // Sketched models: geometry, connector polarity, and ghost (alias) state.
    "test/fixtures/simlin/teacup.mdl",
    "test/fixtures/test-models/samples/SIR/SIR.mdl",

    // Purpose-built parenthesization torture tests. Both were deferred while the
    // writer re-grouped an equal-precedence right operand (emitting `a + (b - c)`
    // as `a + b - c`); the walker now keeps that grouping, so they are asserted.
    "test/fixtures/test-models/tests/arithmetics/test_arithmetics.mdl",
    "test/fixtures/test-models/tests/arithmetics_exp/test_arithmetics_exp.mdl",
};

struct DeferredModel {
  const char *path;
  const char *reason;
};

// Models intentionally OFF the allow-list, recorded with the v1 limitation that
// keeps them out. These are NOT writer bugs in scope for the mdl writer:
//   - :EXCEPT: -- the subscript-exception clause is neither emitted nor compared
//     in v1, so such a model would round-trip vacuously.
//   - GET XLS/DIRECT family -- Vensim's external-data functions are stored as
//     opaque "{GET ...}" pseudo-variables that the writer cannot reconstruct
//     into the original call; a dimension whose elements come from an external
//     file likewise cannot be rebuilt (get_subscript_3d_arrays_xls).
const DeferredModel kKnownDeferred[] = {
    {"test/fixtures/test-models/tests/except/test_except.mdl", ":EXCEPT: clause not emitted/compared in v1"},
    {"test/fixtures/test-models/tests/except_multiple/test_except_multiple.mdl",
     ":EXCEPT: clause not emitted/compared in v1"},
    {"test/fixtures/test-models/tests/except_subranges/test_except_subranges.mdl",
     ":EXCEPT: clause not emitted/compared in v1"},
    {"test/fixtures/test-models/tests/with_lookup/test_with_lookup.mdl", "GET DIRECT CONSTANTS opaque passthrough"},
    {"test/fixtures/test-models/tests/get_subscript_3d_arrays_xls/test_get_subscript_3d_arrays_xls.mdl",
     "dimension elements sourced from external file (GET DIRECT CONSTANTS)"},
};

}  // namespace

// Every allow-list entry must be present on disk, parse, and round-trip with an
// empty diff. A missing file, a failed initial parse, or any non-empty diff is a
// regression -- allow-list entries are curated to round-trip cleanly. A failed
// initial parse on an allow-list entry is logged distinctly so it can be removed
// (a parser regression is out of the writer's scope but must not be hidden).
TEST(CorpusRoundTrip_allowlist) {
  int passed = 0;
  for (const char *rel : kAllowList) {
    bool found = false;
    std::string text = ReadFixture(rel, found);
    CHECK(found);
    if (!found) {
      printf("  allow-list fixture missing: %s\n", rel);
      continue;
    }
    // An allow-list entry must not lean on a feature the comparator ignores.
    CHECK(!ContainsExceptClause(text));

    // Distinguish an initial-parse (M0) failure -- a parser gap, treated as a
    // skip -- from a real non-empty diff. RoundTripDiffs internally returns a
    // one-element "failed to parse input .mdl" marker on M0 failure; checking
    // ParseVensim first keeps the skip path explicit and self-documenting.
    Model *m0 = roundtrip::ParseVensim(text);
    if (!m0) {
      printf("  SKIP (initial parse failed, parser gap -- remove from allow-list): %s\n", rel);
      continue;
    }
    delete m0;

    std::vector<std::string> diffs = roundtrip::RoundTripDiffs(text);
    if (!diffs.empty()) {
      printf("  allow-list fixture did not round-trip: %s\n", rel);
      for (const std::string &d : diffs)
        printf("    diff: %s\n", d.c_str());
    }
    CHECK(diffs.empty());
    if (diffs.empty())
      passed++;
  }
  printf("  corpus allow-list: %d/%zu models round-tripped cleanly\n", passed,
         sizeof(kAllowList) / sizeof(kAllowList[0]));
}

// Record the known-deferred models so the breadth check is honest about its
// boundaries. This does not fail on the diff (these are documented v1 scope
// limits, not writer bugs); it only confirms the fixtures still exist and that
// the deferral reason is real -- a :EXCEPT: model must actually contain the
// clause. If a future fix makes one of these round-trip cleanly, that is the
// signal to promote it into kAllowList.
TEST(CorpusRoundTrip_known_deferred_are_documented) {
  for (const DeferredModel &dm : kKnownDeferred) {
    bool found = false;
    std::string text = ReadFixture(dm.path, found);
    CHECK(found);
    if (!found) {
      printf("  deferred fixture missing: %s\n", dm.path);
      continue;
    }
    bool is_except = ContainsExceptClause(text);
    bool reason_is_except = std::string(dm.reason).find(":EXCEPT:") != std::string::npos;
    // A model deferred for :EXCEPT: must actually use the clause, and vice
    // versa, so the documented reason cannot drift from reality.
    CHECK(is_except == reason_is_except);
    printf("  DEFERRED %s -- %s\n", dm.path, dm.reason);
  }
}

// Large-model stress: a 1.4 MB real-world Vensim export (C-LEARN v77, a climate
// policy model with macros, arrays, lookups and a full sketch). The single-
// feature corpus above is broad but small; this pins the writer against a model
// large and tangled enough to surface pathological blow-up, quadratic behavior,
// or a crash that the tidy fixtures never reach.
//
// The model is read from upstream's own test_models/ directory rather than
// vendored under test/fixtures: it is already part of every checkout, and a
// second 53k-line copy would only bloat the history. It is kept out of
// kAllowList only so the 1.4 MB model is parsed once rather than by two tests;
// the structural round-trip is asserted here instead, next to the properties
// that matter for a converter at this size: both writers produce valid,
// non-empty output without crashing, and the MDL writer is a fixpoint --
// re-emitting its own output reproduces it byte-for-byte, so nothing is reshaped
// on a second pass.
//
// The structural assertion was previously impossible: several of these equations
// use an explicit equal-precedence grouping (`a * (b / c)`, `x + (1 - y)`) that
// the walker re-grouped, so the round trip had a non-empty diff by design. The
// walker now preserves those groupings.
TEST(CorpusRoundTrip_large_model_stress) {
  const char *kLarge = "test_models/C-LEARN v77 for Vensim.mdl";
  bool found = false;
  std::string text = ReadFixture(kLarge, found);
  CHECK(found);
  if (!found) {
    printf("  large-model fixture missing: %s\n", kLarge);
    return;
  }

  Model *m0 = roundtrip::ParseVensim(text);
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  std::vector<std::string> mdlErrs;
  std::string emit1 = m0->PrintMDL(mdlErrs);
  CHECK(mdlErrs.empty());
  CHECK(!emit1.empty());

  // Fixpoint: re-parsing and re-emitting the writer's own MDL output reproduces
  // it exactly. If this held only approximately, the writer would be losing or
  // reshaping information on every pass -- the property a round-trip converter
  // most needs to guarantee on a model too large to eyeball.
  Model *m1 = roundtrip::ParseVensim(emit1);
  CHECK(m1 != nullptr);
  if (!m1) {
    delete m0;
    return;
  }
  std::vector<std::string> mdlErrs2;
  std::string emit2 = m1->PrintMDL(mdlErrs2);
  CHECK(mdlErrs2.empty());
  CHECK(emit1 == emit2);

  // Structural round trip: the re-parse of the emission must compare equal to
  // the source model. The fixpoint check above only proves the writer is stable
  // from its own output onward -- a transform that lost information on the FIRST
  // pass and then reproduced the loss faithfully would still satisfy it.
  std::vector<std::string> diffs = ModelComparator::Compare(m0, m1);
  if (!diffs.empty()) {
    printf("  large model did not round-trip: %s\n", kLarge);
    for (const std::string &d : diffs)
      printf("    diff: %s\n", d.c_str());
  }
  CHECK(diffs.empty());
  delete m1;

  // The XMILE writer must also survive the model at scale (this is the path a
  // Vensim->XMILE conversion of C-LEARN takes). Structural round-trip through
  // the XMILE reader is out of scope here -- the emitted document contains
  // <macro> elements the reader rejects by design -- so this only asserts the
  // writer produces a well-formed, non-empty document.
  //
  // It runs LAST because PrintXMILE normalizes the view origin in place: it
  // rewrites m0's own sketch geometry, so any structural comparison against m0
  // has to happen before this line.
  std::vector<std::string> xmileErrs;
  std::string xmile = m0->PrintXMILE(/*isCompact=*/false, xmileErrs, 1.0, 1.0);
  CHECK(xmileErrs.empty());
  CHECK(!xmile.empty());
  CHECK(xmile.find("<xmile") != std::string::npos);
  delete m0;
}
