// XMILE normalization emits ONE <model> element.
//
// XMILEGenerator has two emission shapes. The module decomposition
// (generateModelAsModules / generateModelAsGroups) writes a base <model> plus
// one sibling <model> per view or per populated group, referenced by <module>
// elements; the sector form (generateModelAsSectors) writes a single <model>
// whose <views> carry the groups. Which one runs is decided by
// Model::PrintXMILE.
//
// XmileReader::ProcessFile refuses a document with sibling <model> elements --
// that shape signals <module> submodels, which are out of scope -- so the module
// decomposition produces XMILE this project cannot read back. That is a fine
// trade for Vensim and Dynamo input, which have no modules of their own and
// whose conversion is one-way anyway. It is not a fine trade for XMILE input,
// where the conversion is a NORMALIZATION and its output has to be re-importable:
// before this, `XMUtil model.xmile` on any model with two populated groups wrote
// five <model>-ish elements, and running the tool on its own output failed with
// "multiple <model> elements are not supported".
//
// So an XMILE-sourced Model takes the sector form regardless of what the caller
// asked for. These tests pin both halves of that: the XMILE side always emits
// one <model> and reaches a byte fixpoint, and the .mdl side still decomposes
// into modules exactly as before.

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/XMUtil.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

size_t CountOccurrences(const std::string &haystack, const std::string &needle) {
  if (needle.empty())
    return 0;
  size_t n = 0;
  for (size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + needle.size()))
    n++;
  return n;
}

// How many <model> elements the document holds -- the property that decides
// whether XmileReader will take it back. Counts the bare root form the sector
// path writes and the named form the module path writes; <model_units> matches
// neither, and </model> does not match "<model>" (the byte after '<' is '/').
size_t CountModelElements(const std::string &doc) {
  return CountOccurrences(doc, "<model>") + CountOccurrences(doc, "<model ");
}

std::string WrapModel(const std::string &variables, const std::string &views) {
  return "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\">\n"
         "  <header><name>normalization</name></header>\n"
         "  <sim_specs><start>0</start><stop>10</stop><dt>1</dt></sim_specs>\n"
         "  <model>\n"
         "    <variables>\n" +
         variables +
         "    </variables>\n"
         "    <views>\n" +
         views +
         "    </views>\n"
         "  </model>\n"
         "</xmile>\n";
}

const char *const kFourAuxes =
    "      <aux name=\"a\"><eqn>1</eqn></aux>\n"
    "      <aux name=\"b\"><eqn>a * 2</eqn></aux>\n"
    "      <aux name=\"c\"><eqn>b + 1</eqn></aux>\n"
    "      <aux name=\"d\"><eqn>3</eqn></aux>\n";

const char *const kSixAuxes =
    "      <aux name=\"a\"><eqn>1</eqn></aux>\n"
    "      <aux name=\"b\"><eqn>a * 2</eqn></aux>\n"
    "      <aux name=\"c\"><eqn>b + 1</eqn></aux>\n"
    "      <aux name=\"d\"><eqn>3</eqn></aux>\n"
    "      <aux name=\"e\"><eqn>d - 1</eqn></aux>\n"
    "      <aux name=\"f\"><eqn>e * c</eqn></aux>\n";

// The three properties every XMILE normalization owes its caller, checked
// together because a document that fails any one of them is not a normalization:
// it emits exactly one <model>, that emission re-parses through our own reader,
// and a second pass reproduces it byte for byte. `label` names the case in the
// failure output. Always runs at least three checks, so a case can never report
// PASS by falling through.
void CheckNormalizesToOneReadableModel(const char *label, const std::string &source) {
  std::vector<std::string> errs;
  const std::string doc = xmileroundtrip::NormalizeXMILE(source, errs);
  if (doc.empty()) {
    printf("  %s: normalization failed: %s\n", label, errs.empty() ? "(no diagnostic)" : errs.front().c_str());
  }
  CHECK(!doc.empty());

  const size_t models = CountModelElements(doc);
  if (models != 1)
    printf("  %s: emitted %zu <model> elements, expected 1\n", label, models);
  CHECK(models == 1);

  // Re-parsing is the assertion the <model> count exists to serve: a second
  // <model> makes ProcessFile refuse the document outright.
  std::vector<std::string> reparseErrs;
  Model *reparsed = doc.empty() ? nullptr : xmileroundtrip::ParseXMILE(doc, reparseErrs);
  if (!reparsed) {
    printf("  %s: emitted document does not re-parse: %s\n", label,
           reparseErrs.empty() ? "(no diagnostic)" : reparseErrs.front().c_str());
  }
  CHECK(reparsed != nullptr);
  delete reparsed;

  const std::string fixpoint = xmileroundtrip::FixpointFailure(source);
  if (!fixpoint.empty())
    printf("  %s: %s\n", label, fixpoint.c_str());
  CHECK(fixpoint.empty());
}

}  // namespace

// The reported repro: two populated groups. The module path emitted a base
// <model> plus one per group; the second conversion pass then failed outright.
TEST(Normalization_two_group_model_emits_one_readable_model) {
  const std::string model = WrapModel(kFourAuxes,
                                      "      <group name=\"G1\"><var>a</var><var>b</var></group>\n"
                                      "      <group name=\"G2\"><var>c</var><var>d</var></group>\n");
  CheckNormalizesToOneReadableModel("two groups", model);

  std::vector<std::string> errs;
  const std::string doc = xmileroundtrip::NormalizeXMILE(model, errs);
  CHECK(!doc.empty());
  // The grouping is not lost in the process -- it moves from <module>/<model>
  // nesting to the <views><group> shape, which is what the reader takes back.
  CHECK(doc.find("<group name=\"G1\">") != std::string::npos);
  CHECK(doc.find("<group name=\"G2\">") != std::string::npos);
  CHECK(CountOccurrences(doc, "<module") == 0);
  // Each variable keeps exactly one definition.
  CHECK(CountOccurrences(doc, "<aux name=\"a\">") == 1);
  CHECK(CountOccurrences(doc, "<aux name=\"d\">") == 1);
}

// Three groups, to show the emission does not merely happen to collapse at two.
TEST(Normalization_three_group_model_emits_one_readable_model) {
  const std::string model = WrapModel(kSixAuxes,
                                      "      <group name=\"G1\"><var>a</var><var>b</var></group>\n"
                                      "      <group name=\"G2\"><var>c</var><var>d</var></group>\n"
                                      "      <group name=\"G3\"><var>e</var><var>f</var></group>\n");
  CheckNormalizesToOneReadableModel("three groups", model);
}

// Nested ownership is the fact the module path expressed structurally, by
// putting one <module> inside another's <variables>. The sector form states it
// as owner="...", and that attribute has to survive the conversion -- the
// nesting is silently gone otherwise, and nothing about the document looks wrong.
TEST(Normalization_nested_group_owners_survive) {
  const std::string model = WrapModel(kSixAuxes,
                                      "      <group name=\"Top\"><var>a</var><var>b</var></group>\n"
                                      "      <group name=\"Middle\" owner=\"Top\"><var>c</var><var>d</var></group>\n"
                                      "      <group name=\"Leaf\" owner=\"Middle\"><var>e</var><var>f</var></group>\n");
  CheckNormalizesToOneReadableModel("nested owners", model);

  std::vector<std::string> errs;
  const std::string doc = xmileroundtrip::NormalizeXMILE(model, errs);
  CHECK(!doc.empty());
  CHECK(doc.find("<group name=\"Middle\" owner=\"Top\">") != std::string::npos);
  CHECK(doc.find("<group name=\"Leaf\" owner=\"Middle\">") != std::string::npos);
  CHECK(doc.find("<group name=\"Top\">") != std::string::npos);

  // ...and the re-import rebuilds the chain rather than merely accepting the
  // attributes: Leaf -> Middle -> Top, two hops.
  std::vector<std::string> reparseErrs;
  Model *m = xmileroundtrip::ParseXMILE(doc, reparseErrs);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *leaf = nullptr;
  for (ModelGroup *g : m->Groups()) {
    if (g->sName == "Leaf")
      leaf = g;
  }
  CHECK(leaf != nullptr);
  int depth = 0;
  for (ModelGroup *up = leaf ? leaf->pOwner : nullptr; up && depth < 16; up = up->pOwner)
    depth++;
  CHECK(depth == 2);
  delete m;
}

// A group holding no variables was the crash case on the module path (no
// <model>, so no <variables> for a child's <module> to sit in). The sector form
// emits it like any other group, so the nesting stays whole -- but it has to be
// emitted, or the child's owner= names something that is not there.
TEST(Normalization_group_owner_holding_no_variables_is_still_emitted) {
  const std::string model = WrapModel(kFourAuxes,
                                      "      <group name=\"Hollow\"></group>\n"
                                      "      <group name=\"Kid\" owner=\"Hollow\"><var>a</var><var>b</var></group>\n"
                                      "      <group name=\"G2\"><var>c</var><var>d</var></group>\n");
  CheckNormalizesToOneReadableModel("empty owner", model);

  std::vector<std::string> errs;
  const std::string doc = xmileroundtrip::NormalizeXMILE(model, errs);
  CHECK(!doc.empty());
  CHECK(doc.find("<group name=\"Hollow\"/>") != std::string::npos);
  CHECK(doc.find("<group name=\"Kid\" owner=\"Hollow\">") != std::string::npos);
}

// The multi-VIEW half of the same writer asymmetry: generateModelAsModules emits
// one <model> per view once a Model holds two or more. An XMILE-sourced Model
// never does -- XmileReader::ProcessViews takes the first view with geometry and
// warns about the rest -- so the branch is unreachable from this input, and the
// emission is a single <model> for that reason as well as for the group one.
// The test states both facts, since the reader's behavior is what makes the
// writer's unreachable and a change to either would go unnoticed otherwise.
//
// Deliberately NOT a red-before-green case: with no groups in play, this
// document already took the sector path. Its job is to pin the reader behavior
// that keeps the multi-view writer branch out of reach; the case that really did
// emit several <model> elements from a multi-view document is the next test.
TEST(Normalization_multi_view_model_emits_one_readable_model) {
  const std::string model = WrapModel(
      "      <aux name=\"a\"><eqn>1</eqn></aux>\n"
      "      <aux name=\"b\"><eqn>a * 2</eqn></aux>\n",
      "      <view view_type=\"stock_flow\"><aux name=\"a\" x=\"100\" y=\"100\"/></view>\n"
      "      <view view_type=\"stock_flow\"><aux name=\"b\" x=\"200\" y=\"200\"/></view>\n");

  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(m->Views().size() == 1);
  bool warned = false;
  for (const std::string &e : errs) {
    if (e.find("multi-view") != std::string::npos)
      warned = true;
  }
  CHECK(warned);
  delete m;

  CheckNormalizesToOneReadableModel("multi-view", model);
}

// A multi-view document that also groups its variables -- the shape
// simlin-test/land_model/land_model.stmx has, and the one that really did emit
// several <model> elements. The reader keeps the first view and drops the rest,
// so what reached the writer was one view and several populated groups, which is
// the group branch of the decomposition rather than the view branch. Either way
// the document was not re-importable, and now is.
TEST(Normalization_multi_view_with_groups_emits_one_readable_model) {
  const std::string model = WrapModel(
      "      <aux name=\"a\"><eqn>1</eqn></aux>\n"
      "      <aux name=\"b\"><eqn>a * 2</eqn></aux>\n"
      "      <aux name=\"c\"><eqn>3</eqn></aux>\n"
      "      <aux name=\"d\"><eqn>c + 1</eqn></aux>\n",
      "      <view view_type=\"stock_flow\">\n"
      "        <aux name=\"a\" uid=\"1\" x=\"100\" y=\"100\"/>\n"
      "        <aux name=\"b\" uid=\"2\" x=\"200\" y=\"100\"/>\n"
      "        <group name=\"Left\" x=\"90\" y=\"90\"><item uid=\"1\"/></group>\n"
      "        <group name=\"Right\" x=\"190\" y=\"90\"><item uid=\"2\"/></group>\n"
      "      </view>\n"
      "      <view view_type=\"stock_flow\">\n"
      "        <aux name=\"c\" uid=\"1\" x=\"100\" y=\"300\"/>\n"
      "        <aux name=\"d\" uid=\"2\" x=\"200\" y=\"300\"/>\n"
      "      </view>\n");

  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(m->Views().size() == 1);
  // Two populated groups is what sent this down the module path.
  int populated = 0;
  for (ModelGroup *g : m->Groups()) {
    if (!g->vVariables.empty())
      populated++;
  }
  CHECK(populated == 2);
  delete m;

  CheckNormalizesToOneReadableModel("multi-view with groups", model);
}

// Groups and a sketch view together. This is the shape the corpus model
// simlin-test/land_model/land_model.stmx has, and the one that shows the cost of
// the sector form: generateSectorViews writes the <group> membership list only
// when the model has NO view, so a model with a real sketch loses its grouping
// on the way out. The membership is gone from the emission and the round trip
// reports it -- which is why land_model stays in the corpus deferred table. This
// test records the behavior as it stands so a future fix flips a documented
// expectation rather than a silent one; the document is still readable and still
// a fixpoint, which is what this change was for.
TEST(Normalization_groups_are_dropped_when_the_model_has_a_sketch_view) {
  const std::string model = WrapModel(
      "      <aux name=\"a\"><eqn>1</eqn></aux>\n"
      "      <aux name=\"b\"><eqn>a * 2</eqn></aux>\n"
      "      <aux name=\"c\"><eqn>3</eqn></aux>\n",
      "      <view view_type=\"stock_flow\">\n"
      "        <aux name=\"a\" uid=\"1\" x=\"100\" y=\"100\"/>\n"
      "        <aux name=\"b\" uid=\"2\" x=\"200\" y=\"100\"/>\n"
      "        <aux name=\"c\" uid=\"3\" x=\"300\" y=\"100\"/>\n"
      "        <group name=\"Left\" x=\"90\" y=\"90\"><item uid=\"1\"/><item uid=\"2\"/></group>\n"
      "        <group name=\"Right\" x=\"290\" y=\"90\"><item uid=\"3\"/></group>\n"
      "      </view>\n");

  // The reader does register both groups, so the loss is the writer's.
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK(m->Groups().size() == 2);
  delete m;

  CheckNormalizesToOneReadableModel("groups plus sketch", model);

  std::vector<std::string> normErrs;
  const std::string doc = xmileroundtrip::NormalizeXMILE(model, normErrs);
  CHECK(!doc.empty());
  // Documented gap, not an endorsement: generateSectorViews' group emission is
  // behind a views.empty() guard. Hoisting it out is not free -- it changes the
  // .mdl -> XMILE default output for most of the corpus -- so it is its own
  // piece of work.
  CHECK(doc.find("<group name=\"Left\"") == std::string::npos);
  CHECK(doc.find("<group name=\"Right\"") == std::string::npos);
}

// The caller's sectors preference cannot select the module form for XMILE input,
// in either direction and whatever order it is stated in. Model::SetAsSectors
// stays the caller's to set -- the provenance flag is separate and is OR-ed in
// -- so setting it false AFTER the parse must not undo anything.
TEST(Normalization_as_sectors_preference_does_not_change_xmile_output) {
  const std::string model = WrapModel(kFourAuxes,
                                      "      <group name=\"G1\"><var>a</var><var>b</var></group>\n"
                                      "      <group name=\"G2\"><var>c</var><var>d</var></group>\n");

  std::string emitted[3];
  const bool prefs[3] = {false, true, false};
  const bool setAfterParse[3] = {false, true, true};
  for (int i = 0; i < 3; i++) {
    std::vector<std::string> errs;
    Model *m = xmileroundtrip::ParseXMILE(model, errs);
    CHECK(m != nullptr);
    if (!m)
      return;
    if (setAfterParse[i])
      m->SetAsSectors(prefs[i]);
    std::vector<std::string> printErrs;
    emitted[i] = m->PrintXMILE(/*isCompact=*/false, printErrs, 1.0, 1.0);
    CHECK(printErrs.empty());
    delete m;
  }
  CHECK(CountModelElements(emitted[0]) == 1);
  CHECK_EQ_STR(emitted[1], emitted[0]);
  CHECK_EQ_STR(emitted[2], emitted[0]);
}

// The same guarantee at the extern-C boundary, where isAsSectors is a documented
// accepted-but-unused parameter (src/XMUtil.h). Both values must give the same
// document, and it must be one convert_xmile_to_xmile can be run on again --
// which is exactly what `XMUtil model.xmile` twice does.
TEST(Normalization_c_entry_ignores_is_as_sectors) {
  const std::string model = WrapModel(kFourAuxes,
                                      "      <group name=\"G1\"><var>a</var><var>b</var></group>\n"
                                      "      <group name=\"G2\"><var>c</var><var>d</var></group>\n");
  const uint32_t len = static_cast<uint32_t>(model.size());

  char *withFalse = convert_xmile_to_xmile(model.c_str(), len, "test.xmile", /*isLongName=*/-1,
                                           /*isAsSectors=*/false);
  CHECK(withFalse != nullptr);
  char *withTrue = convert_xmile_to_xmile(model.c_str(), len, "test.xmile", /*isLongName=*/-1,
                                          /*isAsSectors=*/true);
  CHECK(withTrue != nullptr);
  if (!withFalse || !withTrue) {
    free(withFalse);
    free(withTrue);
    return;
  }
  const std::string pass1(withFalse);
  CHECK_EQ_STR(std::string(withTrue), pass1);
  CHECK(CountModelElements(pass1) == 1);
  free(withFalse);
  free(withTrue);

  // Second pass over the first pass's output: this returned NULL before, with
  // "multiple <model> elements are not supported" on the log.
  char *pass2Raw = convert_xmile_to_xmile(pass1.c_str(), static_cast<uint32_t>(pass1.size()), "test.regen.xmile",
                                          /*isLongName=*/-1, /*isAsSectors=*/false);
  CHECK(pass2Raw != nullptr);
  if (!pass2Raw)
    return;
  CHECK_EQ_STR(std::string(pass2Raw), pass1);
  free(pass2Raw);
}

// The other side of the contract: .mdl input is UNCHANGED. Vensim has no
// modules, so rolling its group banners up into Stella modules adds structure
// the source could not express -- a nicety worth keeping, and the reason the
// decomposition exists at all. A multi-banner .mdl must still produce it.
TEST(MdlModules_multi_group_mdl_still_decomposes_into_modules) {
  const std::string banner1 =
      "\r\n********************************************************\r\n\tGrpOne"
      "\r\n********************************************************~\r\n\t\t\r\n\t|\r\n\r\n"
      "a=1\r\n\t~\t\r\n\t~\t\r\n\t|\r\n\r\n";
  const std::string banner2 =
      "\r\n********************************************************\r\n\tGrpTwo"
      "\r\n********************************************************~\r\n\t\t\r\n\t|\r\n\r\n"
      "b=a\r\n\t~\t\r\n\t~\t\r\n\t|\r\n\r\n"
      "INITIAL TIME=0\r\n\t~\t\r\n\t~\t\r\n\t|\r\n\r\n"
      "FINAL TIME=10\r\n\t~\t\r\n\t~\t\r\n\t|\r\n\r\n"
      "TIME STEP=1\r\n\t~\t\r\n\t~\t\r\n\t|\r\n\r\n"
      "SAVEPER=TIME STEP\r\n\t~\t\r\n\t~\t\r\n\t|\r\n\r\n";
  const std::string mdl = "{UTF-8}\r\n" + banner1 + banner2;

  char *out = convert_mdl_to_xmile(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "test.mdl", /*isCompact=*/false,
                                   /*isLongName=*/0, /*isAsSectors=*/false);
  CHECK(out != nullptr);
  if (!out)
    return;
  const std::string doc(out);
  free(out);

  // The module form: a base <model> plus one <model name="..."> per group, and
  // a <module> reference for each. More than one <model> element is the whole
  // point here -- it is what the XMILE side had to stop doing.
  CHECK(CountModelElements(doc) > 1);
  CHECK(doc.find("<model name=\"GrpOne\">") != std::string::npos);
  CHECK(doc.find("<model name=\"GrpTwo\">") != std::string::npos);
  CHECK(doc.find("<module name=\"GrpOne\"") != std::string::npos);
  CHECK(doc.find("<module name=\"GrpTwo\"") != std::string::npos);
}
