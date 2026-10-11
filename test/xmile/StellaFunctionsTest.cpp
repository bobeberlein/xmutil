// Stella builtins without a Vensim builtin of the same name, as met in FRIDA.
//
// PREVIOUS and MEAN have exact Vensim forms (DELAY FIXED over one TIME STEP;
// SUM over ELMCOUNT). The rest -- ALLOCATE, MINSOFT, RUNCUM, ... -- are kept as
// written, NAME(args), for Vensim macros of the same name to supply, with a
// warning per call. Also here: SELF, a parameterless macro called by its bare
// name, quoted names in a <connect>, and a comment-only <eqn>.
#include <cstdlib>
#include <string>
#include <vector>

#include "../../src/Mdl/MDLFormat.h"
#include "../../src/Model.h"
#include "../../src/XMUtil.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

bool Contains(const std::string &hay, const std::string &needle) {
  return hay.find(needle) != std::string::npos;
}

std::string Xmile(const std::string &variables, const std::string &extra = "") {
  return R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>0.25</dt></sim_specs>
  <dimensions>
    <dim name="R"><elem name="r1"/><elem name="r2"/></dim>
    <dim name="C"><elem name="c1"/><elem name="c2"/><elem name="c3"/></dim>
  </dimensions>
  )" + extra +
         R"(
  <model><variables>
    <aux name="x"><eqn>TIME</eqn></aux>
)" + variables +
         R"(
  </variables></model>
</xmile>
)";
}

std::string ToMdl(const std::string &xmile) {
  char *out = convert_xmile_to_mdl(xmile.c_str(), static_cast<uint32_t>(xmile.size()), "s.xmile", -1);
  const std::string mdl(out ? out : "");
  free(out);
  return mdl;
}

std::vector<std::string> ParseErrors(const std::string &xmile) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(xmile.c_str(), errs);
  delete m;
  return errs;
}

}  // namespace

TEST(StellaFunctions_previous_is_delay_fixed_over_one_dt) {
  const std::string mdl = ToMdl(Xmile(R"(
    <aux name="lagged"><eqn>PREVIOUS(x)</eqn></aux>
    <aux name="lagged init"><eqn>PREVIOUS(x, 7)</eqn></aux>
    <aux name="counter"><eqn>PREVIOUS(SELF, 0) + DT</eqn></aux>
    <aux name="arr"><dimensions><dim name="R"/></dimensions><eqn>PREVIOUS(SELF, 1)*2</eqn></aux>
)"));
  CHECK(Contains(mdl, "lagged = DELAY FIXED(x, TIME STEP, 0)"));
  CHECK(Contains(mdl, "lagged init = DELAY FIXED(x, TIME STEP, 7)"));
  // SELF is the variable itself, with its own subscripts when arrayed.
  CHECK(Contains(mdl, "counter = DELAY FIXED(counter, TIME STEP, 0) + TIME STEP"));
  CHECK(Contains(mdl, "arr[R] = DELAY FIXED(arr[R], TIME STEP, 1) * 2"));
}

TEST(StellaFunctions_mean_is_sum_over_count) {
  const std::string mdl = ToMdl(Xmile(R"(
    <aux name="arr"><dimensions><dim name="R"/></dimensions><eqn>1</eqn></aux>
    <aux name="m2"><dimensions><dim name="R"/><dim name="C"/></dimensions><eqn>1</eqn></aux>
    <aux name="avg"><eqn>MEAN(arr[*])</eqn></aux>
    <aux name="avg2"><eqn>MEAN(m2[*, *])</eqn></aux>
    <aux name="avg3"><eqn>2^MEAN(x, 4, 3)</eqn></aux>
)"));
  CHECK(Contains(mdl, "avg = (SUM(arr[R!]) / ELMCOUNT(R))"));
  CHECK(Contains(mdl, "avg2 = (SUM(m2[R!, C!]) / (ELMCOUNT(R) * ELMCOUNT(C)))"));
  CHECK(Contains(mdl, "avg3 = 2 ^ ((x + 4 + 3) / 3)"));
}

TEST(StellaFunctions_stella_only_calls_are_kept_for_macros) {
  const std::string xmile = Xmile(R"(
    <aux name="soft"><eqn>MINSOFT(x, 5, 0.5) + RUNCUM(x)</eqn></aux>
)");
  const std::string mdl = ToMdl(xmile);
  CHECK(Contains(mdl, "soft = MINSOFT(x, 5, 0.5) + RUNCUM(x)"));
  int warnings = 0;
  for (const std::string &e : ParseErrors(xmile)) {
    if (Contains(e, "has no Vensim equivalent"))
      warnings++;
  }
  CHECK(warnings == 2);
}

TEST(StellaFunctions_parameterless_macro_called_by_bare_name) {
  const std::string mdl = ToMdl(Xmile(R"(
    <aux name="floor"><eqn>MAX(TINY, x)</eqn></aux>
)",
                                      R"(<macro name="TINY"><eqn>1e-12</eqn></macro>)"));
  // Vensim macros take at least one parameter: an unused one, passed 0.
  CHECK(Contains(mdl, ":MACRO: TINY(unused input)"));
  CHECK(Contains(mdl, "floor = MAX(TINY(0), x)"));
  CHECK(!Contains(mdl, "TINY 1"));
  // ...which is Vensim xmutil reads back, to the same text.
  char *again = convert_to_mdl(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "t.mdl", -1);
  CHECK(again != nullptr);
  CHECK(Contains(again ? again : "", "floor = MAX(TINY(0), x)"));
  free(again);
}

// Stella quotes a name part that is not a plain identifier, and leaves an
// unconnected input's equation as a comment.
TEST(StellaFunctions_quoted_connect_and_comment_only_eqn) {
  const std::string xmile = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="src$"><eqn>3</eqn></aux>
    <module name="A"><connect to='A."in$"' from='."src$"'/></module>
  </variables></model>
  <model name="A"><variables>
    <aux name="in$" access="input"><eqn>{Enter equation for use when not hooked up}</eqn></aux>
    <aux name="y"><eqn>"in$"*2</eqn></aux>
    <aux name="unused"><eqn>{nothing here}</eqn></aux>
  </variables></model>
</xmile>
)";
  const std::string mdl = ToMdl(xmile);
  CHECK(Contains(mdl, "y = \"src$\" * 2"));
  bool sawEmpty = false;
  for (const std::string &e : ParseErrors(xmile)) {
    CHECK(!Contains(e, "syntax error"));
    if (Contains(e, "has neither <eqn> nor <gf>"))
      sawEmpty = true;
  }
  CHECK(sawEmpty);
}

// Vensim spells out every arrayed reference's subscripts; XMILE may leave them
// off. The .mdl writer takes the equation's own (the element, in a per-element
// equation), else the variable's dimensions -- banged inside an array function.
TEST(StellaFunctions_bare_arrayed_reference_gets_its_subscripts) {
  const std::string mdl = ToMdl(Xmile(R"(
    <aux name="arr"><dimensions><dim name="R"/></dimensions><eqn>1</eqn></aux>
    <aux name="m"><dimensions><dim name="R"/><dim name="C"/></dimensions><eqn>arr*2</eqn></aux>
    <aux name="total"><eqn>SUM(arr) + MEAN(arr)</eqn></aux>
    <aux name="per"><dimensions><dim name="R"/></dimensions>
      <element subscript="r1"><eqn>arr</eqn></element>
      <element subscript="r2"><eqn>arr*3</eqn></element>
    </aux>
)"));
  CHECK(Contains(mdl, "m[R, C] = arr[R] * 2"));
  CHECK(Contains(mdl, "total = SUM(arr[R!]) + (SUM(arr[R!]) / ELMCOUNT(R))"));
  CHECK(Contains(mdl, "per[r1] = arr[r1]"));
  CHECK(Contains(mdl, "per[r2] = arr[r2] * 3"));
}

// Vensim macros have no defaults: a call leaving out a <parm default> gets it.
TEST(StellaFunctions_macro_call_gets_parm_defaults) {
  const std::string mdl = ToMdl(Xmile(R"(
    <aux name="g"><eqn>GROW(x) + GROW(x, 1)</eqn></aux>
)",
                                      R"(<macro name="GROW"><eqn>input*(1+rate)</eqn>
    <parm>input</parm><parm default="0.5">rate</parm></macro>)"));
  CHECK(Contains(mdl, "g = GROW(x, 0.5) + GROW(x, 1)"));
}

TEST(StellaFunctions_log10_is_vensim_log_base_10) {
  const std::string mdl = ToMdl(Xmile(R"(
    <aux name="decades"><eqn>LOG10(x + 1)</eqn></aux>
)"));
  CHECK(Contains(mdl, "decades = LOG(x + 1, 10)"));
}

// Vensim units have no powers and no %: written out as products and Percent.
TEST(StellaFunctions_units_powers_and_percent) {
  CHECK(mdl::VensimUnitsText("kmol/(metre^2*year*ppm)") == "kmol/((metre*metre)*year*ppm)");
  CHECK(mdl::VensimUnitsText("1/deg C^2") == "1/(deg C*deg C)");
  CHECK(mdl::VensimUnitsText("W*yr/(metre^2*Mt VOC)") == "W*yr/((metre*metre)*Mt VOC)");
  CHECK(mdl::VensimUnitsText("(m/s)^2") == "((m/s)*(m/s))");
  CHECK(mdl::VensimUnitsText("year^-1") == "1/(year)");
  CHECK(mdl::VensimUnitsText("m^3/s") == "(m*m*m)/s");
  CHECK(mdl::VensimUnitsText("%") == "Percent");
  CHECK(mdl::VensimUnitsText("%/year") == "Percent/year");
  CHECK(mdl::VensimUnitsText("m^0.5") == "m^0.5");  // not a whole power
  CHECK(mdl::VensimUnitsText("people/year") == "people/year");

  // Through a conversion, and back in as Vensim.
  const std::string mdl = ToMdl(Xmile(R"(
    <aux name="area"><eqn>1</eqn><units>metre^2</units></aux>
    <aux name="share"><eqn>1</eqn><units>%</units></aux>
)"));
  CHECK(Contains(mdl, "\t~\t(metre*metre)\n") || Contains(mdl, "\t~\t(metre*metre)\r\n"));
  CHECK(Contains(mdl, "\t~\tPercent\n") || Contains(mdl, "\t~\tPercent\r\n"));
  char *again = convert_to_mdl(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "u.mdl", -1);
  CHECK(again != nullptr);
  CHECK(Contains(again ? again : "", "area = 1"));
  free(again);
}

// An equation that is NAN alone means A FUNCTION OF() only in a model saved on
// the causal loop diagram layer; elsewhere it is the missing value. NAN(inputs)
// is A FUNCTION OF(inputs) either way.
TEST(StellaFunctions_bare_nan_depends_on_cld_layer) {
  const char *vars = R"(
    <aux name="later"><eqn>NAN</eqn></aux>
    <aux name="depends"><eqn>NAN(x)</eqn></aux>
)";
  const std::string onModel = ToMdl(Xmile(vars, R"(<isee:prefs layer="model"/>)"));
  CHECK(Contains(onModel, "later = :NA:"));
  CHECK(Contains(onModel, "depends = A FUNCTION OF(x)"));
  const std::string onCld = ToMdl(Xmile(vars, R"(<isee:prefs layer="cld"/>)"));
  CHECK(Contains(onCld, "later = A FUNCTION OF()"));
  CHECK(Contains(onCld, "depends = A FUNCTION OF(x)"));
}

// The writer saves a model that is mostly A FUNCTION OF on the CLD layer.
TEST(StellaFunctions_mostly_a_function_of_is_saved_on_cld_layer) {
  auto layerOf = [](const char *mdl) {
    char *out = convert_mdl_to_xmile(mdl, static_cast<uint32_t>(std::string(mdl).size()), "c.mdl", false, 0, false);
    const std::string xmile(out ? out : "");
    free(out);
    return Contains(xmile, "layer=\"cld\"") ? "cld" : Contains(xmile, "layer=\"model\"") ? "model" : "?";
  };
  const char *mostly =
      "{UTF-8}\na = A FUNCTION OF(b)\n\t~\t\n\t~\t\n\t|\n\nb = A FUNCTION OF()\n\t~\t\n\t~\t\n\t|\n\n"
      "c = 1\n\t~\t\n\t~\t\n\t|\n\n";
  const char *half = "{UTF-8}\na = A FUNCTION OF(b)\n\t~\t\n\t~\t\n\t|\n\nb = 2\n\t~\t\n\t~\t\n\t|\n\n";
  CHECK(std::string(layerOf(mostly)) == "cld");
  CHECK(std::string(layerOf(half)) == "model");
}
