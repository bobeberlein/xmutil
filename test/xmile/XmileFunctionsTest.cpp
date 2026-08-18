#include <string>

#include "../../src/Function/Function.h"
#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/Variable.h"
#include "../../src/Vensim/VensimParse.h"
#include "../../src/Xmile/XmileFunctions.h"
#include "../TestHarness.h"

namespace {

// Fixture stands up a Model whose namespace has every Vensim-canonical
// Function* registered, which is the precondition the lookup helpers rely on.
// VensimParse's ctor runs ReadyFunctions() against pSymbolNameSpace; its dtor
// clears the VPObject global. We deliberately keep the VensimParse alive for
// the whole test so the registered functions stay reachable.
struct Fixture {
  Model m;
  VensimParse vp;
  Fixture() : vp(&m) {
  }
};

}  // namespace

TEST(XmileFunctions_bare_keyword_time_creates_Time_variable) {
  Fixture f;
  Variable *v = xmile::LookupBareKeyword(f.m.GetNameSpace(), "time");
  CHECK(v != nullptr);
  if (v)
    CHECK_EQ_STR(v->GetName(), "Time");
}

TEST(XmileFunctions_bare_keyword_dt_maps_to_TIME_STEP) {
  Fixture f;
  Variable *v = xmile::LookupBareKeyword(f.m.GetNameSpace(), "dt");
  CHECK(v != nullptr);
  if (v)
    CHECK_EQ_STR(v->GetName(), "TIME STEP");
}

TEST(XmileFunctions_bare_keyword_initial_time_maps_to_INITIAL_TIME) {
  Fixture f;
  Variable *v = xmile::LookupBareKeyword(f.m.GetNameSpace(), "initial_time");
  CHECK(v != nullptr);
  if (v)
    CHECK_EQ_STR(v->GetName(), "INITIAL TIME");
}

TEST(XmileFunctions_bare_keyword_unknown_returns_null) {
  Fixture f;
  Variable *v = xmile::LookupBareKeyword(f.m.GetNameSpace(), "totally_made_up");
  CHECK(v == nullptr);
}

TEST(XmileFunctions_function_smth1_maps_to_SMOOTH) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "smth1", 2);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetName(), "SMOOTH");
}

TEST(XmileFunctions_function_safediv_2arg_is_ZIDZ) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "safediv", 2);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetName(), "ZIDZ");
}

TEST(XmileFunctions_function_safediv_3arg_is_XIDZ) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "safediv", 3);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetName(), "XIDZ");
}

TEST(XmileFunctions_function_int_maps_to_INTEGER) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "int", 1);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetName(), "INTEGER");
}

TEST(XmileFunctions_function_fallback_uppercase_underscores) {
  Fixture f;
  // "if_then_else" is absent from kFuncMap, so the underbar-to-space +
  // uppercase fallback must produce "IF THEN ELSE" and find FunctionIfThenElse.
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "if_then_else", 3);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetName(), "IF THEN ELSE");
}

TEST(XmileFunctions_function_unknown_returns_null) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "completely_unknown_fn", 1);
  CHECK(fn == nullptr);
}

// MIN/MAX carry one XMILE spelling for both the scalar pairwise form and the
// array reducer; the reader must resolve VMIN/VMAX for the one-argument call
// and MIN/MAX for the two-argument call.
TEST(XmileFunctions_min_1arg_is_VMIN) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "min", 1);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetName(), "VMIN");
}

TEST(XmileFunctions_min_2arg_is_MIN) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "min", 2);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetName(), "MIN");
}

TEST(XmileFunctions_max_1arg_is_VMAX) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "max", 1);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetName(), "VMAX");
}

TEST(XmileFunctions_max_2arg_is_MAX) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "max", 2);
  CHECK(fn != nullptr);
  if (fn)
    CHECK_EQ_STR(fn->GetName(), "MAX");
}

// The argument-count range the XMILE reader enforces. Most functions pin a
// single count (min == max == NumberArgs); the delay/trend/ramp/random builtins
// whose trailing argument XMILE may omit carry a widened lower bound.
TEST(XmileFunctions_fixed_arity_has_single_arg_count) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "sin", 1);
  CHECK(fn != nullptr);
  if (fn) {
    CHECK(fn->MinNumberArgs() == 1);
    CHECK(fn->MaxNumberArgs() == 1);
  }
}

TEST(XmileFunctions_delay_fixed_accepts_optional_initial) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "delay", 2);
  CHECK(fn != nullptr);
  if (fn) {
    CHECK_EQ_STR(fn->GetName(), "DELAY FIXED");
    CHECK(fn->MinNumberArgs() == 2);
    CHECK(fn->MaxNumberArgs() == 3);
  }
}

TEST(XmileFunctions_trend_accepts_optional_initial) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "trend", 2);
  CHECK(fn != nullptr);
  if (fn) {
    CHECK(fn->MinNumberArgs() == 2);
    CHECK(fn->MaxNumberArgs() == 3);
  }
}

TEST(XmileFunctions_random_uniform_accepts_optional_seed) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "uniform", 2);
  CHECK(fn != nullptr);
  if (fn) {
    CHECK_EQ_STR(fn->GetName(), "RANDOM UNIFORM");
    CHECK(fn->MinNumberArgs() == 2);
    CHECK(fn->MaxNumberArgs() == 3);
  }
}

// DELAY N reorders its trailing (order, initial) pair between the XMILE and
// Vensim signatures, so its reduced-arity form cannot be soundly translated;
// it stays pinned at exactly four arguments.
TEST(XmileFunctions_delay_n_is_strict_arity) {
  Fixture f;
  Function *fn = xmile::LookupFunction(f.m.GetNameSpace(), "delayn", 4);
  CHECK(fn != nullptr);
  if (fn) {
    CHECK_EQ_STR(fn->GetName(), "DELAY N");
    CHECK(fn->MinNumberArgs() == 4);
    CHECK(fn->MaxNumberArgs() == 4);
  }
}
