// Phase 6 round-trip tests for the XMILE reader: <gf> graphical functions in
// both inline (WITH LOOKUP) and standalone shapes, plus the <xscale>-only
// variant the fishbanks corpus exercises and the type="extrapolate" flag.
//
// Each fixture is a hand-written XMILE document so the wire shape under test
// is visible at the call site.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Equation.h"
#include "../../src/Symbol/Expression.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/Symbol/Variable.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

const char *kWithLookupExplicitXpts = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="input"><eqn>5</eqn></aux>
    <aux name="output">
      <eqn>input</eqn>
      <gf>
        <xpts>0,5,10</xpts>
        <ypts>0,3,10</ypts>
      </gf>
    </aux>
  </variables></model>
</xmile>
)";

const char *kWithLookupXscaleOnly = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="input"><eqn>5</eqn></aux>
    <aux name="output">
      <eqn>input</eqn>
      <gf>
        <xscale min="0" max="10"/>
        <ypts>0,3,10</ypts>
      </gf>
    </aux>
  </variables></model>
</xmile>
)";

const char *kStandaloneGf = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="f">
      <gf>
        <xpts>0,5,10</xpts>
        <ypts>0,3,10</ypts>
      </gf>
    </aux>
  </variables></model>
</xmile>
)";

const char *kExtrapolateGf = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="input"><eqn>5</eqn></aux>
    <aux name="output">
      <eqn>input</eqn>
      <gf type="extrapolate">
        <xpts>0,5,10</xpts>
        <ypts>0,3,10</ypts>
      </gf>
    </aux>
  </variables></model>
</xmile>
)";

// An <eqn> body that is whitespace-only (e.g. a newline from a pretty-printed
// XMILE file) must be treated as "no eqn" so the variable routes to the
// standalone-GF branch, not the WITH LOOKUP branch where ParseEquation would
// fail on the whitespace string.
const char *kStandaloneGfWhitespaceEqn = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="f">
      <eqn>
</eqn>
      <gf>
        <xpts>0,5,10</xpts>
        <ypts>0,3,10</ypts>
      </gf>
    </aux>
  </variables></model>
</xmile>
)";

}  // namespace

TEST(Lookup_with_lookup_explicit_xpts_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kWithLookupExplicitXpts);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Lookup_with_lookup_xscale_only_round_trip) {
  // xscale -> xs derived; output emits explicit <xpts>. Re-parse sees the
  // same xs (the derived ones) as the original -- comparator equivalent.
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kWithLookupXscaleOnly);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Lookup_standalone_gf_round_trip) {
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kStandaloneGf);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Lookup_extrapolate_flag_round_trip) {
  // type="extrapolate" -> ExpressionTable::SetExtrapolate(true). The writer
  // emits the type attribute back on output, and the re-parse re-applies it.
  std::vector<std::string> diffs = xmileroundtrip::RoundTripDiffs(kExtrapolateGf);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

TEST(Lookup_with_lookup_shape_check) {
  // Direct shape check: an inline gf with non-empty eqn produces an
  // ExpressionLookup with both an input expression and a table.
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kWithLookupExplicitXpts, errs);
  CHECK(m != nullptr);
  CHECK(errs.empty());
  if (!m)
    return;
  Variable *out = static_cast<Variable *>(m->GetNameSpace()->Find("output"));
  CHECK(out != nullptr);
  if (out && !out->GetAllEquations().empty()) {
    Equation *eq = out->GetEquation(0);
    Expression *rhs = eq->GetExpression();
    CHECK(rhs != nullptr);
    if (rhs) {
      ExpressionLookup *lk = dynamic_cast<ExpressionLookup *>(rhs);
      CHECK(lk != nullptr);
      if (lk)
        CHECK(lk->GetTable() != nullptr);
    }
  }
  delete m;
}

TEST(Lookup_standalone_gf_shape_check) {
  // Standalone GF: the equation's expression IS the ExpressionTable.
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kStandaloneGf, errs);
  CHECK(m != nullptr);
  CHECK(errs.empty());
  if (!m)
    return;
  Variable *f = static_cast<Variable *>(m->GetNameSpace()->Find("f"));
  CHECK(f != nullptr);
  if (f && !f->GetAllEquations().empty()) {
    Equation *eq = f->GetEquation(0);
    Expression *rhs = eq->GetExpression();
    CHECK(rhs != nullptr);
    if (rhs)
      CHECK(rhs->GetType() == EXPTYPE_Table);
  }
  delete m;
}

TEST(Lookup_xscale_derives_correct_xs) {
  // Mid-level check: xscale min=0 max=10 with 3 ys -> xs = [0, 5, 10].
  // Exact equality is safe here: the derivation a + i*(b-a)/(N-1) reduces
  // to integer arithmetic for these inputs (10/2 = 5).
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kWithLookupXscaleOnly, errs);
  CHECK(m != nullptr);
  CHECK(errs.empty());
  if (!m)
    return;
  Variable *out = static_cast<Variable *>(m->GetNameSpace()->Find("output"));
  if (out && !out->GetAllEquations().empty()) {
    ExpressionLookup *lk = dynamic_cast<ExpressionLookup *>(out->GetEquation(0)->GetExpression());
    if (lk) {
      ExpressionTable *t = lk->GetTable();
      if (t) {
        std::vector<double> *xs = t->GetXVals();
        if (xs && xs->size() == 3) {
          CHECK((*xs)[0] == 0.0);
          CHECK((*xs)[1] == 5.0);
          CHECK((*xs)[2] == 10.0);
        }
      }
    }
  }
  delete m;
}

// A <ypts> with one multi-byte garbage token ("abc") before valid numbers must
// report skipped=1, not skipped=3 (the byte count of "abc").
const char *kMalformedXpts = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="f">
      <gf>
        <xpts>abc, 1.0, 2.0</xpts>
        <ypts>0, 3, 10</ypts>
      </gf>
    </aux>
  </variables></model>
</xmile>
)";

TEST(Lookup_malformed_xpts_token_counted_once_not_per_byte) {
  // "abc" is a 3-byte garbage run before "1.0, 2.0". The fixed ParseDoubleList
  // must emit "1 malformed token(s) skipped" (one distinct bad token), not
  // "3 malformed token(s) skipped" (three bytes).
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kMalformedXpts, errs);
  CHECK(m != nullptr);
  // Exactly one error message for the malformed xpts token.
  bool found_token_msg = false;
  for (const std::string &e : errs) {
    if (e.find("1 malformed token(s)") != std::string::npos) {
      found_token_msg = true;
    }
    // None of the error messages should claim more than 1 skipped token.
    CHECK(e.find("2 malformed token(s)") == std::string::npos);
    CHECK(e.find("3 malformed token(s)") == std::string::npos);
  }
  CHECK(found_token_msg);
  delete m;
}

// Two distinct garbage tokens separated by a comma must each count individually.
// The cycle-3 in_bad_token flag was only cleared on successful strtod, so
// "abc, xyz, 1.0" collapsed the two bad tokens into one -- this test pins
// the correct token-boundary semantics against regression.
const char *kMalformedXptsTwoBadTokens = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="f">
      <gf>
        <xpts>abc, xyz, 1.0</xpts>
        <ypts>0, 3, 10</ypts>
      </gf>
    </aux>
  </variables></model>
</xmile>
)";

TEST(Lookup_malformed_xpts_two_bad_tokens_counted_separately) {
  // "abc" and "xyz" are two distinct bad tokens separated by ", ". The
  // separator crossing must reset in_bad_token so each is counted independently,
  // yielding "2 malformed token(s) skipped", not "1".
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kMalformedXptsTwoBadTokens, errs);
  CHECK(m != nullptr);
  bool found_two_token_msg = false;
  for (const std::string &e : errs) {
    if (e.find("2 malformed token(s)") != std::string::npos) {
      found_two_token_msg = true;
    }
    CHECK(e.find("1 malformed token(s)") == std::string::npos);
    CHECK(e.find("3 malformed token(s)") == std::string::npos);
  }
  CHECK(found_two_token_msg);
  delete m;
}

TEST(Lookup_whitespace_only_eqn_routes_to_standalone_gf) {
  // Defense-in-depth: a whitespace-only (newline-only) <eqn> body on an aux
  // that also has a <gf> must be treated as "no eqn" and produce a standalone
  // graphical function (ExpressionTable, token '('), not fail to parse.
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(kStandaloneGfWhitespaceEqn, errs);
  CHECK(m != nullptr);
  CHECK(errs.empty());
  if (!m)
    return;
  Variable *f = static_cast<Variable *>(m->GetNameSpace()->Find("f"));
  CHECK(f != nullptr);
  if (f && !f->GetAllEquations().empty()) {
    Equation *eq = f->GetEquation(0);
    Expression *rhs = eq->GetExpression();
    CHECK(rhs != nullptr);
    if (rhs)
      CHECK(rhs->GetType() == EXPTYPE_Table);
  }
  delete m;
}
