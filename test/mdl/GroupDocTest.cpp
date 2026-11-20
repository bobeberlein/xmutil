// A Vensim group banner carries a name line and, between the closing bar's '~'
// and the '|', the group's documentation:
//
//   ********************************************************
//   	.Financial Results
//   ********************************************************~
//   	Variables relating to financial performance
//   	|
//
// The reader used to cut the name at its first space (".Financial Results"
// became "Financial", colliding with any real "Financial" group) and threw the
// documentation away. Both now survive into ModelGroup and out through the
// .mdl banner, the XMILE <group>'s <doc> (sector form) and the <module>'s
// <doc> (module form).

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/XMUtil.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

ModelGroup *GroupNamed(Model *m, const std::string &name) {
  for (ModelGroup *g : m->Groups()) {
    if (g->sName == name)
      return g;
  }
  return nullptr;
}

std::string Banner(const char *name, const char *doc) {
  const std::string bar(56, '*');
  return "\r\n" + bar + "\r\n\t" + name + "\r\n" + bar + "~\r\n\t" + doc + "\r\n\t|\r\n";
}

std::string Model2() {
  return "{UTF-8}\r\n" + Banner(".Financial", "Variables relating to financial performance") +
         "revenue = 100\r\n\t~~|\r\n" + Banner(".Financial Results", "First line\r\n\tsecond line") +
         "profit = revenue - 1\r\n\t~~|\r\n" + Banner(".Quiet", "") + "q = 3\r\n\t~~|\r\n";
}

std::string CommentOf(Model *m, const std::string &group) {
  ModelGroup *g = GroupNamed(m, group);
  return g ? g->sComment : "<missing>";
}

std::string Take(char *s) {
  std::string out = s ? s : "";
  free(s);
  return out;
}

}  // namespace

TEST(GroupDoc_name_keeps_its_spaces) {
  Model *m = roundtrip::ParseVensim(Model2());
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(GroupNamed(m, "Financial Results") != nullptr);
  CHECK(GroupNamed(m, "Financial 1") == nullptr);
  ModelGroup *g = GroupNamed(m, "Financial Results");
  CHECK(g && g->vVariables.size() == 1);
  delete m;
}

TEST(GroupDoc_banner_comment_is_read) {
  Model *m = roundtrip::ParseVensim(Model2());
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(CommentOf(m, "Financial"), "Variables relating to financial performance");
  // Line breaks are kept (CRLF folded to LF); a banner with no text has none,
  // and does not inherit the previous banner's.
  CHECK_EQ_STR(CommentOf(m, "Financial Results"), "First line\n\tsecond line");
  CHECK_EQ_STR(CommentOf(m, "Quiet"), "");
  delete m;
}

TEST(GroupDoc_survives_mdl_round_trip_as_a_fixpoint) {
  Model *m = roundtrip::ParseVensim(Model2());
  CHECK(m != nullptr);
  if (!m)
    return;
  std::vector<std::string> errs;
  const std::string out = m->PrintMDL(errs);
  delete m;
  CHECK(out.find("Variables relating to financial performance") != std::string::npos);

  Model *m2 = roundtrip::ParseVensim(out);
  CHECK(m2 != nullptr);
  if (!m2)
    return;
  CHECK_EQ_STR(CommentOf(m2, "Financial"), "Variables relating to financial performance");
  CHECK_EQ_STR(CommentOf(m2, "Financial Results"), "First line\n\tsecond line");
  const std::string out2 = m2->PrintMDL(errs);
  delete m2;

  // Compared from the second write on: this model has no .Control section, and
  // the first write synthesizes one in a form the second normalizes. The
  // banners themselves are stable from the first write.
  Model *m3 = roundtrip::ParseVensim(out2);
  CHECK(m3 != nullptr);
  if (!m3)
    return;
  const std::string out3 = m3->PrintMDL(errs);
  delete m3;
  CHECK(out2 == out3);
  const std::string banners = out.substr(0, out.find(".Control"));
  CHECK(banners == out2.substr(0, out2.find(".Control")));
}

TEST(GroupDoc_travels_through_xmile_both_forms) {
  const std::string mdl = Model2();
  const uint32_t len = static_cast<uint32_t>(mdl.size());

  // Module form (the default with two or more groups): <doc> on the <module>.
  std::string modules = Take(convert_mdl_to_xmile(mdl.c_str(), len, "g.mdl", false, 0, /*isAsSectors=*/false));
  CHECK(modules.find("<module name=\"Financial\">\n                <doc>Variables relating to financial "
                     "performance</doc>") != std::string::npos);
  CHECK(modules.find("<module name=\"Financial Results\">") != std::string::npos);

  // Sector form, no sketch: <doc> on the <group>, and back again to .mdl.
  std::string sectors = Take(convert_mdl_to_xmile(mdl.c_str(), len, "g.mdl", false, 0, /*isAsSectors=*/true));
  CHECK(sectors.find("<group name=\"Financial\">\n                <doc>Variables relating to financial "
                     "performance</doc>") != std::string::npos);
  std::string back = Take(convert_xmile_to_mdl(sectors.c_str(), static_cast<uint32_t>(sectors.size()), "g.xmile", 0));
  Model *m = roundtrip::ParseVensim(back);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(CommentOf(m, "Financial"), "Variables relating to financial performance");
  CHECK_EQ_STR(CommentOf(m, "Financial Results"), "First line\n\tsecond line");
  delete m;
}

// XMILE allows a '|' in a group's <doc>; in a .mdl banner it would end the
// banner early, so the writer substitutes it like any modeler text.
TEST(GroupDoc_pipe_in_xmile_doc_is_sanitized_for_mdl) {
  const char *xmile =
      "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
      "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\">\n"
      "  <sim_specs><start>0</start><stop>10</stop><dt>1</dt></sim_specs>\n"
      "  <model><variables><aux name=\"x\"><eqn>1</eqn></aux></variables>\n"
      "    <views><group name=\"G\"><doc>left | right</doc><var>x</var></group></views>\n"
      "  </model>\n</xmile>\n";
  std::string mdl = Take(convert_xmile_to_mdl(xmile, static_cast<uint32_t>(strlen(xmile)), "g.xmile", 0));
  CHECK(mdl.find("left / right") != std::string::npos);
  Model *m = roundtrip::ParseVensim(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(CommentOf(m, "G"), "left / right");
  ModelGroup *g = GroupNamed(m, "G");
  CHECK(g && g->vVariables.size() == 1);
  delete m;
}
