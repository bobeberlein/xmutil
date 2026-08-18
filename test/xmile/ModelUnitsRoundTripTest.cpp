// Regression tests for model-level unit declarations (<model_units> / "22:").
//
// XMILE's <unit> carries three distinguishable things: a canonical name, an
// optional derived-unit EQUATION (a formula: "kg*m/s^2" for a Newton, "1" for a
// dimensionless quantity), and any number of aliases. Vensim's settings-section
// "22:" line carries only a flat comma-separated list of interchangeable names.
//
// The declaration used to be stored as one comma-joined string, and the XMILE
// writer reconstructed the <unit> by guessing which field had been the equation
// with a rule that recognized only a literal "$". Every other equation came back
// out as a fabricated <alias>, and a Stella <unit name="$"> lost its name to the
// same rule. These tests pin the whole matrix: an equation stays an equation
// through XMILE, the "$" spelling survives in BOTH directions, and the ".mdl"
// side still gets one flat, sanitized, byte-stable 22: line.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Vensim/VensimParse.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

// Wrap a <model_units> body in the smallest document the reader accepts. The
// model needs at least one variable: an empty <variables> list is not what a
// real document looks like, and a unit declaration is only interesting on a
// model that could carry units.
std::string XmileWithUnits(const std::string &unitsBody) {
  return R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <header><name>units</name></header>
  <sim_specs method="euler"><start>0</start><stop>1</stop><dt>1</dt></sim_specs>
  <model_units>
)" + unitsBody +
         R"(
  </model_units>
  <model>
    <variables>
      <aux name="a"><eqn>1</eqn></aux>
    </variables>
  </model>
</xmile>
)";
}

// The minimal sketch frame plus :L marker a .mdl needs before the Vensim parser
// will read its settings section at all (VensimParse only scans for the :L block
// after the \\\---/// ... ///---\\\ sketch frame). Mirrors the constants in
// test/mdl/ControlRoundTripTest.cpp; duplicated rather than shared so this file
// stays readable on its own.
const char *kSketchAndSettingsMarker =
    "\\\\\\---///\r\n"
    "V300  Do not put anything below this section - it will be ignored\r\n"
    "*View 1\r\n"
    "$192-192-192,0,Helvetica|10|B|0-0-0|0-0-0|-1--1--1|-1--1--1|96,96,100,0\r\n"
    "///---\\\\\\\r\n"
    ":L\x7F<%^E!@\r\n";

// A complete .mdl carrying the given settings lines (each must end in CRLF).
std::string MdlWithSettings(const std::string &settingsLines) {
  return std::string("{UTF-8}\r\n") +
         "INITIAL TIME = 0\r\n\t~~|\r\n"
         "FINAL TIME = 100\r\n\t~~|\r\n"
         "TIME STEP = 1\r\n\t~~|\r\n"
         "SAVEPER = TIME STEP\r\n\t~~|\r\n" +
         kSketchAndSettingsMarker + settingsLines + "15:0,0,0,0,0,0\r\n";
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

// Every "22:" payload in .mdl text, in emission order, with the trailing CR
// stripped (the writer's final pass converts LF to CRLF).
std::vector<std::string> UnitEquivLines(const std::string &mdl) {
  std::vector<std::string> lines;
  for (size_t pos = 0; pos < mdl.size();) {
    size_t end = mdl.find('\n', pos);
    if (end == std::string::npos)
      end = mdl.size();
    std::string line = mdl.substr(pos, end - pos);
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    if (line.rfind("22:", 0) == 0)
      lines.push_back(line.substr(3));
    pos = end + 1;
  }
  return lines;
}

// XMILE text -> .mdl text. Returns "" on any failure.
std::string XmileToMdl(const std::string &xmile) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(xmile, errs);
  if (!m)
    return std::string();
  errs.clear();
  std::string mdl = m->PrintMDL(errs);
  delete m;
  return errs.empty() ? mdl : std::string();
}

// .mdl text -> XMILE text. Returns "" on any failure.
std::string MdlToXmile(const std::string &mdl) {
  Model *m = ParseMdl(mdl);
  if (!m)
    return std::string();
  std::vector<std::string> errs;
  std::string xmile = m->PrintXMILE(/*isCompact=*/false, errs, /*xscale=*/1.0, /*yscale=*/1.0);
  delete m;
  return errs.empty() ? xmile : std::string();
}

bool Contains(const std::string &haystack, const std::string &needle) {
  return haystack.find(needle) != std::string::npos;
}

// The <model_units> element of an XMILE document, or "" if it has none. The
// assertions below have to be scoped to it: <eqn> is also every variable's
// equation tag, so an unscoped search would answer questions about the model
// body instead of about the unit declarations.
std::string ModelUnitsBlock(const std::string &xmile) {
  const size_t open = xmile.find("<model_units>");
  if (open == std::string::npos)
    return std::string();
  const size_t close = xmile.find("</model_units>", open);
  if (close == std::string::npos)
    return std::string();
  return xmile.substr(open, close - open);
}

// Number of non-overlapping occurrences of needle in haystack.
int CountOf(const std::string &haystack, const std::string &needle) {
  int n = 0;
  for (size_t pos = haystack.find(needle); pos != std::string::npos; pos = haystack.find(needle, pos + needle.size()))
    n++;
  return n;
}

}  // namespace

// A general (non-"$") equation must come back out as an <eqn>, not as an
// invented alias, and the alias list must not grow.
TEST(ModelUnits_general_eqn_survives_xmile_round_trip) {
  const std::string xmile = XmileWithUnits(R"(    <unit name="Widget">
      <eqn>w</eqn>
      <alias>Widgets</alias>
    </unit>)");

  std::vector<std::string> errs;
  const std::string out = xmileroundtrip::NormalizeXMILE(xmile, errs);
  CHECK(!out.empty());
  if (out.empty()) {
    printf("  normalization failed: %s\n", errs.empty() ? "(no diagnostic)" : errs.front().c_str());
    return;
  }

  const std::string units = ModelUnitsBlock(out);
  CHECK(!units.empty());
  CHECK(Contains(units, "<unit name=\"Widget\">"));
  CHECK(Contains(units, "<eqn>w</eqn>"));
  CHECK(Contains(units, "<alias>Widgets</alias>"));
  // The defect: the equation body was re-emitted as a second alias.
  CHECK(!Contains(units, "<alias>w</alias>"));
  CHECK(CountOf(units, "<alias>") == 1);
  if (!Contains(units, "<eqn>w</eqn>") || Contains(units, "<alias>w</alias>"))
    printf("  emitted units: %s\n", units.c_str());

  // ...and normalizing the emission again must reproduce it byte for byte, so
  // the equation cannot decay into an alias one pass later.
  const std::string failure = xmileroundtrip::FixpointFailure(xmile);
  CHECK(failure.empty());
  if (!failure.empty())
    printf("  not a fixpoint: %s\n", failure.c_str());
}

// A real derived-unit formula (the case <eqn> exists for) survives XMILE, and
// the .mdl flattening of it is stable across a re-import.
TEST(ModelUnits_derived_unit_formula_survives) {
  const std::string xmile = XmileWithUnits(R"(    <unit name="Newton">
      <eqn>kg*m/s^2</eqn>
    </unit>)");

  std::vector<std::string> errs;
  const std::string out = xmileroundtrip::NormalizeXMILE(xmile, errs);
  CHECK(!out.empty());
  if (out.empty()) {
    printf("  normalization failed: %s\n", errs.empty() ? "(no diagnostic)" : errs.front().c_str());
    return;
  }
  const std::string units = ModelUnitsBlock(out);
  CHECK(!units.empty());
  CHECK(Contains(units, "<unit name=\"Newton\">"));
  CHECK(Contains(units, "<eqn>kg*m/s^2</eqn>"));
  CHECK(!Contains(units, "<alias>"));
  if (!Contains(units, "<eqn>kg*m/s^2</eqn>"))
    printf("  emitted units: %s\n", units.c_str());

  // Vensim has no derived-unit concept, so the formula flattens into the flat
  // name list. That is lossy by construction, but it must at least be STABLE:
  // re-importing the .mdl and writing it again must not move the field around.
  const std::string mdl = XmileToMdl(xmile);
  CHECK(!mdl.empty());
  if (mdl.empty())
    return;
  const std::vector<std::string> first = UnitEquivLines(mdl);
  CHECK(first.size() == 1);
  if (first.size() != 1)
    return;
  CHECK_EQ_STR(first[0].c_str(), "Newton,kg*m/s^2");

  Model *m = ParseMdl(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  std::vector<std::string> reErrs;
  const std::string mdl2 = m->PrintMDL(reErrs);
  delete m;
  CHECK(reErrs.empty());
  const std::vector<std::string> second = UnitEquivLines(mdl2);
  CHECK(second.size() == 1);
  if (second.size() != 1)
    return;
  CHECK_EQ_STR(second[0].c_str(), first[0].c_str());
}

// The "$" currency spelling, coming FROM XMILE: <unit name="Dollar"><eqn>$</eqn>
// keeps its shape through XMILE, and flattens to Vensim's own spelling, which
// puts the "$" first ("22:$,Dollar,Dollars,$s" -- the literal byte sequence real
// Vensim files in test/fixtures use).
TEST(ModelUnits_dollar_eqn_from_xmile) {
  const std::string xmile = XmileWithUnits(R"(    <unit name="Dollar">
      <eqn>$</eqn>
      <alias>Dollars</alias>
      <alias>$s</alias>
    </unit>)");

  std::vector<std::string> errs;
  const std::string out = xmileroundtrip::NormalizeXMILE(xmile, errs);
  CHECK(!out.empty());
  if (out.empty()) {
    printf("  normalization failed: %s\n", errs.empty() ? "(no diagnostic)" : errs.front().c_str());
    return;
  }
  const std::string units = ModelUnitsBlock(out);
  CHECK(!units.empty());
  CHECK(Contains(units, "<unit name=\"Dollar\">"));
  CHECK(Contains(units, "<eqn>$</eqn>"));
  CHECK(Contains(units, "<alias>Dollars</alias>"));
  CHECK(Contains(units, "<alias>$s</alias>"));
  CHECK(CountOf(units, "<alias>") == 2);

  const std::string mdl = XmileToMdl(xmile);
  CHECK(!mdl.empty());
  if (mdl.empty())
    return;
  const std::vector<std::string> lines = UnitEquivLines(mdl);
  CHECK(lines.size() == 1);
  if (lines.size() != 1)
    return;
  CHECK_EQ_STR(lines[0].c_str(), "$,Dollar,Dollars,$s");
}

// The same spelling coming FROM VENSIM: a "22:$,..." line still yields
// <eqn>$</eqn> on the readable name, and the full .mdl -> XMILE -> .mdl loop
// returns the original line unchanged.
TEST(ModelUnits_dollar_line_from_vensim) {
  const std::string mdl = MdlWithSettings("22:$,Dollar,Dollars,$s\r\n");

  const std::string xmile = MdlToXmile(mdl);
  CHECK(!xmile.empty());
  if (xmile.empty())
    return;
  const std::string units = ModelUnitsBlock(xmile);
  CHECK(!units.empty());
  CHECK(Contains(units, "<unit name=\"Dollar\">"));
  CHECK(Contains(units, "<eqn>$</eqn>"));
  CHECK(Contains(units, "<alias>Dollars</alias>"));
  CHECK(Contains(units, "<alias>$s</alias>"));

  // Closing the loop is what makes the "$" mapping a spelling convention rather
  // than a one-way rewrite: the field order must land back where Vensim had it.
  const std::string mdl2 = XmileToMdl(xmile);
  CHECK(!mdl2.empty());
  if (mdl2.empty())
    return;
  const std::vector<std::string> lines = UnitEquivLines(mdl2);
  CHECK(lines.size() == 1);
  if (lines.size() != 1)
    return;
  CHECK_EQ_STR(lines[0].c_str(), "$,Dollar,Dollars,$s");
}

// A present-but-empty <eqn/> -- what real Stella exports carry -- states no
// formula. It must not become a field, and in particular must not let the "$"
// rule steal the unit's name (the PureAIModel.stmx / PureHumanModel.stmx shape).
TEST(ModelUnits_empty_eqn_is_not_a_field) {
  const std::string xmile = XmileWithUnits(R"(    <unit name="$">
      <eqn/>
      <alias>dollar</alias>
      <alias>dollars</alias>
    </unit>)");

  std::vector<std::string> errs;
  const std::string out = xmileroundtrip::NormalizeXMILE(xmile, errs);
  CHECK(!out.empty());
  if (out.empty()) {
    printf("  normalization failed: %s\n", errs.empty() ? "(no diagnostic)" : errs.front().c_str());
    return;
  }
  const std::string units = ModelUnitsBlock(out);
  CHECK(!units.empty());
  CHECK(Contains(units, "<unit name=\"$\">"));
  CHECK(!Contains(units, "<eqn"));
  CHECK(Contains(units, "<alias>dollar</alias>"));
  CHECK(Contains(units, "<alias>dollars</alias>"));
  CHECK(CountOf(units, "<alias>") == 2);
  // The defect: the first alias was promoted to the name and the "$" name
  // demoted to an equation.
  CHECK(!Contains(units, "<unit name=\"dollar\">"));
  if (Contains(units, "<unit name=\"dollar\">") || Contains(units, "<eqn"))
    printf("  emitted units: %s\n", units.c_str());

  const std::string failure = xmileroundtrip::FixpointFailure(xmile);
  CHECK(failure.empty());
  if (!failure.empty())
    printf("  not a fixpoint: %s\n", failure.c_str());
}

// The two degenerate shapes: aliases with no equation (the common Vensim case)
// and an equation with no aliases. Neither may acquire the other's fields.
TEST(ModelUnits_eqn_only_and_aliases_only) {
  const std::string xmile = XmileWithUnits(R"(    <unit name="Hour">
      <alias>Hours</alias>
    </unit>
    <unit name="Dimensionless">
      <eqn>1</eqn>
    </unit>)");

  std::vector<std::string> errs;
  const std::string out = xmileroundtrip::NormalizeXMILE(xmile, errs);
  CHECK(!out.empty());
  if (out.empty()) {
    printf("  normalization failed: %s\n", errs.empty() ? "(no diagnostic)" : errs.front().c_str());
    return;
  }
  const std::string units = ModelUnitsBlock(out);
  CHECK(!units.empty());
  CHECK(Contains(units, "<unit name=\"Hour\">"));
  CHECK(Contains(units, "<alias>Hours</alias>"));
  CHECK(Contains(units, "<unit name=\"Dimensionless\">"));
  CHECK(Contains(units, "<eqn>1</eqn>"));
  // Exactly one of each: the alias-only unit gained no <eqn> and the eqn-only
  // unit gained no <alias>.
  CHECK(CountOf(units, "<eqn>") == 1);
  CHECK(CountOf(units, "<alias>") == 1);
  if (CountOf(units, "<eqn>") != 1 || CountOf(units, "<alias>") != 1)
    printf("  emitted units: %s\n", units.c_str());

  const std::string mdl = XmileToMdl(xmile);
  CHECK(!mdl.empty());
  if (mdl.empty())
    return;
  const std::vector<std::string> lines = UnitEquivLines(mdl);
  CHECK(lines.size() == 2);
  if (lines.size() != 2)
    return;
  CHECK_EQ_STR(lines[0].c_str(), "Hour,Hours");
  CHECK_EQ_STR(lines[1].c_str(), "Dimensionless,1");
}

// Every 22: payload shape the vendored Vensim corpus contains must survive a
// .mdl -> Model -> .mdl trip byte for byte, in order. The writer flattens from a
// structured declaration now, so this pins that the flattening is an exact
// inverse of the 22: parse rather than merely a plausible one.
TEST(ModelUnits_vensim_settings_lines_round_trip_byte_identically) {
  // The first is the "$" shape; the rest are ordinary flat lists, including one
  // with embedded spaces and one with five fields.
  const char *const kPayloads[] = {
      "$,Dollar,Dollars,$s",
      "Day,Days",
      "Person,People,Persons",
      "degreesC,DegreesC,degrees C,Degrees C,DegreeC",
      "tons CO2,Tons CO2,tonsCO2,TonsCO2,TonsCO2eq,Tons CO2eq,tonsCO2eq,tons CO2eq",
      // Degenerate but legal: a single-field line declares no equivalence, and a
      // lone "$" has no name to hand the "$" rule.
      "Widgets",
      "$",
      // A "$" that is NOT leading is just another spelling in Vensim's flat
      // list; it must stay in place rather than being hoisted to the front.
      "Dollar,$,Dollars",
  };

  std::string settings;
  for (const char *payload : kPayloads)
    settings += std::string("22:") + payload + "\r\n";

  const std::string mdl = MdlWithSettings(settings);
  Model *m = ParseMdl(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  std::vector<std::string> errs;
  const std::string regen = m->PrintMDL(errs);
  delete m;
  CHECK(errs.empty());
  CHECK(!regen.empty());
  if (regen.empty())
    return;

  const std::vector<std::string> lines = UnitEquivLines(regen);
  const size_t expected = sizeof(kPayloads) / sizeof(kPayloads[0]);
  CHECK(lines.size() == expected);
  if (lines.size() != expected) {
    for (const std::string &l : lines)
      printf("  re-emitted: 22:%s\n", l.c_str());
    return;
  }
  for (size_t i = 0; i < expected; i++)
    CHECK_EQ_STR(lines[i].c_str(), kPayloads[i]);
}

// Characters that are structural in a .mdl settings line -- ',' (the 22: field
// separator), '|' and a line break (either would end the line early and make the
// rest re-import as garbage, #849) -- are legal inside an XMILE <unit>. They must
// therefore be neutralized where the .mdl line is built, and only there: the
// XMILE -> XMILE path has no such hazard and must keep the text verbatim.
TEST(ModelUnits_mdl_fields_are_sanitized_but_xmile_is_not) {
  const std::string xmile = XmileWithUnits(R"(    <unit name="a,b">
      <alias>c|d</alias>
      <alias>e
f</alias>
    </unit>)");

  std::vector<std::string> errs;
  const std::string out = xmileroundtrip::NormalizeXMILE(xmile, errs);
  CHECK(!out.empty());
  if (out.empty()) {
    printf("  normalization failed: %s\n", errs.empty() ? "(no diagnostic)" : errs.front().c_str());
    return;
  }
  const std::string units = ModelUnitsBlock(out);
  CHECK(!units.empty());
  CHECK(Contains(units, "<unit name=\"a,b\">"));
  CHECK(Contains(units, "<alias>c|d</alias>"));
  if (!Contains(units, "<unit name=\"a,b\">") || !Contains(units, "<alias>c|d</alias>"))
    printf("  emitted units: %s\n", units.c_str());

  // On the .mdl side all three collapse: ',' and the break become a space, '|'
  // becomes '/' (a non-whitespace substitute, so nothing can be trimmed away),
  // and the whole declaration stays on ONE line with exactly three fields.
  const std::string mdl = XmileToMdl(xmile);
  CHECK(!mdl.empty());
  if (mdl.empty())
    return;
  const std::vector<std::string> lines = UnitEquivLines(mdl);
  CHECK(lines.size() == 1);
  if (lines.size() != 1) {
    for (const std::string &l : lines)
      printf("  emitted: 22:%s\n", l.c_str());
    return;
  }
  CHECK_EQ_STR(lines[0].c_str(), "a b,c/d,e f");

  // The sanitized line must also be a fixpoint: re-importing it and writing it
  // again cannot introduce another substitution.
  Model *m = ParseMdl(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  std::vector<std::string> reErrs;
  const std::string mdl2 = m->PrintMDL(reErrs);
  delete m;
  CHECK(reErrs.empty());
  const std::vector<std::string> again = UnitEquivLines(mdl2);
  CHECK(again.size() == 1);
  if (again.size() != 1)
    return;
  CHECK_EQ_STR(again[0].c_str(), "a b,c/d,e f");
}

// The one documented exception to byte-identical 22: re-emission, and the reason
// it is worth having: the sanitizing is applied to every field the writer emits,
// whichever reader produced it. A '|' arriving from a .mdl is neutralized just
// like one arriving from XMILE, so the substitution rule does not depend on
// provenance and the result is a fixpoint after the first pass. No line in the
// vendored Vensim corpus carries such a character, so this costs nothing there.
TEST(ModelUnits_hazardous_char_from_vensim_is_neutralized_too) {
  const std::string mdl = MdlWithSettings("22:a|b,c\r\n");
  Model *m = ParseMdl(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  std::vector<std::string> errs;
  const std::string regen = m->PrintMDL(errs);
  delete m;
  CHECK(errs.empty());

  const std::vector<std::string> lines = UnitEquivLines(regen);
  CHECK(lines.size() == 1);
  if (lines.size() != 1)
    return;
  CHECK_EQ_STR(lines[0].c_str(), "a/b,c");

  Model *m2 = ParseMdl(regen);
  CHECK(m2 != nullptr);
  if (!m2)
    return;
  std::vector<std::string> reErrs;
  const std::string regen2 = m2->PrintMDL(reErrs);
  delete m2;
  CHECK(reErrs.empty());
  const std::vector<std::string> again = UnitEquivLines(regen2);
  CHECK(again.size() == 1);
  if (again.size() != 1)
    return;
  CHECK_EQ_STR(again[0].c_str(), "a/b,c");
}
