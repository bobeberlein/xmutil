// Regression tests for free-text sanitization in the .mdl writer (#849).
//
// A raw structural character in a variable's <doc> (a '|') or <units> (a '~')
// is emitted verbatim by UnitsCommentTrailer today, which terminates the
// equation entry early. On re-import the FOLLOWING variable is silently dropped
// and a syntax error is thrown -- real data loss. These tests pin that every
// variable survives an XMILE -> MDL -> Model round trip and that the sanitized
// substitutes (`|` -> `/`, `~` in units -> space) are a re-import fixpoint.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/Vensim/VensimParse.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

// Aux `a` carries a '~' in its units AND a '|' in its documentation; `b`
// references it. The writer emits variables alphabetically, so `a`'s corrupted
// trailer precedes `b`: a premature field terminator drops `b` on re-import.
const char *kFreeText = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <header><name>free-text</name></header>
  <sim_specs method="euler"><start>0</start><stop>1</stop><dt>1</dt></sim_specs>
  <model>
    <variables>
      <aux name="a"><eqn>1</eqn><units>widget ~ approx</units><doc>alpha | beta = 5</doc></aux>
      <aux name="b"><eqn>a + 2</eqn></aux>
    </variables>
  </model>
</xmile>
)";

// Parse XMILE and emit .mdl text via the writer. Returns "" and populates errs
// on any failure.
std::string XmileToMdl(const char *xmile, std::vector<std::string> &errs) {
  Model *m = xmileroundtrip::ParseXMILE(xmile, errs);
  if (!m)
    return "";
  errs.clear();
  std::string mdl = m->PrintMDL(errs);
  delete m;
  return mdl;
}

// Re-parse .mdl text into a fresh Model via the Vensim reader. Returns nullptr
// on parse failure.
Model *ParseMdl(const std::string &mdl) {
  Model *m = new Model();
  VensimParse vp{m};
  if (!vp.ProcessFile("<test>", mdl.c_str(), mdl.size())) {
    delete m;
    return nullptr;
  }
  m->RunPostParsePipeline();
  return m;
}

bool HasVariable(Model *m, const std::string &name) {
  Symbol *s = m->GetNameSpace()->Find(name);
  return s != nullptr && s->isType() == Symtype_Variable;
}

}  // namespace

// Test A: the data-loss anchor. Both variables must survive the round trip;
// today `b` vanishes because `a`'s raw `|`/`~` terminate its entry early.
TEST(FreeTextSanitize_all_variables_survive_roundtrip) {
  std::vector<std::string> errs;
  std::string mdl = XmileToMdl(kFreeText, errs);
  CHECK(!mdl.empty());

  Model *m = ParseMdl(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;

  CHECK(HasVariable(m, "a"));
  CHECK(HasVariable(m, "b"));
  delete m;
}

// Test B: content and idempotence. The '|' becomes '/', the units field carries
// no bare '~', and a second MDL -> Model -> MDL pass is a fixpoint.
TEST(FreeTextSanitize_substitutes_and_fixpoint) {
  std::vector<std::string> errs;
  std::string mdl = XmileToMdl(kFreeText, errs);
  CHECK(!mdl.empty());

  // The '|' in the doc must be substituted (to '/'), never emitted raw.
  CHECK(mdl.find("alpha / beta") != std::string::npos);
  CHECK(mdl.find("alpha | beta") == std::string::npos);

  // The units field must not carry a bare '~' in its text (the only '~' bytes
  // in the file should be the field separators).
  CHECK(mdl.find("widget") != std::string::npos);
  CHECK(mdl.find("widget ~") == std::string::npos);

  // MDL -> Model -> MDL must reach a fixpoint (no accreting whitespace on the
  // units/comment fields).
  Model *m1 = ParseMdl(mdl);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  std::vector<std::string> e1;
  std::string mdl2 = m1->PrintMDL(e1);
  delete m1;
  CHECK(!mdl2.empty());

  Model *m2 = ParseMdl(mdl2);
  CHECK(m2 != nullptr);
  if (!m2)
    return;
  std::vector<std::string> e2;
  std::string mdl3 = m2->PrintMDL(e2);
  delete m2;
  CHECK_EQ_STR(mdl2, mdl3);
}
