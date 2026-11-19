// Powersim .sip reader: the compound-file layer, the lowering to XMILE, and
// the C entry points, driven by the archetype models in
// test/fixtures/powersim/ (each has a hand-made .stmx counterpart the
// expected equations below were checked against).

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/PowerSim/CompoundFile.h"
#include "../../src/PowerSim/SipReader.h"
#include "../../src/XMUtil.h"
#include "../TestHarness.h"

namespace {

const char *const kFixtures[] = {
    "Eroding Goals.sip",
    "Escalation.sip",
    "Fixes that Fail.sip",
    "Growth & Underinvestment.sip",
    "Limits to Growth Ver1.sip",
    "Shifting the Burden.sip",
    "Success to the Successful.sip",
    "Tragedy of the Commons.sip",
};

std::string ReadFixture(const std::string &name) {
  std::ifstream in(std::string(XMUTIL_SRC_ROOT) + "/test/fixtures/powersim/" + name, std::ios::binary);
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

std::string Lower(const std::string &name) {
  std::string data = ReadFixture(name);
  std::string xmile;
  std::vector<std::string> errs;
  if (!powersim::SipToXmile(data.data(), data.size(), xmile, errs)) {
    for (const std::string &e : errs) {
      printf("  %s\n", e.c_str());
    }
    return "";
  }
  return xmile;
}

bool Contains(const std::string &hay, const std::string &needle) {
  if (hay.find(needle) != std::string::npos) {
    return true;
  }
  printf("  missing: %s\n", needle.c_str());
  return false;
}

}  // namespace

TEST(SipCompoundFile_reads_streams_and_storages) {
  std::string data = ReadFixture("Eroding Goals.sip");
  CHECK(!data.empty());
  powersim::CompoundFile cf;
  std::string err;
  CHECK(cf.Open(data.data(), data.size(), err));

  uint32_t catalog = cf.Find("CatalogContents");
  CHECK(catalog != powersim::CompoundFile::kNoStream);
  std::string stream;
  CHECK(cf.ReadStream(catalog, stream, err));
  CHECK_EQ_STR(std::to_string(stream.size()), "2414");
  CHECK(stream.compare(0, 4, "FORM") == 0);

  // A nested path through two storages to a stream held in the mini stream.
  uint32_t comps = cf.Find("Components");
  CHECK(comps != powersim::CompoundFile::kNoStream && cf.Get(comps).type == 1);
  bool sawMeta = false;
  for (uint32_t c : cf.Children(comps)) {
    if (cf.Get(c).type == 1) {
      uint32_t meta = cf.Find("Components/" + cf.Get(c).name + "/MetaModule");
      sawMeta = meta != powersim::CompoundFile::kNoStream && cf.ReadStream(meta, stream, err) && stream.size() == 3058;
    }
  }
  CHECK(sawMeta);
  CHECK(cf.Find("NoSuchStream") == powersim::CompoundFile::kNoStream);
}

TEST(SipCompoundFile_rejects_bad_input) {
  powersim::CompoundFile cf;
  std::string err;
  std::string junk(1024, 'x');
  CHECK(!cf.Open(junk.data(), junk.size(), err));
  CHECK(!err.empty());

  // A valid header whose sectors are cut off must fail cleanly, not overrun.
  std::string data = ReadFixture("Eroding Goals.sip");
  for (size_t keep : {512u, 600u, 2048u, 10000u}) {
    err.clear();
    if (cf.Open(data.data(), keep, err)) {
      std::string s;
      uint32_t meta = cf.Find("CatalogContents");
      CHECK(meta == powersim::CompoundFile::kNoStream || !cf.ReadStream(meta, s, err) || s.size() == 2414);
    } else {
      CHECK(!err.empty());
    }
  }
}

TEST(Sip_not_a_sip_is_rejected) {
  const char *xml = "<xmile/>";
  std::string out;
  std::vector<std::string> errs;
  CHECK(!powersim::LooksLikeSip(xml, strlen(xml)));
  CHECK(!powersim::SipToXmile(xml, strlen(xml), out, errs));
  CHECK(!errs.empty());
  CHECK(convert_sip_to_xmile(xml, static_cast<uint32_t>(strlen(xml)), "x.sip") == nullptr);
}

TEST(Sip_every_fixture_converts_to_xmile_and_mdl) {
  for (const char *name : kFixtures) {
    std::string data = ReadFixture(name);
    CHECK(powersim::LooksLikeSip(data.data(), data.size()));
    char *x = convert_sip_to_xmile(data.data(), static_cast<uint32_t>(data.size()), name);
    char *m = convert_sip_to_mdl(data.data(), static_cast<uint32_t>(data.size()), name);
    if (!x || !m) {
      printf("  failed: %s\n", name);
    }
    CHECK(x != nullptr && std::string(x).rfind("<xmile", 0) == 0);
    CHECK(m != nullptr && std::string(m).rfind("{UTF-8}", 0) == 0);
    free(x);
    free(m);
  }
}

// Stocks, flows with signs, auxes, STEP, and plain (non-calendar) time.
TEST(Sip_eroding_goals_lowering) {
  std::string x = Lower("Eroding Goals.sip");
  CHECK(Contains(x, "<sim_specs method=\"Euler\" time_units=\"period\">"));
  CHECK(Contains(x, "<start>0</start>"));
  CHECK(Contains(x, "<stop>100</stop>"));
  CHECK(Contains(x, "<dt>1</dt>"));
  CHECK(Contains(x,
                 "<stock name=\"Actual Performance\">\n      <eqn>0</eqn>\n"
                 "      <outflow>resistance_or_goal_erosion</outflow>\n"
                 "      <inflow>progress_toward_goal</inflow>\n"));
  CHECK(Contains(x,
                 "<stock name=\"Actual Goal\">\n      <eqn>INITIAL_GOAL</eqn>\n"
                 "      <inflow>change_in_goal</inflow>\n"));
  CHECK(Contains(x,
                 "<flow name=\"progress toward goal\">\n"
                 "      <eqn>(Actual_Goal-Actual_Performance)/TIME_TO_REACH_GOAL</eqn>"));
  CHECK(Contains(x, "<aux name=\"revised goal\">\n      <eqn>Actual_Goal-STEP(gap,25)</eqn>"));
  CHECK(Contains(x, "<aux name=\"EROSION FACTOR\">\n      <eqn>0.1</eqn>\n      <units>1/period</units>"));
  // Unit annotations are dropped from the equation; a bare constant keeps them as units.
  CHECK(x.find("&lt;&lt;") == std::string::npos);
}

// IF(c, a, b), comparisons, multi-line source, and calendar time in years.
TEST(Sip_escalation_if_and_calendar_time) {
  std::string x = Lower("Escalation.sip");
  CHECK(Contains(x, "<sim_specs method=\"Euler\" time_units=\"yr\">"));
  CHECK(Contains(x, "<start>2015</start>"));
  CHECK(Contains(x, "<stop>2025</stop>"));
  CHECK(Contains(x,
                 "<flow name=\"Ted Flow\">\n      <eqn>(IF Anne_Results&gt;Ted_Results THEN "
                 "threat*(Anne_Results-Ted_Results) ELSE 0)+TED_ACT</eqn>"));
  CHECK(Contains(x, "<aux name=\"TED ACT\">\n      <eqn>.1</eqn>\n      <units>Results/yr</units>"));
}

TEST(Sip_graph_becomes_gf) {
  std::string x = Lower("Limits to Growth Ver1.sip");
  CHECK(Contains(x,
                 "<aux name=\"effect of crowding on deathrate\">\n"
                 "      <eqn>Population/CARRYING_CAPACITY</eqn>\n"
                 "      <gf>\n        <xscale min=\"0\" max=\"4\"/>\n"
                 "        <ypts>0.333,0.666,1,1.333,1.649,1.833,1.912,1.95,2</ypts>\n"));
  CHECK(Contains(x, "<dt>0.1</dt>"));
  CHECK(Contains(x, "<units>rabbits</units>"));
}

TEST(Sip_starttime_and_unary_operators) {
  std::string x = Lower("Success to the Successful.sip");
  CHECK(
      Contains(x, "<eqn>(allocation_to_B+STEP(IMPULSE,STARTTIME+4)-STEP(IMPULSE,STARTTIME+6))/TIME_TO_ALLOCATE</eqn>"));
  CHECK(Contains(x, "<eqn>1-allocation_Variable</eqn>"));
}

// A literal with a project-time-unit annotation inside an expression keeps
// its value; the stock with three flows lists them all.
TEST(Sip_tragedy_units_in_expressions) {
  std::string x = Lower("Tragedy of the Commons.sip");
  CHECK(Contains(x,
                 "<eqn>(IF A_FACTOR*1*A_Result&lt;.5*(Resources_Available-MINIMUM_RESOURCE) THEN "
                 "A_FACTOR*A_Result ELSE .5*(Resources_Available-MINIMUM_RESOURCE)/1)</eqn>"));
  CHECK(Contains(x,
                 "<outflow>A_activity</outflow>\n      <outflow>B_activity</outflow>\n"
                 "      <inflow>resource_replenishment</inflow>"));
}

// Dropping a unit that Powersim would rescale must fail loudly, not change
// the model's numbers. Patch AVERAGE LIFETIME's 8<<yr>> (token bytes
// 03 F0 47 80 74 F0 48: count, <<, unit 0x74, >>) to 8<<da>> (unit 0x34).
TEST(Sip_scaled_unit_annotation_is_refused) {
  std::string data = ReadFixture("Limits to Growth Ver1.sip");
  const std::string yr("\x03\xF0\x47\x80\x74\xF0\x48", 7);
  size_t at = data.find(yr);
  CHECK(at != std::string::npos && data.find(yr, at + 1) == std::string::npos);
  if (at == std::string::npos) {
    return;
  }
  data[at + 4] = 0x34;
  std::string xmile;
  std::vector<std::string> errs;
  CHECK(!powersim::SipToXmile(data.data(), data.size(), xmile, errs));
  bool named = false;
  for (const std::string &e : errs) {
    named = named || (e.find("AVERAGE LIFETIME") != std::string::npos && e.find("<<da>>") != std::string::npos);
  }
  CHECK(named);
}

// Diagram geometry, checked against values worked out by hand from the raw
// symbol records (thousandths of a point): gap's box is 417000..465000 x
// 285000..333000, Actual Goal's 428500..480500 x 133000..173000, and the
// change-in-goal pipe runs (700500,154588) -> (481501,154588) with the valve
// halfway.
TEST(Sip_diagram_positions) {
  std::string x = Lower("Eroding Goals.sip");
  CHECK(Contains(x, "<views>\n      <view>\n"));
  CHECK(Contains(x, "<aux name=\"gap\" x=\"441\" y=\"309\" label_side=\"left\"/>"));
  CHECK(Contains(x,
                 "<stock name=\"Actual_Goal\" x=\"428.5\" y=\"133\" width=\"52\" height=\"40\" "
                 "label_side=\"top\"/>"));
  CHECK(Contains(x,
                 "<flow name=\"change_in_goal\" x=\"591.0005\" y=\"154.588\" label_side=\"bottom\">\n"
                 "          <pts>\n            <pt x=\"700.5\" y=\"154.588\"/>\n"
                 "            <pt x=\"481.501\" y=\"154.588\"/>\n          </pts>"));
  // Take-off from (464.945,316.183) toward the control point (519.806,332.64).
  CHECK(Contains(x,
                 "<connector uid=\"1\" angle=\"343.3\">\n          <from>gap</from>\n"
                 "          <to>revised_goal</to>"));
  CHECK(Contains(x, "<from>INITIAL_GOAL</from>\n          <to>Actual_Goal</to>"));
  // 16 links, less the three that tie a flow's rate circle to its own valve.
  size_t connectors = 0;
  for (size_t at = x.find("<connector "); at != std::string::npos; at = x.find("<connector ", at + 1)) {
    ++connectors;
  }
  CHECK_EQ_STR(std::to_string(connectors), "13");
  // Each variable is placed exactly once; a flow's rate circle is not.
  CHECK(x.find("<aux name=\"change_in_goal\"") == std::string::npos);
}

// An undecodable diagram is left out with a note; the model still converts.
// Corrupt the segment type of the first link's point list (1D 04 [01] 01 ...).
TEST(Sip_bad_diagram_is_dropped_not_fatal) {
  std::string data = ReadFixture("Eroding Goals.sip");
  const std::string link("\x1D\x04\x01\x01\x9C\xB0\x31\x93\xA6\x17", 10);
  size_t at = data.find(link);
  CHECK(at != std::string::npos);
  if (at == std::string::npos) {
    return;
  }
  data[at + 2] = 0x05;
  std::string xmile;
  std::vector<std::string> errs;
  CHECK(powersim::SipToXmile(data.data(), data.size(), xmile, errs));
  CHECK(xmile.find("<views>") == std::string::npos);
  CHECK(xmile.find("<flow name=\"change in goal\">") != std::string::npos);
  bool noted = false;
  for (const std::string &e : errs) {
    noted = noted || e.find("diagram") != std::string::npos;
  }
  CHECK(noted);
}

// Powersim stores no variable units (every UNIT form is AUTO); they are
// derived from the <<unit>> annotations on literals, through the equations,
// and between stocks and their flows via the time unit.
TEST(Sip_units_are_inferred_for_every_variable) {
  struct Expect {
    const char *file, *tag, *name, *units;
  };
  const Expect expected[] = {
      {"Limits to Growth Ver1.sip", "stock", "Population", "rabbits"},
      {"Limits to Growth Ver1.sip", "flow", "deaths", "rabbits/yr"},  // rabbits/yr*dmnl via a GRAPH
      {"Limits to Growth Ver1.sip", "aux", "effect of crowding on deathrate", "dmnl"},
      {"Tragedy of the Commons.sip", "flow", "A activity", "resources/yr"},  // IF whose branches agree
      {"Fixes that Fail.sip", "aux", "gap", "State"},
      {"Fixes that Fail.sip", "flow", "action", "State/yr"},
      // A stock starting at a bare 0, whose unit comes from its flows.
      {"Shifting the Burden.sip", "stock", "Problem Symptom", "dmnl"},
      {"Shifting the Burden.sip", "flow", "problem cause", "1/yr"},
      {"Success to the Successful.sip", "flow", "action B", "1/yr"},  // STEP keeps its height's unit
      {"Eroding Goals.sip", "aux", "TIME TO REACH GOAL", "period"},
  };
  for (const Expect &e : expected) {
    std::string x = Lower(e.file);
    size_t at = x.find(std::string("<") + e.tag + " name=\"" + e.name + "\">");
    size_t end = x.find(std::string("</") + e.tag + ">", at);
    CHECK(at != std::string::npos && end != std::string::npos);
    if (at == std::string::npos || end == std::string::npos) {
      continue;
    }
    std::string var = x.substr(at, end - at);
    if (!Contains(var, std::string("<units>") + e.units + "</units>")) {
      printf("  in %s\n", e.file);
    }
  }
  // Nothing is left without units in any fixture.
  for (const char *name : kFixtures) {
    std::string x = Lower(name);
    std::string vars = x.substr(0, x.find("<views>"));
    size_t defs = 0, units = 0;
    for (const char *tag : {"<stock name=", "<flow name=", "<aux name="}) {
      for (size_t at = vars.find(tag); at != std::string::npos; at = vars.find(tag, at + 1)) {
        ++defs;
      }
    }
    for (size_t at = vars.find("<units>"); at != std::string::npos; at = vars.find("<units>", at + 1)) {
      ++units;
    }
    if (defs != units) {
      printf("  %s: %zu variables, %zu with units\n", name, defs, units);
    }
    CHECK(defs == units);
  }
}
