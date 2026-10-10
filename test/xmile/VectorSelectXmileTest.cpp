// VECTOR SELECT in XMILE.
//
// XMILE has no VECTOR SELECT, so the XMILE writer spells it with array
// builtins over the `Dim.*` its bang (Dim!) subscripts become (FunctionVectorSelect::
// OutputComputable): the missing value when nothing is selected, else the
// reduction of the selected elements -- weighted by the selection for actions
// 0-4, unweighted for 6-10. test_models/VectorSelect.mdl carries the examples
// from Vensim's documentation of the function.
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include "../../src/XMUtil.h"
#include "../TestHarness.h"

namespace {

std::string ReadFile(const std::string &path) {
  std::ifstream in(path, std::ios::in | std::ios::binary);
  if (!in.is_open())
    return std::string();
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

std::string MdlToXmile(const std::string &mdl) {
  char *out = convert_mdl_to_xmile(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "model.mdl", false, -1, false);
  std::string s(out ? out : "");
  free(out);
  return s;
}

// XMILE escapes < and > in element text.
std::string Unescape(std::string s) {
  for (const auto &rep :
       {std::make_pair(std::string("&lt;"), std::string("<")), std::make_pair(std::string("&gt;"), std::string(">"))}) {
    for (size_t p = 0; (p = s.find(rep.first, p)) != std::string::npos;)
      s.replace(p, rep.first.size(), rep.second);
  }
  return s;
}

bool Contains(const std::string &s, const std::string &what) {
  return s.find(what) != std::string::npos;
}

}  // namespace

TEST(VectorSelect_documentation_examples_translate_to_array_builtins) {
  const std::string mdl = ReadFile(std::string(XMUTIL_SRC_ROOT) + "/test_models/VectorSelect.mdl");
  CHECK(!mdl.empty());
  if (mdl.empty())
    return;
  const std::string xmile = Unescape(MdlToXmile(mdl));
  CHECK(!xmile.empty());
  CHECK(!Contains(xmile, "VECTOR SELECT"));
  CHECK(!Contains(xmile, "VECTOR_SELECT"));

  // Action 2, weighted minimum; missing value 1 for a task with no
  // prerequisites.
  CHECK(Contains(xmile,
                 "( IF SUM(IF prereq_map[task, prereq.*] <> 0 THEN 1 ELSE 0) = 0 THEN 1 ELSE "
                 "MIN(IF prereq_map[task, prereq.*] <> 0 THEN prereq_map[task, prereq.*]*task_is_complete[prereq.*] "
                 "ELSE 1e+38) )"));
  // Action 1, weighted product.
  CHECK(Contains(
      xmile, "PROD(IF prereq_map[task, prereq.*] <> 0 THEN prereq_map[task, prereq.*]*task_quality[prereq.*] ELSE 1)"));
  // Action 0, weighted sum, with :NA: (-1e+38) as the missing value; the bang
  // on the first dimension disaggregates.
  CHECK(
      Contains(xmile,
               "( IF SUM(IF age_grouping_map[AgeGroup, Age.*] <> 0 THEN 1 ELSE 0) = 0 THEN -1e+38 ELSE "
               "SUM(IF age_grouping_map[AgeGroup, Age.*] <> 0 THEN age_grouping_map[AgeGroup, Age.*]*population[Age.*] "
               "ELSE 0) )"));
  CHECK(Contains(xmile,
                 "SUM(IF age_grouping_map[AgeGroup.*, Age] <> 0 THEN "
                 "age_grouping_map[AgeGroup.*, Age]*effect_pollution_deaths_by_age_group[AgeGroup.*] ELSE 0)"));
  // A compound expression argument is parenthesized as a whole.
  CHECK(Contains(xmile, "edge_matrix[x, y, edge.*]*(edge_temp[edge.*]-temp[x, y])"));
  // A bang on a dimension mapped to the variable's own names the mapped one.
  CHECK(Contains(xmile, "adjacency_matrix[x, y, xp.*, yp.*]*(temp[xp.*, yp.*]-temp[x, y])"));
}

TEST(VectorSelect_every_numerical_action) {
  // One selection, one expression, each numerical action; 5 has no XMILE
  // equivalent and is written as is.
  std::string mdl =
      "{UTF-8}\n"
      "d : d1, d2, d3\n\t~\t\n\t~\t\n\t|\n"
      "s[d] = 1, 0, 2\n\t~\t\n\t~\t\n\t|\n"
      "e[d] = 4, 5, 6\n\t~\t\n\t~\t\n\t|\n";
  for (int action = 0; action <= 10; action++)
    mdl += "a" + std::to_string(action) + " = VECTOR SELECT(s[d!], e[d!], 7, " + std::to_string(action) +
           ", 0)\n\t~\t\n\t~\t\n\t|\n";
  mdl +=
      "INITIAL TIME = 0\n\t~\t\n\t~\t\n\t|\n"
      "FINAL TIME = 1\n\t~\t\n\t~\t\n\t|\n"
      "TIME STEP = 1\n\t~\t\n\t~\t\n\t|\n"
      "SAVEPER = 1\n\t~\t\n\t~\t\n\t|\n";
  const std::string xmile = Unescape(MdlToXmile(mdl));
  CHECK(!xmile.empty());
  const std::string none = "( IF SUM(IF s[d.*] <> 0 THEN 1 ELSE 0) = 0 THEN 7 ELSE ";
  const char *const expected[] = {
      "SUM(IF s[d.*] <> 0 THEN s[d.*]*e[d.*] ELSE 0) )",
      "PROD(IF s[d.*] <> 0 THEN s[d.*]*e[d.*] ELSE 1) )",
      "MIN(IF s[d.*] <> 0 THEN s[d.*]*e[d.*] ELSE 1e+38) )",
      "MAX(IF s[d.*] <> 0 THEN s[d.*]*e[d.*] ELSE -1e+38) )",
      "SUM(IF s[d.*] <> 0 THEN s[d.*]*e[d.*] ELSE 0)/SUM(IF s[d.*] <> 0 THEN 1 ELSE 0) )",
      nullptr,
      "SUM(IF s[d.*] <> 0 THEN e[d.*] ELSE 0) )",
      "PROD(IF s[d.*] <> 0 THEN e[d.*] ELSE 1) )",
      "MIN(IF s[d.*] <> 0 THEN e[d.*] ELSE 1e+38) )",
      "MAX(IF s[d.*] <> 0 THEN e[d.*] ELSE -1e+38) )",
      "SUM(IF s[d.*] <> 0 THEN e[d.*] ELSE 0)/SUM(IF s[d.*] <> 0 THEN 1 ELSE 0) )",
  };
  for (int action = 0; action <= 10; action++) {
    const std::string tag = "<aux name=\"a" + std::to_string(action) + "\">";
    size_t at = xmile.find(tag);
    CHECK(at != std::string::npos);
    if (at == std::string::npos)
      continue;
    const std::string eqn = xmile.substr(at, xmile.find("</eqn>", at) - at);
    if (expected[action]) {
      if (!Contains(eqn, none + expected[action]))
        printf("  action %d: %s\n", action, eqn.c_str());
      CHECK(Contains(eqn, none + expected[action]));
    } else {
      CHECK(Contains(eqn, "VECTOR SELECT("));  // action 5: no equivalent, written as is
    }
  }
}

// On input `Dim.*`, `*:Dim` and a bare `*` (bound to the variable's own
// dimension there) are all a Vensim `Dim!` bang subscript, so all three come
// out of .mdl the same way. Only VECTOR SELECT writes `Dim.*`.
TEST(VectorSelect_xmile_dimension_wildcards_read_as_bang) {
  const char *kXmile = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>1</stop><dt>1</dt></sim_specs>
  <dimensions><dim name="d"><elem name="d1"/><elem name="d2"/></dim></dimensions>
  <model>
    <variables>
      <aux name="a"><dimensions><dim name="d"/></dimensions><eqn>1</eqn></aux>
      <aux name="dot star"><eqn>SUM(a[d.*])</eqn></aux>
      <aux name="star colon"><eqn>SUM(a[*:d])</eqn></aux>
      <aux name="bare star"><eqn>SUM(a[*])</eqn></aux>
    </variables>
  </model>
</xmile>
)";
  char *out = convert_xmile_to_mdl(kXmile, static_cast<uint32_t>(std::string(kXmile).size()), "w.xmile", -1);
  CHECK(out != nullptr);
  const std::string mdl(out ? out : "");
  free(out);
  CHECK(Contains(mdl, "dot star = SUM(a[d!])"));
  CHECK(Contains(mdl, "star colon = SUM(a[d!])"));
  CHECK(Contains(mdl, "bare star = SUM(a[d!])"));
  // Outside VECTOR SELECT, XMILE written back spells each of them `*`.
  char *again = convert_xmile_to_xmile(kXmile, static_cast<uint32_t>(std::string(kXmile).size()), "w.xmile", -1, false);
  CHECK(again != nullptr);
  const std::string xmile(again ? again : "");
  free(again);
  size_t n = 0;
  for (size_t p = 0; (p = xmile.find("SUM(a[*])", p)) != std::string::npos; p++)
    n++;
  CHECK(n == 3);
}
// .mdl -> XMILE -> .mdl. XMILE has no VECTOR SELECT and the reader does not
// guess one back from its translation (an ordinary equation of the same shape
// would be turned into one), so each comes back as that translation -- the
// same values, in Vensim's SUM / PROD / VMIN over IF THEN ELSE with the bang
// subscripts restored. The element references the model compares against
// (`edge = edge.top` in the XMILE) come back as the elements.
TEST(VectorSelect_round_trip_comes_back_as_its_translation) {
  const std::string mdl = ReadFile(std::string(XMUTIL_SRC_ROOT) + "/test_models/VectorSelect.mdl");
  CHECK(!mdl.empty());
  if (mdl.empty())
    return;
  const std::string xmile = MdlToXmile(mdl);
  char *out = convert_xmile_to_mdl(xmile.c_str(), static_cast<uint32_t>(xmile.size()), "v.xmile", -1);
  CHECK(out != nullptr);
  const std::string back(out ? out : "");
  free(out);
  if (back.empty())
    return;
  // Long equations are wrapped, at a space that may or may not survive the
  // break; compare with all whitespace and continuations taken out.
  auto squash = [](const std::string &s) {
    std::string out;
    for (char c : s)
      if (c != ' ' && c != '\t' && c != '\r' && c != '\n' && c != '\\')
        out += c;
    return out;
  };
  const std::string flat = squash(back);
  CHECK(!Contains(back, "VECTOR SELECT"));
  CHECK(Contains(flat, squash("SUM(IF THEN ELSE(prereq map[task, prereq!] <> 0, 1, 0))")));
  CHECK(Contains(flat, squash("VMIN(IF THEN ELSE(prereq map[task, prereq!] <> 0,")));
  CHECK(Contains(flat, squash("PROD(IF THEN ELSE(prereq map[task, prereq!] <> 0,")));
  CHECK(Contains(flat, squash("(edge = top)")));
  CHECK(Contains(flat, squash("(edge = bottom)")));
  // A bang over a dimension mapped to the variable's own keeps naming it.
  CHECK(Contains(flat, squash("task quality[prereq!]")));
  // From there on the translation is a fixpoint.
  const std::string xmile2 = MdlToXmile(back);
  char *again = convert_xmile_to_mdl(xmile2.c_str(), static_cast<uint32_t>(xmile2.size()), "v.xmile", -1);
  CHECK(again != nullptr);
  const std::string back2(again ? again : "");
  free(again);
  CHECK(back2 == back);
}