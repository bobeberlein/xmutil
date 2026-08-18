// Regression tests for TABXL extrapolate preservation in the .mdl writer (#854).
//
// An XMILE <gf type="extrapolate"> is marked on read (ExpressionTable::Extrapolate).
// Vensim has no definition-level extrapolate flag: the ONLY way to preserve it
// through .mdl is a TABXL(table, input) call site, which re-imports as
// extrapolating (a plain table(input) re-imports as continuous). These tests
// pin that a referenced standalone extrapolating lookup emits TABXL and that the
// flag survives an XMILE -> MDL -> Model round trip.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Equation.h"
#include "../../src/Symbol/Expression.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/Symbol/Variable.h"
#include "../../src/Vensim/VensimParse.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

// f is a standalone extrapolating graphical function; user references it as a
// lookup call, giving the call site TABXL can mark.
const char *kReferencedExtrapolate = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <header><name>extrap</name></header>
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="input"><eqn>5</eqn></aux>
    <aux name="f">
      <gf type="extrapolate">
        <xpts>0,5,10</xpts>
        <ypts>0,3,10</ypts>
      </gf>
    </aux>
    <aux name="user"><eqn>f(input)</eqn></aux>
  </variables></model>
</xmile>
)";

std::string XmileToMdl(const char *xmile, std::vector<std::string> &errs) {
  Model *m = xmileroundtrip::ParseXMILE(xmile, errs);
  if (!m)
    return "";
  errs.clear();
  std::string mdl = m->PrintMDL(errs);
  delete m;
  return mdl;
}

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

// Whether the standalone lookup variable `name` re-imported as extrapolating.
bool LookupIsExtrapolating(Model *m, const std::string &name) {
  Symbol *s = m->GetNameSpace()->Find(name);
  if (!s || s->isType() != Symtype_Variable)
    return false;
  Variable *v = static_cast<Variable *>(s);
  if (v->GetAllEquations().empty())
    return false;
  Expression *rhs = v->GetEquation(0)->GetExpression();
  if (!rhs || rhs->GetType() != EXPTYPE_Table)
    return false;
  return static_cast<ExpressionTable *>(rhs)->Extrapolate();
}

}  // namespace

TEST(Extrapolate_referenced_standalone_emits_tabxl) {
  std::vector<std::string> errs;
  std::string mdl = XmileToMdl(kReferencedExtrapolate, errs);
  CHECK(!mdl.empty());

  // The call site must be TABXL(f, input), not a plain f(input) continuous call.
  CHECK(mdl.find("TABXL(f, input)") != std::string::npos);
  CHECK(mdl.find("f(input)") == std::string::npos);
}

TEST(Extrapolate_survives_xmile_to_mdl_to_model) {
  std::vector<std::string> errs;
  std::string mdl = XmileToMdl(kReferencedExtrapolate, errs);
  CHECK(!mdl.empty());

  Model *m = ParseMdl(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  // The TABXL call site must re-mark the referenced lookup as extrapolating.
  CHECK(LookupIsExtrapolating(m, "f"));
  delete m;
}
