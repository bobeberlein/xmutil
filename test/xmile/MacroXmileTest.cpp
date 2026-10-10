// Macros between Vensim and XMILE.
//
// Inside a Vensim macro, `name$` names the model's own `name` rather than a
// macro-local one. XMILE has no such reference: the time variables are
// builtins it knows by their plain names (TIME, DT, STARTTIME, STOPTIME), and
// anything else is simply named. So the .mdl reader drops the `$` (and marks
// the variable), the XMILE writer names the time variables as builtins, the
// XMILE reader reads <macro> back into a Vensim macro, and the .mdl writer
// restores the `$` on the time variables and on anything marked.
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include "../../src/Function/Function.h"
#include "../../src/Model.h"
#include "../../src/Symbol/ExpressionList.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/Symbol/Variable.h"
#include "../../src/XMUtil.h"
#include "../TestHarness.h"
#include "../mdl/RoundTrip.h"

namespace {

std::string ReadFile(const std::string &path) {
  std::ifstream in(path, std::ios::in | std::ios::binary);
  if (!in.is_open())
    return std::string();
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

const char *kSimpleMacroPath = "/test_models/SimpleMacro.mdl";

std::string MdlToXmile(const std::string &mdl) {
  char *out = convert_mdl_to_xmile(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "model.mdl", false, -1, false);
  std::string s(out ? out : "");
  free(out);
  return s;
}

std::string XmileToMdl(const std::string &xmile) {
  char *out = convert_xmile_to_mdl(xmile.c_str(), static_cast<uint32_t>(xmile.size()), "model.xmile", -1);
  std::string s(out ? out : "");
  free(out);
  return s;
}

std::string MdlToMdl(const std::string &mdl) {
  char *out = convert_to_mdl(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "model.mdl", -1);
  std::string s(out ? out : "");
  free(out);
  return s;
}

// The text between the first `start` and the following `end` (exclusive).
std::string Between(const std::string &s, const std::string &start, const std::string &end) {
  size_t a = s.find(start);
  if (a == std::string::npos)
    return std::string();
  size_t b = s.find(end, a + start.size());
  return s.substr(a, b == std::string::npos ? std::string::npos : b - a);
}

bool Contains(const std::string &s, const std::string &what) {
  return s.find(what) != std::string::npos;
}

}  // namespace

// .mdl -> XMILE: the macro body names the time variables as XMILE builtins, and
// no `$` reaches the XMILE.
TEST(MacroXmile_simple_macro_writes_builtins_without_dollar) {
  const std::string mdl = ReadFile(std::string(XMUTIL_SRC_ROOT) + kSimpleMacroPath);
  CHECK(!mdl.empty());
  if (mdl.empty())
    return;
  const std::string xmile = MdlToXmile(mdl);
  const std::string macro = Between(xmile, "<macro", "</macro>");
  CHECK(!macro.empty());
  CHECK(Contains(macro, "IF TIME &lt;= tstart"));  // Time$ -> TIME
  CHECK(Contains(macro, "MAX(tend-tstart, DT)"));  // TIME STEP$ -> DT
  CHECK(!Contains(macro, "$"));
  CHECK(!Contains(macro, "TIME_STEP"));
  // The model calls the macro by name.
  CHECK(Contains(xmile, "RAMP_FROM_TO(y_start, y_end, x_start, x_end, is_linear)"));
}

// .mdl -> XMILE -> .mdl: the macro comes back with its arguments and body, the
// time variables as Time$ / TIME STEP$, and the model still calling it. A
// second trip reproduces the .mdl exactly.
TEST(MacroXmile_simple_macro_round_trips) {
  const std::string mdl = ReadFile(std::string(XMUTIL_SRC_ROOT) + kSimpleMacroPath);
  CHECK(!mdl.empty());
  if (mdl.empty())
    return;
  const std::string back = XmileToMdl(MdlToXmile(mdl));
  CHECK(!back.empty());
  if (back.empty())
    return;
  const std::string macro = Between(back, ":MACRO:", ":END OF MACRO:");
  CHECK(Contains(macro, ":MACRO: RAMP FROM TO(xfrom, xto, tstart, tend, islinear)"));
  CHECK(Contains(macro, "IF THEN ELSE(Time$ <= tstart"));
  CHECK(Contains(macro, "MAX(tend - tstart, TIME STEP$)"));
  CHECK(Contains(back, "uses ramp = RAMP FROM TO(y start, y end, x start, x end, is linear)"));

  // The macro's body is the original body: the same variables, each defined.
  Model *a = roundtrip::ParseVensim(mdl);
  Model *b = roundtrip::ParseVensim(back);
  CHECK(a && b);
  if (a && b) {
    CHECK(a->MacroFunctions().size() == 1 && b->MacroFunctions().size() == 1);
    if (a->MacroFunctions().size() == 1 && b->MacroFunctions().size() == 1) {
      MacroFunction *ma = a->MacroFunctions()[0];
      MacroFunction *mb = b->MacroFunctions()[0];
      CHECK_EQ_STR(mb->GetName(), ma->GetName());
      CHECK(mb->Args()->Length() == ma->Args()->Length());
      for (Variable *v : a->GetVariables(ma->NameSpace())) {
        if (v->GetAllEquations().empty())
          continue;  // parameters and the time builtins
        Symbol *s = mb->NameSpace()->Find(v->GetName());
        CHECK(s && s->isType() == Symtype_Variable && !static_cast<Variable *>(s)->GetAllEquations().empty());
        if (!s)
          printf("  macro body variable missing: %s\n", v->GetName().c_str());
      }
    }
  }
  delete a;
  delete b;

  CHECK(XmileToMdl(MdlToXmile(back)) == back);
}

// An XMILE macro names the time builtins plainly; written as .mdl each takes
// Vensim's `$` inside the macro -- and only there.
TEST(MacroXmile_xmile_macro_time_builtins_get_dollar) {
  const char *kXmile = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>0.5</dt></sim_specs>
  <model>
    <variables>
      <aux name="clock"><eqn>elapsed(2)</eqn></aux>
      <aux name="now"><eqn>TIME</eqn></aux>
    </variables>
  </model>
  <macro name="elapsed">
    <eqn>elapsed</eqn>
    <parm>offset</parm>
    <variables>
      <aux name="elapsed"><eqn>(TIME - STARTTIME + offset) / (STOPTIME - STARTTIME) + DT * SAVEPER</eqn></aux>
    </variables>
  </macro>
</xmile>
)";
  const std::string mdl = XmileToMdl(kXmile);
  CHECK(!mdl.empty());
  const std::string macro = Between(mdl, ":MACRO:", ":END OF MACRO:");
  CHECK(Contains(macro, ":MACRO: elapsed(offset)"));
  CHECK(Contains(macro, "Time$"));
  CHECK(Contains(macro, "INITIAL TIME$"));
  CHECK(Contains(macro, "FINAL TIME$"));
  CHECK(Contains(macro, "TIME STEP$"));
  CHECK(Contains(macro, "SAVEPER$"));
  CHECK(!Contains(macro, "offset$"));  // a parameter is the macro's own
  // Outside the macro the model's own variables are named plainly.
  CHECK(Contains(mdl, "clock = elapsed(2)"));
  CHECK(Contains(mdl, "now = Time"));
  CHECK(!Contains(mdl, "now = Time$"));
  // And the .mdl reads back with the macro intact.
  Model *m = roundtrip::ParseVensim(mdl);
  CHECK(m && m->MacroFunctions().size() == 1);
  delete m;
}

// A macro may refer to any of the model's variables as `name$`, not just the
// time builtins. XMILE gets the plain name; .mdl -> .mdl keeps the `$`.
TEST(MacroXmile_mdl_global_reference_keeps_its_dollar) {
  const char *kMdl =
      "{UTF-8}\n"
      ":MACRO: SCALED(x)\n"
      "SCALED = x * scale$\n"
      "\t~\t\n\t~\t\n\t|\n"
      ":END OF MACRO:\n"
      "scale = 3\n\t~\t\n\t~\t\n\t|\n"
      "y = SCALED(2)\n\t~\t\n\t~\t\n\t|\n"
      "INITIAL TIME = 0\n\t~\t\n\t~\t\n\t|\n"
      "FINAL TIME = 10\n\t~\t\n\t~\t\n\t|\n"
      "TIME STEP = 1\n\t~\t\n\t~\t\n\t|\n"
      "SAVEPER = 1\n\t~\t\n\t~\t\n\t|\n";
  const std::string back = MdlToMdl(kMdl);
  CHECK(Contains(Between(back, ":MACRO:", ":END OF MACRO:"), "SCALED = x * scale$"));
  const std::string xmile = MdlToXmile(kMdl);
  const std::string macro = Between(xmile, "<macro", "</macro>");
  CHECK(Contains(macro, "<eqn>x*scale</eqn>"));
  CHECK(!Contains(macro, "$"));
}
