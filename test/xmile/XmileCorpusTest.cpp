// XMILE corpus round-trip test (waves 1 + 2).
//
// Exercises the .xmile and Stella .stmx models vendored under test/fixtures/
// (provenance in test/fixtures/README.md). Every allow-listed model must
// round-trip cleanly in BOTH directions:
//   - XMILE -> Model -> XMILE -> Model' (xmileroundtrip::RoundTripDiffs)
//   - XMILE -> Model -> MDL -> VensimParse (xmileroundtrip::XmileToMdlDiffs)
// A non-empty ModelComparator diff in either direction fails the test. Wave 1
// covered scalar / builtin / lookup / delay models; wave 2 added arrayed models
// (reference subscripts and bare `*` wildcards now round-trip) and the Stella
// .stmx dialect (legacy 1.0 format, isee: namespace, {...} inline comments).
//
// kKnownDeferred records every candidate model that cannot round-trip yet, each
// with a specific one-line reason (sketch geometry, external-data builtins,
// Stella-dialect gaps, builtin-semantic mismatches). kRejected records the
// module-submodel documents the reader rejects by design and asserts the
// rejection diagnostic. Together the three tables account for every .xmile/.stmx
// model in test/fixtures -- nothing is silently dropped; a missing fixture fails.
//
// XmileCorpus_every_fixture_is_accounted_for enforces that invariant
// mechanically: it walks every *.xmile / *.stmx under test/fixtures/ and fails
// if one is in none of the tables (nor the covered-elsewhere list), so a newly
// added fixture cannot slip through untested.

#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../TestHarness.h"
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

// Models that parse and round-trip cleanly in both directions. Curated from a
// survey of the whole scalar/builtin/lookup/delay corpus; each was confirmed to
// compare equal after the wave-1 reader/writer fixes.
const char *const kAllowList[] = {
    "test/fixtures/sdeverywhere/models/active_initial/active_initial.xmile",
    "test/fixtures/sdeverywhere/models/comments/comments.xmile",
    "test/fixtures/sdeverywhere/models/delayfixed2/delayfixed2.xmile",
    "test/fixtures/sdeverywhere/models/elmcount/elmcount.xmile",
    "test/fixtures/sdeverywhere/models/initial/initial.xmile",
    "test/fixtures/sdeverywhere/models/npv/npv.xmile",
    "test/fixtures/sdeverywhere/models/prune/prune.xmile",
    "test/fixtures/sdeverywhere/models/pulsetrain/pulsetrain.xmile",
    "test/fixtures/sdeverywhere/models/specialchars/specialchars.xmile",
    "test/fixtures/sdeverywhere/models/subalias/subalias.xmile",
    "test/fixtures/sdeverywhere/models/trend/trend.xmile",
    "test/fixtures/simlin-test/lookup_minimal/lookup_minimal.xmile",
    "test/fixtures/test-models/samples/teacup/teacup.xmile",
    "test/fixtures/test-models/samples/teacup/teacup_w_diagram.xmile",
    "test/fixtures/test-models/tests/abs/test_abs.xmile",
    "test/fixtures/test-models/tests/active_initial/test_active_initial.xmile",
    "test/fixtures/test-models/tests/arguments/test_arguments.xmile",
    "test/fixtures/test-models/tests/builtin_int/builtin_int.xmile",
    "test/fixtures/test-models/tests/builtin_max/builtin_max.xmile",
    "test/fixtures/test-models/tests/builtin_min/builtin_min.xmile",
    "test/fixtures/test-models/tests/chained_initialization/test_chained_initialization.xmile",
    "test/fixtures/test-models/tests/comparisons/comparisons.xmile",
    "test/fixtures/test-models/tests/constant_expressions/test_constant_expressions.xmile",
    "test/fixtures/test-models/tests/delay_parentheses/test_delay_parentheses.xmile",
    "test/fixtures/test-models/tests/delay_pipeline/test_pipeline_delays.xmile",
    "test/fixtures/test-models/tests/delays/test_delays.xmile",
    // A 2-argument isee delay() (no initial value) now pads to a full-arity
    // DELAY FIXED(input, delay, input) at read time, so XMILE->MDL emits valid
    // Vensim and both directions round-trip.
    "test/fixtures/test-models/tests/delay_xmile/test_delay_xmile.xmile",
    "test/fixtures/test-models/tests/euler_step_vs_saveper/test_euler_step_vs_saveper.xmile",
    "test/fixtures/test-models/tests/eval_order/eval_order.xmile",
    "test/fixtures/test-models/tests/exp/test_exp.xmile",
    "test/fixtures/test-models/tests/exponentiation/exponentiation.xmile",
    "test/fixtures/test-models/tests/function_capitalization/test_function_capitalization.xmile",
    "test/fixtures/test-models/tests/game/test_game.xmile",
    "test/fixtures/test-models/tests/if_stmt/if_stmt.xmile",
    "test/fixtures/test-models/tests/initial_function/test_initial.xmile",
    "test/fixtures/test-models/tests/limits/test_limits.xmile",
    "test/fixtures/test-models/tests/line_breaks/test_line_breaks.xmile",
    "test/fixtures/test-models/tests/line_continuation/test_line_continuation.xmile",
    "test/fixtures/test-models/tests/ln/test_ln.xmile",
    "test/fixtures/test-models/tests/log/test_log.xmile",
    "test/fixtures/test-models/tests/logicals/test_logicals.xmile",
    "test/fixtures/test-models/tests/logicals/test_logicals_caseinsensitive.xmile",
    "test/fixtures/test-models/tests/lookups/test_lookups.xmile",
    "test/fixtures/test-models/tests/lookups/test_lookups_xpts_sep.xmile",
    "test/fixtures/test-models/tests/lookups/test_lookups_xscale.xmile",
    "test/fixtures/test-models/tests/lookups/test_lookups_ypts_sep.xmile",
    "test/fixtures/test-models/tests/lookups_inline/test_lookups_inline.xmile",
    "test/fixtures/test-models/tests/lookups_inline_bounded/test_lookups_inline_bounded.xmile",
    "test/fixtures/test-models/tests/lookups_simlin/test_lookups.xmile",
    "test/fixtures/test-models/tests/lookups_with_expr/test_lookups_with_expr.xmile",
    "test/fixtures/test-models/tests/model_doc/model_doc.xmile",
    "test/fixtures/test-models/tests/non_negative_all/test_non_negative_all1.xmile",
    "test/fixtures/test-models/tests/non_negative_all/test_non_negative_all2.xmile",
    "test/fixtures/test-models/tests/non_negative_flows/test_non_negative_flows.xmile",
    "test/fixtures/test-models/tests/non_negative_flows/test_non_negative_flows_behavior.xmile",
    "test/fixtures/test-models/tests/non_negative_stocks/test_non_negative_stocks.xmile",
    "test/fixtures/test-models/tests/non_negative_stocks/test_non_negative_stocks_behavior.xmile",
    "test/fixtures/test-models/tests/number_handling/test_number_handling.xmile",
    "test/fixtures/test-models/tests/parentheses/test_parens.xmile",
    // pi() and bare pi now lower to a full-precision numeric literal at read
    // time (Vensim has no PI builtin); the XMILE writer emits equation numbers
    // round-trip-exactly, so both directions compare equal.
    "test/fixtures/test-models/tests/pi/test_pi.xmile",
    "test/fixtures/test-models/tests/reference_capitalization/test_reference_capitalization.xmile",
    "test/fixtures/test-models/tests/rounding/test_rounding.xmile",
    "test/fixtures/test-models/tests/rounding_simlin/test_rounding.xmile",
    "test/fixtures/test-models/tests/sqrt/test_sqrt.xmile",
    "test/fixtures/test-models/tests/subscript_1d_arrays/test_subscript_1d_arrays.xmile",
    "test/fixtures/test-models/tests/subscript_constant_call/test_subscript_constant_call.xmile",
    "test/fixtures/test-models/tests/trend/test_trend.xmile",
    "test/fixtures/test-models/tests/trig/test_trig.xmile",
    "test/fixtures/test-models/tests/variable_ranges/test_variable_ranges.xmile",
    "test/fixtures/test-models/tests/xidz_zidz/xidz_zidz.xmile",
    "test/fixtures/test-models/tests/zeroled_decimals/test_zeroled_decimals.xmile",
    // Wave 2: arrayed models. The reader now folds equation-reference subscripts
    // into the flat SYMBOL/BANG shape both writers consume and binds bare `*`
    // wildcards to their referenced variable's dimension family post-parse (see
    // Model::ResolveWildcardSubscripts); these round-trip in both directions.
    "test/fixtures/sdeverywhere/models/delay/delay.xmile",
    "test/fixtures/sdeverywhere/models/delayfixed/delayfixed.xmile",
    "test/fixtures/sdeverywhere/models/except/except.xmile",
    "test/fixtures/sdeverywhere/models/except2/except2.xmile",
    "test/fixtures/sdeverywhere/models/index/index.xmile",
    "test/fixtures/sdeverywhere/models/interleaved/interleaved.xmile",
    "test/fixtures/sdeverywhere/models/longeqns/longeqns.xmile",
    "test/fixtures/sdeverywhere/models/mapping/mapping.xmile",
    "test/fixtures/sdeverywhere/models/multimap/multimap.xmile",
    "test/fixtures/sdeverywhere/models/ref/ref.xmile",
    "test/fixtures/sdeverywhere/models/smooth/smooth.xmile",
    "test/fixtures/sdeverywhere/models/smooth3/smooth3.xmile",
    "test/fixtures/sdeverywhere/models/subscript/subscript.xmile",
    "test/fixtures/sdeverywhere/models/sum/sum.xmile",
    "test/fixtures/test-models/tests/subscript_2d_arrays/test_subscript_2d_arrays.xmile",
    "test/fixtures/test-models/tests/subscript_3d_arrays/test_subscript_3d_arrays.xmile",
    "test/fixtures/test-models/tests/subscript_3d_arrays_lengthwise/test_subscript_3d_arrays_lengthwise.xmile",
    "test/fixtures/test-models/tests/subscript_3d_arrays_widthwise/test_subscript_3d_arrays_widthwise.xmile",
    "test/fixtures/test-models/tests/subscript_docs/subscript_docs.xmile",
    "test/fixtures/test-models/tests/subscript_individually_defined_1_of_2d_arrays/"
    "subscript_individually_defined_1_of_2d_arrays.xmile",
    "test/fixtures/test-models/tests/subscript_individually_defined_1_of_2d_arrays_from_floats/"
    "subscript_individually_defined_1_of_2d_arrays_from_floats.xmile",
    "test/fixtures/test-models/tests/subscript_individually_defined_1d_arrays/"
    "subscript_individually_defined_1d_arrays.xmile",
    "test/fixtures/test-models/tests/subscript_individually_defined_stocks/"
    "test_subscript_individually_defined_stocks.xmile",
    "test/fixtures/test-models/tests/subscript_mixed_assembly/test_subscript_mixed_assembly.xmile",
    "test/fixtures/test-models/tests/subscript_multiples/test_multiple_subscripts.xmile",
    "test/fixtures/test-models/tests/subscript_selection/subscript_selection.xmile",
    "test/fixtures/test-models/tests/subscript_subranges/test_subscript_subrange.xmile",
    "test/fixtures/test-models/tests/subscript_subranges_equal/test_subscript_subrange_equal.xmile",
    "test/fixtures/test-models/tests/subscript_updimensioning/test_subscript_updimensioning.xmile",
    "test/fixtures/test-models/tests/subscripted_delays/test_subscripted_delays.xmile",
    "test/fixtures/test-models/tests/subscripted_flows/test_subscripted_flows.xmile",
    // Multi-flow sketch geometry: a stock with a vertical flow now round-trips.
    // Two fixes made the view a geometric fixpoint -- SetViewStart no longer
    // lets a straight (0,0) connector peg the view origin (VensimView.cpp), and
    // each flow pipe's <pts> endpoints anchor at the connected element's center
    // on both axes instead of a connector routing midpoint (XMILEGenerator.cpp).
    "test/fixtures/test-models/tests/time/test_time.xmile",
    "test/fixtures/test-models/tests/subscript_switching/subscript_switching.xmile",
    // Non-ASCII identifiers: the XMILE equation lexer now admits UTF-8 bytes
    // (>= 0x80) as identifier constituents, matching the Vensim lexer, so
    // accented Latin names round-trip in both directions.
    "test/fixtures/test-models/tests/unicode_characters/unicode_test_model.xmile",
    // Both carry a real <flow> literally named "<stock> net flow". The MDL
    // writer used to classify a net-flow carrier by that NAME, so it deleted
    // these modeler-authored flows and left the sketch pointing at a variable
    // with no equation -- which segfaulted the next pass. The carrier is now
    // identified by an explicit provenance flag, so the flows survive.
    "test/fixtures/test-models/tests/smooth_and_stock/test_smooth_and_stock.xmile",
    "test/fixtures/test-models/tests/stocks_with_expressions/test_stock_with_expression.xmile",
    // Wave 2: Stella .stmx dialect. Same round-trip contract as the .xmile
    // corpus -- the reader dispatches on the <xmile> root, not the file
    // extension, so a .stmx (including the legacy 1.0-format variants and
    // isee:-namespaced content) that round-trips cleanly belongs here.
    "test/fixtures/simlin-test/alias1/alias1.stmx",
    "test/fixtures/simlin-test/arms_race_3party/arms_race.stmx",
    "test/fixtures/simlin-test/builtin_init/builtin_init.stmx",
    "test/fixtures/simlin-test/circular-dep-1/model.stmx",
    "test/fixtures/simlin-test/decoupled_stocks/decoupled.stmx",
    "test/fixtures/simlin-test/logistic_growth_ltm/logistic_growth.stmx",
    "test/fixtures/test-models/samples/arrays/a2a/a2a.stmx",
    "test/fixtures/test-models/samples/arrays/non-a2a/non-a2a.stmx",
    "test/fixtures/test-models/samples/SIR/SIR.stmx",
    "test/fixtures/test-models/samples/SIR/SIR_legacy.stmx",
    "test/fixtures/test-models/samples/teacup/teacup.stmx",
    "test/fixtures/test-models/samples/teacup/teacup_legacy.stmx",
    // Equal-precedence groupings: the MDL walker used to re-group an operand at
    // its parent's own precedence level (emitting `Input * (Delay Time / 3)` as
    // `Input * Delay Time / 3`), which showed up here as an XMILE->MDL diff on
    // the DELAY3 pipeline stocks' initial values and on the arithmetic torture
    // model. The grouping is now preserved, so all three round-trip.
    "test/fixtures/simlin-test/delays/model.xmile",
    "test/fixtures/test-models/tests/delays2/delays.xmile",
    "test/fixtures/test-models/tests/arithmetics_exp/test_arithmetics_exp.xmile",
    // Negative-literal grouping: `INTEGER(-0.9 / 1)` was emitted for the tree
    // INTEGER((-0.9) / 1), and the leading sign re-parsed as the grammar's
    // lowest-precedence unary minus, so the re-import read INTEGER(-(0.9 / 1)).
    // That was quantum's ONLY remaining diff -- its recorded deferral reason
    // (a :SUPPLEMENTARY marker lost on MDL reparse) no longer reproduces -- and
    // with the sign parenthesized both directions round-trip.
    "test/fixtures/sdeverywhere/models/quantum/quantum.xmile",
    // Both declare <unit name="$"> with an empty <eqn/> and two aliases. The
    // XMILE writer used to rebuild a <unit> by guessing which field of a
    // flattened comma-joined string was the equation -- a rule that recognized
    // only a literal "$" -- so it re-emitted these as <unit name="dollar">
    // <eqn>$</eqn>, promoting the first alias to the name. The declaration is
    // now held as a UnitEquiv (name / eqn / aliases kept apart), so it survives
    // unchanged and both round-trip.
    "test/fixtures/simlin-test/ai-information/PureAIModel.stmx",
    "test/fixtures/simlin-test/ai-information/PureHumanModel.stmx",
};

struct DeferredModel {
  const char *path;
  const char *reason;
};

// Candidate models that cannot round-trip in wave 1, each with the reason.
// These are recorded (not asserted for an empty diff) so the corpus stays
// honest about its boundaries; the test only confirms the fixtures still exist.
const DeferredModel kKnownDeferred[] = {
    // Out-of-scope Vensim data/allocation builtins (the MDL writer's v1 scope
    // never supported these; the XMILE equation lexer rejects the untranslated
    // "{...}" wrappers and "???" placeholders the exporter leaves behind).
    {"test/fixtures/sdeverywhere/models/allocate/allocate.xmile",
     "ALLOCATE AVAILABLE (untranslated function) not supported"},
    {"test/fixtures/sdeverywhere/models/directconst/directconst.xmile",
     "GET DIRECT CONSTANTS external-data builtin not supported"},
    {"test/fixtures/sdeverywhere/models/directdata/directdata.xmile",
     "GET DIRECT DATA external-data builtin not supported"},
    {"test/fixtures/sdeverywhere/models/directlookups/directlookups.xmile",
     "GET DIRECT LOOKUPS external-data builtin not supported"},
    {"test/fixtures/sdeverywhere/models/directsubs/directsubs.xmile",
     "GET DIRECT SUBSCRIPT (exported as \"???\") not supported"},
    {"test/fixtures/sdeverywhere/models/extdata/extdata.xmile", "VECTOR SELECT / external-data builtins not supported"},
    {"test/fixtures/sdeverywhere/models/getdata/getdata.xmile", "GET DATA external-data builtin not supported"},
    {"test/fixtures/sdeverywhere/models/vector/vector.xmile", "VECTOR ELM MAP builtin not supported"},
    // Reader gap: a <gf> graphical function on a per-element subscripted variable
    // is intentionally unsupported (ProcessPerElementEquations skips the <gf>).
    {"test/fixtures/sdeverywhere/models/lookup/lookup.xmile",
     "<gf> on a per-element subscripted variable is not supported by the reader"},
    // Per-element variable over reordered shared-element dimension aliases: the
    // element's owning family (used to emit <dimensions> and to bind bare `*`)
    // depends on dimension declaration order, which the XMILE writer reorders,
    // so the declared dimension flips (e.g. DimA<->DimX) across a round trip.
    // XMILE->MDL passes; XMILE->XMILE does not. Pre-existing ownership limitation
    // (see the "bug elsewhere" note in SymbolList::OutputComputable), exposed now
    // that reference subscripts actually round-trip.
    {"test/fixtures/sdeverywhere/models/arrays_cname/arrays_cname.xmile",
     "per-element var over reordered shared-element dimension aliases (owning family unstable across round trip)"},
    {"test/fixtures/sdeverywhere/models/arrays_varname/arrays_varname.xmile",
     "per-element var over reordered shared-element dimension aliases (owning family unstable across round trip)"},
    // Array literal apply-to-all body (`var = 1, 2, 3`): the XMILE comma-list
    // per-element constant form is not accepted by the equation grammar.
    {"test/fixtures/test-models/tests/min_max_1arg/test_min_max_1arg.xmile",
     "array-literal apply-to-all body (`1, 2, 3`) not accepted by the equation grammar"},
    // Stella control-variable aliases (STARTTIME/STOPTIME/DT) declared as members
    // of a Control <group>: the reader materializes them as ordinary variables,
    // but the MDL writer emits the canonical INITIAL TIME/FINAL TIME/TIME STEP in
    // .Control, so the reparsed model lacks the aliases. XMILE->XMILE passes.
    {"test/fixtures/sdeverywhere/models/sumif/sumif.xmile",
     "Stella control-var aliases (STARTTIME/STOPTIME/DT) in a Control group not reconciled with Vensim control vars"},
    // Sketch-geometry (not array) gaps: arrays round-trip, view geometry does
    // not. The vertical-flow origin drift is fixed (see test_time /
    // subscript_switching on the allow-list); these remaining models still show
    // a per-element write/read asymmetry on their flow-adjacent elements after
    // origin normalization -- a distinct, deeper geometry gap.
    {"test/fixtures/sdeverywhere/models/sir/model/sir.xmile",
     "flow-adjacent element geometry write/read asymmetry after origin normalization"},
    {"test/fixtures/sdeverywhere/models/sir/sir.xmile",
     "flow-adjacent element geometry write/read asymmetry after origin normalization"},
    {"test/fixtures/test-models/samples/SIR/SIR.xmile",
     "flow-adjacent element geometry write/read asymmetry after origin normalization (subscripted SIR)"},
    {"test/fixtures/test-models/samples/SIR/SIR_reciprocal-dt.xmile",
     "flow-adjacent element geometry write/read asymmetry after origin normalization (subscripted SIR)"},
    // Non-array deferrals carried over from wave 1 (unchanged).
    {"test/fixtures/sdeverywhere/models/sample/sample.xmile",
     "Stella PREVIOUS(SELF, ...) self-reference construct, wave 2 (Stella dialect)"},
    {"test/fixtures/test-models/tests/builtin_mean/builtin_mean.xmile",
     "isee MEAN(list...) referencing TIME not reconstructed on MDL reparse"},
    {"test/fixtures/test-models/tests/input_functions/test_inputs.xmile",
     "XMILE pulse() is now translated to the correct Vensim area-pulse expression "
     "((v/TIME STEP)*PULSE TRAIN(...)), but the XMILE writer expands Vensim PULSE/PULSE TRAIN to an "
     "IF-expression, so XMILE->XMILE re-reads an IF tree instead of the PULSE TRAIN node -- structure "
     "differs across the round trip even though semantics are preserved"},
    // The following three were previously on the allow-list but round-tripped
    // only because the reader silently mis-converted an unsupported construct
    // identically in both directions (a "silent wrong results" defect). The
    // equation-diagnostics fix now rejects them at parse time; each names a
    // construct xmutil's engine has no representation for.
    {"test/fixtures/test-models/tests/subscript_aggregation/test_subscript_aggregation.xmile",
     "sdeverywhere emits vector min/max as LOOKUP(VMIN/VMAX, arr[*]) with the function name as a bare "
     "LOOKUP argument, which the reader cannot resolve (was silently converted to LOOKUP(0, arr))"},
    {"test/fixtures/test-models/tests/subscripted_trig/test_subscripted_trig.xmile",
     "hyperbolic trig COSH/SINH/TANH have no xmutil engine representation (were silently converted to "
     "phantom lookups)"},
    {"test/fixtures/test-models/tests/lookups/test_lookups_no-indirect.xmile",
     "sketch geometry differs on the lookup view (xx)"},
    {"test/fixtures/test-models/tests/lookups_funcnames/test_lookups_funcnames.xmile",
     "variable name embeds the reserved MOD operator keyword"},
    {"test/fixtures/test-models/tests/macro_expression/test_macro_expression.xmile",
     "XMILE <macro> not supported by the reader (out of scope, like <module>)"},
    {"test/fixtures/test-models/tests/macro_multi_expression/test_macro_multi_expression.xmile",
     "XMILE <macro> not supported by the reader (out of scope, like <module>)"},
    {"test/fixtures/test-models/tests/macro_multi_macros/test_macro_multi_macros.xmile",
     "XMILE <macro> not supported by the reader (out of scope, like <module>)"},
    {"test/fixtures/test-models/tests/macro_stock/test_macro_stock.xmile",
     "XMILE <macro> not supported by the reader (out of scope, like <module>)"},
    {"test/fixtures/test-models/tests/special_characters_xmile/test_special_variable_names.xmile",
     "special characters in names are not re-lexable in generated output"},
    {"test/fixtures/test-models/tests/special_characters_xmile_simlin/test_special_variable_names.xmile",
     "special characters in names are not re-lexable in generated output"},
    // Wave 2: Stella .stmx dialect gaps (each a specific reader/writer limitation,
    // not "arrayed"). The module-submodel .stmx files are in kRejected instead.
    {"test/fixtures/simlin-test/ai-information/GeneratedByAIThenEdited.stmx",
     "graphical-function <pts> without exactly 2 <pt> children not emittable"},
    {"test/fixtures/simlin-test/arrays1/arrays.stmx",
     "expression-valued subscript index (`aux[INT(TIME MOD 5)+1]`) not accepted by the subscript grammar"},
    // The multiple-<model> rejection this used to name is fixed (XMILE input is
    // emitted in the single-<model> sector form, so this document now re-imports
    // cleanly). Two independent gaps remain, both visible in the round-trip
    // diff. First: it is the only corpus model with BOTH a sketch <view> and
    // <group> membership, and generateSectorViews writes the <group> list only
    // for a model with no view, so all five groups are dropped on the way out.
    // Second: its flow-adjacent sketch elements shift x by a constant on the
    // second pass, the same origin-normalization asymmetry as the SIR entries
    // above, so it is not a fixpoint either.
    {"test/fixtures/simlin-test/land_model/land_model.stmx",
     "sectors emission drops <group> membership for a model that has a sketch <view>, and the flow-adjacent "
     "geometry drifts on a second pass (same asymmetry as the SIR entries)"},
    {"test/fixtures/simlin-test/previous/model.stmx",
     "Stella PREVIOUS(SELF, ...) self-reference construct (Stella dialect)"},
    {"test/fixtures/simlin-test/step_into_smth1/model.stmx",
     "variable named \"initial\" collides with the INITIAL builtin keyword (Stella dialect)"},
    {"test/fixtures/simlin-test/subscript_index_name_values/model.stmx",
     "dotted dimension.element subscript reference (`Location.Boston`) not accepted by the equation grammar"},
    {"test/fixtures/test-models/samples/arrays/non-a2a/non-a2a-gf.stmx",
     "<gf> on a per-element subscripted variable is not supported by the reader"},
    {"test/fixtures/test-models/samples/display/1style.stmx",
     "presentation-only Stella model: <stock> elements carry no <eqn>"},
    {"test/fixtures/test-models/samples/display/multipoint-connection.stmx",
     "presentation-only Stella model: <aux> carries neither <eqn> nor <gf>"},
};

// Models the reader rejects by design -- real XMILE <module> submodel documents
// (multiple sibling <model> elements). Unlike the deferred table (which only
// checks the fixture exists), each entry here asserts ParseXMILE fails and the
// diagnostic contains the expected substring, so a regression that silently
// started accepting modules would be caught. Modules are out of scope for the
// single-model conversion path (mirrors the <macro>/<module> rejection contract).
struct RejectedModel {
  const char *path;
  const char *expected_error_substr;
};

const RejectedModel kRejected[] = {
    {"test/fixtures/simlin-test/ai-information/WithModulesAndArrays.stmx", "multiple <model> elements"},
    {"test/fixtures/test-models/samples/bpowers-hares_and_lynxes_modules/model.stmx", "multiple <model> elements"},
    {"test/fixtures/test-models/samples/bpowers-hares_and_lynxes_modules/model_legacy.stmx",
     "multiple <model> elements"},
};

// XMILE fixtures that live under test/fixtures/ but are exercised by a different
// test file, so they intentionally appear in neither table above. The three
// simlin default-project models are driven by test/xmile/CorpusRoundTripTest.cpp.
const char *const kCoveredElsewhere[] = {
    "test/fixtures/simlin/fishbanks.xmile",
    "test/fixtures/simlin/logistic-growth.xmile",
    "test/fixtures/simlin/reliability.xmile",
};

}  // namespace

// Every allow-list model must be present on disk and round-trip cleanly in both
// directions. A missing file or any non-empty diff is a regression.
TEST(XmileCorpus_allowlist_round_trips_both_directions) {
  int xx_passed = 0;
  int xm_passed = 0;
  for (const char *rel : kAllowList) {
    bool found = false;
    std::string text = ReadFixture(rel, found);
    CHECK(found);
    if (!found) {
      printf("  allow-list fixture missing: %s\n", rel);
      continue;
    }

    std::vector<std::string> xx = xmileroundtrip::RoundTripDiffs(text);
    if (!xx.empty()) {
      printf("  XMILE->XMILE did not round-trip: %s\n", rel);
      for (const std::string &d : xx)
        printf("    diff: %s\n", d.c_str());
    }
    CHECK(xx.empty());
    if (xx.empty())
      xx_passed++;

    std::vector<std::string> xm = xmileroundtrip::XmileToMdlDiffs(text);
    if (!xm.empty()) {
      printf("  XMILE->MDL did not round-trip: %s\n", rel);
      for (const std::string &d : xm)
        printf("    diff: %s\n", d.c_str());
    }
    CHECK(xm.empty());
    if (xm.empty())
      xm_passed++;
  }
  printf("  xmile corpus allow-list: %d/%zu xmile->xmile, %d/%zu xmile->mdl\n", xx_passed,
         sizeof(kAllowList) / sizeof(kAllowList[0]), xm_passed, sizeof(kAllowList) / sizeof(kAllowList[0]));
}

// Every allow-list model must also be a FIXPOINT of the normalizer: converting
// a document this writer produced must reproduce it byte for byte. The
// structural check above cannot see this class of defect -- a writer that adds
// a paren layer or permutes its variable order on every pass still compares
// structurally equal each time, so the conversion would drift forever without
// any test going red.
TEST(XmileCorpus_allowlist_normalization_is_a_fixpoint) {
  int passed = 0;
  for (const char *rel : kAllowList) {
    bool found = false;
    std::string text = ReadFixture(rel, found);
    CHECK(found);
    if (!found) {
      printf("  allow-list fixture missing: %s\n", rel);
      continue;
    }
    const std::string failure = xmileroundtrip::FixpointFailure(text);
    if (!failure.empty())
      printf("  XMILE normalization is not a fixpoint: %s\n    %s\n", rel, failure.c_str());
    CHECK(failure.empty());
    if (failure.empty())
      passed++;
  }
  printf("  xmile corpus allow-list: %d/%zu reach a fixpoint\n", passed, sizeof(kAllowList) / sizeof(kAllowList[0]));
}

// Confirm each deferred fixture still exists and log its documented reason, so
// the deferral list cannot silently reference a fixture that was moved or
// deleted. If a future fix makes one of these round-trip, promote it into the
// allow-list.
TEST(XmileCorpus_known_deferred_are_documented) {
  for (const DeferredModel &dm : kKnownDeferred) {
    bool found = false;
    ReadFixture(dm.path, found);
    CHECK(found);
    if (!found)
      printf("  deferred fixture missing: %s\n", dm.path);
    else
      printf("  DEFERRED %s -- %s\n", dm.path, dm.reason);
  }
}

// Each by-design rejection must actually fail to parse with the documented
// diagnostic. This is a stronger assertion than the deferred table (which only
// checks existence): a regression that started accepting module documents would
// flip these from a clean rejection to a partial parse and be caught here.
TEST(XmileCorpus_rejected_models_fail_cleanly) {
  for (const RejectedModel &rm : kRejected) {
    bool found = false;
    std::string text = ReadFixture(rm.path, found);
    CHECK(found);
    if (!found) {
      printf("  rejected fixture missing: %s\n", rm.path);
      continue;
    }
    std::vector<std::string> errs;
    Model *m = xmileroundtrip::ParseXMILE(text, errs);
    CHECK(m == nullptr);
    delete m;  // no-op on nullptr; guards against a future non-null return
    bool matched = false;
    for (const std::string &e : errs) {
      if (e.find(rm.expected_error_substr) != std::string::npos) {
        matched = true;
        break;
      }
    }
    CHECK(matched);
    if (!matched)
      printf("  REJECT %s did not report expected error '%s'\n", rm.path, rm.expected_error_substr);
  }
}

// Enforce the "every fixture is in exactly one table" invariant mechanically:
// walk every *.xmile / *.stmx actually on disk under test/fixtures/ and fail if
// any is in none of kAllowList / kKnownDeferred / kRejected / kCoveredElsewhere.
// A comment cannot catch a fixture added later, but this walk does -- so a new
// model must be triaged into a table, never silently untested.
TEST(XmileCorpus_every_fixture_is_accounted_for) {
  std::set<std::string> accounted;
  for (const char *p : kAllowList)
    accounted.insert(p);
  for (const DeferredModel &dm : kKnownDeferred)
    accounted.insert(dm.path);
  for (const RejectedModel &rm : kRejected)
    accounted.insert(rm.path);
  for (const char *p : kCoveredElsewhere)
    accounted.insert(p);

  namespace fs = std::filesystem;
  const std::string root = std::string(XMUTIL_SRC_ROOT);
  const fs::path fixtures = fs::path(root) / "test" / "fixtures";

  int checked = 0;
  int unaccounted = 0;
  std::error_code ec;
  for (fs::recursive_directory_iterator it(fixtures, ec), end; it != end; it.increment(ec)) {
    if (ec) {
      printf("  directory walk error: %s\n", ec.message().c_str());
      break;
    }
    if (!it->is_regular_file())
      continue;
    const std::string ext = it->path().extension().string();
    if (ext != ".xmile" && ext != ".stmx")
      continue;
    // Recover the repo-relative path (the form the tables use) by stripping the
    // XMUTIL_SRC_ROOT prefix and normalizing to forward slashes.
    std::string rel = fs::relative(it->path(), root, ec).generic_string();
    if (ec) {
      printf("  could not relativize %s: %s\n", it->path().string().c_str(), ec.message().c_str());
      continue;
    }
    checked++;
    if (accounted.find(rel) == accounted.end()) {
      unaccounted++;
      printf("  UNACCOUNTED fixture (add to kAllowList / kKnownDeferred / kRejected): %s\n", rel.c_str());
    }
  }

  CHECK(checked > 0);  // a zero count means the walk found nothing -- a path bug
  CHECK(unaccounted == 0);
  printf("  xmile corpus coverage: %d fixtures on disk, %d unaccounted\n", checked, unaccounted);
}
