// XMILE <group> tests: owner resolution, membership, and the module emission
// that reads both back out.
//
// The properties pinned here are invisible to the structural round-trip tests,
// because each of them preserves a model that still parses and still compares
// equal to itself:
//
//   * owner="Parent" on a <group> declared BEFORE the group it names used to
//     resolve against the groups walked so far -- i.e. never -- so a forward
//     reference was dropped and the child came back as a top-level module.
//     XMILE fixes no order among <group> elements, so that is not an exotic
//     document shape; it is half of them.
//   * A group listed under two <view> elements is one ModelGroup, but each
//     occurrence re-appended its members. MDLGenerator::GenerateEquations emits
//     one equation per membership, so the .mdl carried the variable's entire
//     definition twice -- which Vensim rejects.
//   * An owner holding no variables is not emitted, so it had no <variables>
//     list to hold its children's <module> elements, and the cross-level
//     <connect> loop dereferenced the NULL that left behind. Those tests crash
//     the run outright before the fix rather than reporting a failed check.
//   * A whitespace-only <group name> reaches the writer as the empty string;
//     the writer used to abort on it (debug) or emit name="" (release).
//   * An <item uid="N"/> resolving to an <alias> is a ghost PLACEMENT, not a
//     membership claim, and used to outvote the group the variable is really
//     drawn in whenever it came first in the document.
//
// The owner tests assert on ModelGroup::pOwner (what both writers read) and on
// the emitted document; the membership tests assert on the emitted .mdl,
// because the duplicate is a property of the text a downstream tool has to
// swallow.

#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
#include "../../src/Symbol/Variable.h"
#include "../../src/Vensim/VensimParse.h"
#include "../TestHarness.h"
#include "../mdl/ModelComparator.h"
#include "RoundTrip.h"

namespace {

// A document whose <views> holds only <group> children -- the shape
// XMILEGenerator::generateSectorViews emits for a model with sectors and no
// sketch, and the shape that carries owner="...".
std::string WrapGroups(const std::string &variables, const std::string &groups) {
  return "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\">\n"
         "  <header><name>groups</name></header>\n"
         "  <sim_specs><start>0</start><stop>10</stop><dt>1</dt></sim_specs>\n"
         "  <model>\n"
         "    <variables>\n" +
         variables +
         "    </variables>\n"
         "    <views>\n" +
         groups +
         "    </views>\n"
         "  </model>\n"
         "</xmile>\n";
}

const char *const kThreeAuxes =
    "      <aux name=\"alpha\"><eqn>1</eqn></aux>\n"
    "      <aux name=\"beta\"><eqn>2</eqn></aux>\n"
    "      <aux name=\"gamma\"><eqn>3</eqn></aux>\n";

// Whole-file read for the one test that loads a vendored fixture from disk.
// Returns the empty string when the path does not resolve.
std::string ReadFixtureFile(const std::string &path) {
  std::ifstream in(path, std::ios::in | std::ios::binary);
  if (!in.is_open())
    return std::string();
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

ModelGroup *GroupNamed(Model *m, const std::string &name) {
  for (ModelGroup *g : m->Groups()) {
    if (g->sName == name)
      return g;
  }
  return nullptr;
}

size_t CountOccurrences(const std::string &haystack, const std::string &needle) {
  if (needle.empty())
    return 0;
  size_t n = 0;
  for (size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + needle.size()))
    n++;
  return n;
}

bool AnyErrContains(const std::vector<std::string> &errs, const std::string &fragment) {
  for (const std::string &e : errs) {
    if (e.find(fragment) != std::string::npos)
      return true;
  }
  return false;
}

// Emit the model through XMILEGenerator::generateSectorViews, which is where
// <group owner="..."> and the <var> membership list are written. The
// SetAsSectors(true) is redundant for a Model that came from ParseXMILE -- those
// take the sectors path whatever the caller asks for (Model::PrintXMILE) -- and
// is kept only to say out loud which emission each assertion below is about.
// SingleModelNormalizationTest pins that redundancy, so if the two paths ever
// diverge again a test says so rather than these quietly measuring the wrong one.
std::string EmitSectorXMILE(Model *m) {
  std::vector<std::string> errs;
  m->SetAsSectors(true);
  return m->PrintXMILE(/*isCompact=*/false, errs, /*xscale=*/1.0, /*yscale=*/1.0);
}

// One XMILE -> Model -> XMILE pass through the sectors path. Returns the empty
// string on any failure.
std::string NormalizeSectorXMILE(const std::string &text) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(text, errs);
  if (!m)
    return std::string();
  const std::string out = EmitSectorXMILE(m);
  delete m;
  return out;
}

// The longest owner chain reachable from `start`, capped at `limit`. A cycle
// among the pOwner links would make XMILEGenerator::generateModelAsGroups nest
// modules inside each other without end, so the tests assert the depth stays
// finite rather than trusting the walk to terminate.
int OwnerDepth(ModelGroup *start, int limit) {
  int depth = 0;
  for (ModelGroup *up = start ? start->pOwner : nullptr; up && depth < limit; up = up->pOwner)
    depth++;
  return depth;
}

// Round-trip through the SECTORS emission and return the ModelComparator
// differences. This is now the same emission xmileroundtrip::RoundTripDiffs
// gets -- XMILE input always takes the sectors path -- and is kept separate
// because these tests are specifically about the <views><group> shape, which is
// only ever written from here.
std::vector<std::string> SectorRoundTripDiffs(const std::string &text) {
  std::vector<std::string> errs;
  Model *m0 = xmileroundtrip::ParseXMILE(text, errs);
  if (!m0)
    return {errs.empty() ? std::string("failed to parse input XMILE")
                         : ("failed to parse input XMILE: " + errs.front())};
  const std::string regen = EmitSectorXMILE(m0);
  if (regen.empty()) {
    delete m0;
    return {"sectors emission was empty"};
  }
  std::vector<std::string> reparseErrs;
  Model *m1 = xmileroundtrip::ParseXMILE(regen, reparseErrs);
  if (!m1) {
    delete m0;
    return {reparseErrs.empty() ? std::string("failed to re-parse generated XMILE")
                                : ("failed to re-parse generated XMILE: " + reparseErrs.front())};
  }
  std::vector<std::string> diffs = ModelComparator::Compare(m0, m1);
  delete m0;
  delete m1;
  return diffs;
}

// The root <model> element's own body -- the one with no name attribute, which
// carries the TOP-LEVEL <module> list on the default emission path. A group
// nested under an owner must not appear here.
std::string RootModelBody(const std::string &doc) {
  const size_t open = doc.find("<model>");
  if (open == std::string::npos)
    return std::string();
  // The root model contains no nested <model>, so the first close is its own.
  const size_t close = doc.find("</model>", open);
  if (close == std::string::npos)
    return std::string();
  return doc.substr(open, close - open);
}

// Every value of `attr` on `element` in the emitted document, e.g.
// NamedElements(doc, "module", "name") -> the names of all <module> elements.
std::set<std::string> NamedElements(const std::string &doc, const std::string &element, const std::string &attr) {
  const std::string open = "<" + element + " " + attr + "=\"";
  std::set<std::string> found;
  for (size_t at = doc.find(open); at != std::string::npos; at = doc.find(open, at + open.size())) {
    const size_t start = at + open.size();
    const size_t end = doc.find('"', start);
    if (end == std::string::npos)
      break;
    found.insert(doc.substr(start, end - start));
  }
  return found;
}

// The module-path emission is a flat list of <model name="..."> elements plus
// <module name="..."> references that nest them. A reference naming no model is
// a dangling pointer in the document: the receiving tool has a module to draw
// and nothing to put in it, and the group's variables are nowhere. Returns the
// offending names, so a failure says which.
std::vector<std::string> DanglingModuleNames(const std::string &doc) {
  const std::set<std::string> models = NamedElements(doc, "model", "name");
  std::vector<std::string> dangling;
  for (const std::string &module : NamedElements(doc, "module", "name")) {
    if (models.find(module) == models.end())
      dangling.push_back(module);
  }
  return dangling;
}

// Every value of `attr` wherever it appears, which NamedElements cannot give:
// it keys off the element name and requires the attribute to come FIRST, and
// owner="..." never does. The leading space keeps a longer attribute whose name
// ends in `attr` from matching.
std::set<std::string> AttributeValues(const std::string &doc, const std::string &attr) {
  const std::string open = " " + attr + "=\"";
  std::set<std::string> found;
  for (size_t at = doc.find(open); at != std::string::npos; at = doc.find(open, at + open.size())) {
    const size_t start = at + open.size();
    const size_t end = doc.find('"', start);
    if (end == std::string::npos)
      break;
    found.insert(doc.substr(start, end - start));
  }
  return found;
}

// The sectors emission's counterpart to DanglingModuleNames: an owner="X" that
// names no <group name="X"> in the same document is a claim the re-import
// cannot resolve, so the nesting is silently lost. Returns the offending names.
std::vector<std::string> DanglingGroupOwners(const std::string &doc) {
  const std::set<std::string> groups = NamedElements(doc, "group", "name");
  std::vector<std::string> dangling;
  for (const std::string &owner : AttributeValues(doc, "owner")) {
    if (groups.find(owner) == groups.end())
      dangling.push_back(owner);
  }
  return dangling;
}

// How many <model> elements the document holds. XmileReader::ProcessFile
// refuses more than one (sibling <model> elements signal <module> submodels),
// so this decides whether an emitted document can be read back at all. Counts
// the bare root form and the named form the module path writes; <model_units>
// matches neither.
size_t CountModelElements(const std::string &doc) {
  return CountOccurrences(doc, "<model>") + CountOccurrences(doc, "<model ");
}

// Parse Vensim .mdl text the way the CLI does and run the post-parse pipeline.
// The group-banner shapes these tests are about are a Vensim construct: a
// banner with no equations under it is an empty group, and a banner names its
// owner by carrying the owner's path ("Top.E1"), which is how a chain of empty
// levels gets written.
Model *ParseMDL(const std::string &text) {
  Model *m = new Model();
  VensimParse vp{m};
  if (!vp.ProcessFile("<test>", text.c_str(), text.size())) {
    delete m;
    return nullptr;
  }
  m->RunPostParsePipeline();
  return m;
}

// One Vensim group banner and the equations under it. An empty `equations`
// makes the empty banner these tests turn on.
std::string Banner(const std::string &name, const std::string &equations) {
  return "\r\n********************************************************\r\n\t" + name +
         "\r\n********************************************************~\r\n\t\t\r\n\t|\r\n\r\n" + equations;
}

std::string EqnBlock(const std::string &lhs, const std::string &rhs) {
  return lhs + "=" + rhs + "\r\n\t~\t\r\n\t~\t\r\n\t|\r\n\r\n";
}

const char *const kControlSection =
    "INITIAL TIME=0\r\n\t~\t\r\n\t~\t\r\n\t|\r\n\r\n"
    "FINAL TIME=10\r\n\t~\t\r\n\t~\t\r\n\t|\r\n\r\n"
    "TIME STEP=1\r\n\t~\t\r\n\t~\t\r\n\t|\r\n\r\n"
    "SAVEPER=TIME STEP\r\n\t~\t\r\n\t~\t\r\n\t|\r\n\r\n";

// A <view> carrying real sketch geometry, which is what makes the reader
// allocate a VensimView and take the <item uid="N"/> membership path.
std::string WrapGeometryView(const std::string &body) {
  return "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\">\n"
         "  <header><name>groups</name></header>\n"
         "  <sim_specs><start>0</start><stop>10</stop><dt>1</dt></sim_specs>\n"
         "  <model>\n"
         "    <variables>\n"
         "      <aux name=\"alpha\"><eqn>1</eqn></aux>\n"
         "      <aux name=\"beta\"><eqn>alpha</eqn></aux>\n"
         "    </variables>\n"
         "    <views>\n"
         "      <view view_type=\"stock_flow\">\n" +
         body +
         "      </view>\n"
         "    </views>\n"
         "  </model>\n"
         "</xmile>\n";
}

// The group a variable ended up in, by variable name, or "(none)".
std::string GroupOf(Model *m, const std::string &varName) {
  for (ModelGroup *g : m->Groups()) {
    for (Variable *v : g->vVariables) {
      if (v->GetName() == varName)
        return g->sName;
    }
  }
  return "(none)";
}

}  // namespace

// A child group declared before its parent must still be owned by it. Before
// the fix the lookup ran against the groups already walked, so this document
// produced an unowned "Child".
TEST(Group_forward_owner_reference_resolves) {
  const std::string model = WrapGroups(kThreeAuxes,
                                       "      <group name=\"Child\" owner=\"Parent\"><var>alpha</var></group>\n"
                                       "      <group name=\"Parent\"><var>beta</var></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;

  ModelGroup *child = GroupNamed(m, "Child");
  ModelGroup *parent = GroupNamed(m, "Parent");
  CHECK(child != nullptr);
  CHECK(parent != nullptr);
  if (child && parent) {
    CHECK(child->pOwner == parent);
    CHECK(parent->pOwner == nullptr);
  }
  for (const std::string &e : errs)
    printf("  err: %s\n", e.c_str());
  CHECK(!AnyErrContains(errs, "owner"));

  // The owner reaches output as an attribute on the sectors path...
  const std::string doc = EmitSectorXMILE(m);
  CHECK(doc.find("<group name=\"Child\" owner=\"Parent\">") != std::string::npos);
  delete m;

  // ...and that is the ONLY way it reaches output for XMILE input, because the
  // default path emits the same document. The module nesting this used to
  // assert on is still how a Vensim model expresses the same two facts (see
  // Group_vensim_empty_banner_owner_does_not_crash_the_module_path), but for a
  // model read from XMILE it produced sibling <model> elements our own reader
  // refuses, so it is no longer reachable from here.
  std::vector<std::string> defaultErrs;
  Model *m2 = xmileroundtrip::ParseXMILE(model, defaultErrs);
  CHECK(m2 != nullptr);
  if (!m2)
    return;
  std::vector<std::string> printErrs;
  const std::string dflt = m2->PrintXMILE(/*isCompact=*/false, printErrs, 1.0, 1.0);
  CHECK(printErrs.empty());
  CHECK_EQ_STR(dflt, doc);
  CHECK(CountModelElements(dflt) == 1);
  CHECK(CountOccurrences(dflt, "<module") == 0);
  delete m2;
}

// The order that already worked must keep working: resolution is deferred for
// everyone, so a backward reference has to survive the change too.
TEST(Group_backward_owner_reference_still_resolves) {
  const std::string model = WrapGroups(kThreeAuxes,
                                       "      <group name=\"Parent\"><var>beta</var></group>\n"
                                       "      <group name=\"Child\" owner=\"Parent\"><var>alpha</var></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *child = GroupNamed(m, "Child");
  ModelGroup *parent = GroupNamed(m, "Parent");
  CHECK(child != nullptr && parent != nullptr);
  if (child && parent)
    CHECK(child->pOwner == parent);
  CHECK(EmitSectorXMILE(m).find("<group name=\"Child\" owner=\"Parent\">") != std::string::npos);
  delete m;
}

// An owner that names nothing is the modeler's error, not the reader's: say so
// and keep the group, unowned. Silently dropping the attribute would leave a
// hand-edited typo indistinguishable from a group that never claimed an owner.
TEST(Group_owner_naming_unknown_group_is_diagnosed_and_group_survives) {
  const std::string model = WrapGroups(kThreeAuxes,
                                       "      <group name=\"Child\" owner=\"Nowhere\"><var>alpha</var></group>\n"
                                       "      <group name=\"Parent\"><var>beta</var></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *child = GroupNamed(m, "Child");
  CHECK(child != nullptr);
  if (child) {
    CHECK(child->pOwner == nullptr);
    CHECK(child->vVariables.size() == 1);
  }
  CHECK(AnyErrContains(errs, "names no group"));
  const std::string doc = EmitSectorXMILE(m);
  CHECK(doc.find("<group name=\"Child\">") != std::string::npos);
  CHECK(doc.find("owner=\"Nowhere\"") == std::string::npos);
  delete m;
}

// A group that owns itself is a one-element cycle. The Vensim-input module
// emission would try to insert its <module> into its own <variables>, so the
// link is refused at read time, for every writer.
TEST(Group_self_owner_is_diagnosed_and_refused) {
  const std::string model = WrapGroups(kThreeAuxes,
                                       "      <group name=\"Loop\" owner=\"Loop\"><var>alpha</var></group>\n"
                                       "      <group name=\"Other\"><var>beta</var></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *loop = GroupNamed(m, "Loop");
  CHECK(loop != nullptr);
  if (loop)
    CHECK(loop->pOwner == nullptr);
  CHECK(AnyErrContains(errs, "cannot own itself"));
  // The refusal is visible in the emitted document: "Loop" is written once and
  // carries no owner= attribute. Had the link survived, the group would name
  // itself as its own owner -- a claim no re-import can unwind.
  std::vector<std::string> printErrs;
  const std::string doc = m->PrintXMILE(/*isCompact=*/false, printErrs, 1.0, 1.0);
  CHECK(printErrs.empty());
  CHECK(!doc.empty());
  CHECK(CountOccurrences(doc, "<group name=\"Loop\">") == 1);
  CHECK(doc.find("owner=\"Loop\"") == std::string::npos);
  CHECK(DanglingGroupOwners(doc).empty());
  delete m;

  const std::vector<std::string> diffs = SectorRoundTripDiffs(model);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

// A 2-cycle: the second link is the one refused, so the first group keeps the
// nesting it asked for and the chain stays a forest.
TEST(Group_owner_cycle_is_broken_with_a_diagnostic) {
  const std::string model = WrapGroups(kThreeAuxes,
                                       "      <group name=\"Ay\" owner=\"Bee\"><var>alpha</var></group>\n"
                                       "      <group name=\"Bee\" owner=\"Ay\"><var>beta</var></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *ay = GroupNamed(m, "Ay");
  ModelGroup *bee = GroupNamed(m, "Bee");
  CHECK(ay != nullptr && bee != nullptr);
  if (ay && bee) {
    CHECK(ay->pOwner == bee);
    CHECK(bee->pOwner == nullptr);
    // A cycle would make this walk run to the cap instead of stopping at 1.
    CHECK(OwnerDepth(ay, 64) == 1);
    CHECK(OwnerDepth(bee, 64) == 0);
  }
  CHECK(AnyErrContains(errs, "cycle"));
  // Only the closing link is refused, so the emitted document states the one
  // nesting that survives and leaves "Bee" at the top.
  std::vector<std::string> printErrs;
  const std::string doc = m->PrintXMILE(/*isCompact=*/false, printErrs, 1.0, 1.0);
  CHECK(printErrs.empty());
  CHECK(!doc.empty());
  CHECK(CountOccurrences(doc, "<group name=\"Ay\" owner=\"Bee\">") == 1);
  CHECK(CountOccurrences(doc, "<group name=\"Bee\">") == 1);
  CHECK(DanglingGroupOwners(doc).empty());
  delete m;

  const std::vector<std::string> diffs = SectorRoundTripDiffs(model);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

// Three groups in a ring: only the link that closes it is refused, so two of
// the three stated nestings are honored.
TEST(Group_owner_three_cycle_keeps_the_acyclic_prefix) {
  const std::string model = WrapGroups(kThreeAuxes,
                                       "      <group name=\"Ay\" owner=\"Bee\"><var>alpha</var></group>\n"
                                       "      <group name=\"Bee\" owner=\"Cee\"><var>beta</var></group>\n"
                                       "      <group name=\"Cee\" owner=\"Ay\"><var>gamma</var></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *ay = GroupNamed(m, "Ay");
  ModelGroup *bee = GroupNamed(m, "Bee");
  ModelGroup *cee = GroupNamed(m, "Cee");
  CHECK(ay != nullptr && bee != nullptr && cee != nullptr);
  if (ay && bee && cee) {
    CHECK(ay->pOwner == bee);
    CHECK(bee->pOwner == cee);
    CHECK(cee->pOwner == nullptr);
    CHECK(OwnerDepth(ay, 64) == 2);
  }
  CHECK(AnyErrContains(errs, "cycle"));
  std::vector<std::string> printErrs;
  CHECK(!m->PrintXMILE(/*isCompact=*/false, printErrs, 1.0, 1.0).empty());
  delete m;

  const std::vector<std::string> diffs = SectorRoundTripDiffs(model);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

// The same group under two <view> elements is ONE group whose membership is
// the union, not the concatenation. Before the fix each occurrence re-appended
// its <var> children, so the .mdl defined every member twice.
TEST(Group_repeated_across_views_lists_each_member_once) {
  const std::string model =
      WrapGroups(kThreeAuxes,
                 "      <view name=\"v1\"><group name=\"G1\"><var>alpha</var><var>beta</var></group></view>\n"
                 "      <view name=\"v2\"><group name=\"G1\"><var>alpha</var><var>beta</var></group></view>\n"
                 "      <group name=\"G2\"><var>gamma</var></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;

  ModelGroup *g1 = GroupNamed(m, "G1");
  CHECK(g1 != nullptr);
  if (g1) {
    CHECK(g1->vVariables.size() == 2);
    // Order is the document's, and it reaches emitted output.
    if (g1->vVariables.size() == 2) {
      CHECK_EQ_STR(g1->vVariables[0]->GetName(), "alpha");
      CHECK_EQ_STR(g1->vVariables[1]->GetName(), "beta");
    }
  }

  std::vector<std::string> mdlErrs;
  const std::string mdl = m->PrintMDL(mdlErrs);
  CHECK(mdlErrs.empty());
  // The .mdl entry for a scalar is "\r\n<name> = <value>\r\n"; counting that
  // exact shape counts DEFINITIONS, which is what Vensim rejects duplicates of.
  CHECK(CountOccurrences(mdl, "\r\nalpha = 1\r\n") == 1);
  CHECK(CountOccurrences(mdl, "\r\nbeta = 2\r\n") == 1);
  CHECK(CountOccurrences(mdl, "\r\ngamma = 3\r\n") == 1);
  delete m;

  // And the emission is Vensim the Vensim reader accepts: XmileToMdlDiffs
  // re-parses it through VensimParse and compares the two models.
  const std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(model);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

// The same defect seen from the writer's side: a group emitted with its members
// listed twice would grow the document, and the <var> list is what a re-import
// reads back. Byte identity across two passes is asserted alongside the count,
// so a future change that made the duplication compound is caught as well.
TEST(Group_repeated_normalization_does_not_multiply_var_children) {
  const std::string model =
      WrapGroups(kThreeAuxes,
                 "      <view name=\"v1\"><group name=\"G1\"><var>alpha</var><var>beta</var></group></view>\n"
                 "      <view name=\"v2\"><group name=\"G1\"><var>alpha</var><var>beta</var></group></view>\n");

  const std::string pass1 = NormalizeSectorXMILE(model);
  CHECK(!pass1.empty());
  if (pass1.empty())
    return;
  CHECK(CountOccurrences(pass1, "<var>alpha</var>") == 1);
  CHECK(CountOccurrences(pass1, "<var>beta</var>") == 1);

  const std::string pass2 = NormalizeSectorXMILE(pass1);
  CHECK(!pass2.empty());
  CHECK_EQ_STR(pass2, pass1);
}

// A variable cannot be in two groups at once -- Variable::SetGroup holds one
// pointer -- so a document that puts it in two is contradictory. The first
// claim wins (matching the Vensim reader, where the banner above the equation
// decides) and the second is reported instead of silently emitting the
// variable's definition under both banners.
TEST(Group_variable_claimed_by_two_groups_is_defined_once) {
  const std::string model = WrapGroups(kThreeAuxes,
                                       "      <group name=\"G1\"><var>alpha</var></group>\n"
                                       "      <group name=\"G2\"><var>alpha</var><var>beta</var></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *g1 = GroupNamed(m, "G1");
  ModelGroup *g2 = GroupNamed(m, "G2");
  CHECK(g1 != nullptr && g2 != nullptr);
  if (g1 && g2) {
    CHECK(g1->vVariables.size() == 1);
    CHECK(g2->vVariables.size() == 1);
  }
  CHECK(AnyErrContains(errs, "already belongs to group"));

  std::vector<std::string> mdlErrs;
  const std::string mdl = m->PrintMDL(mdlErrs);
  CHECK(CountOccurrences(mdl, "\r\nalpha = 1\r\n") == 1);
  delete m;

  const std::vector<std::string> diffs = xmileroundtrip::XmileToMdlDiffs(model);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

// Stella writes group membership as <item uid="N"/> against the per-view uid,
// not as <var>name</var>. XmileView::ProcessViewGroups reads that shape, and it
// has to reach the same registration and membership rules -- so the owner claim
// resolves across a forward reference here too, and an item claimed by two
// groups is not double-counted.
TEST(Group_item_shape_shares_the_owner_and_membership_rules) {
  const std::string model = WrapGroups(kThreeAuxes,
                                       "      <view view_type=\"stock_flow\">\n"
                                       "        <aux name=\"alpha\" uid=\"1\" x=\"100\" y=\"100\"/>\n"
                                       "        <aux name=\"beta\" uid=\"2\" x=\"200\" y=\"100\"/>\n"
                                       "        <group name=\"Inner\" owner=\"Outer\" x=\"90\" y=\"90\">\n"
                                       "          <item uid=\"1\"/><item uid=\"2\"/>\n"
                                       "        </group>\n"
                                       "        <group name=\"Outer\" x=\"80\" y=\"80\"><item uid=\"1\"/></group>\n"
                                       "      </view>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *inner = GroupNamed(m, "Inner");
  ModelGroup *outer = GroupNamed(m, "Outer");
  CHECK(inner != nullptr && outer != nullptr);
  if (inner && outer) {
    CHECK(inner->pOwner == outer);
    CHECK(inner->vVariables.size() == 2);
    // "alpha" was claimed by Inner first, so Outer's repeat of it is refused
    // rather than giving the variable two banners to be emitted under.
    CHECK(outer->vVariables.empty());
  }
  CHECK(AnyErrContains(errs, "already belongs to group"));

  std::vector<std::string> mdlErrs;
  const std::string mdl = m->PrintMDL(mdlErrs);
  CHECK(mdlErrs.empty());
  CHECK(CountOccurrences(mdl, "\r\nalpha = 1\r\n") == 1);
  CHECK(CountOccurrences(mdl, "\r\nbeta = 2\r\n") == 1);
  delete m;
}

// The real corpus case. simlin-test/land_model/land_model.stmx puts "Agriculture
// Land" and "gdp deflator" in two of its five view groups; before the fix the .mdl carried
// "Agriculture Land" twice and "gdp deflator" three times, which Vensim rejects.
// The model stays in the XMILE corpus DEFERRED table for unrelated reasons (its
// sketch geometry drifts across a pass, and the sectors emission drops group
// membership for a model that has a real <view>), so no corpus test converts it
// -- this is the only guard on that regression.
TEST(Group_land_model_corpus_defines_every_variable_once) {
  const std::string stmx =
      ReadFixtureFile(std::string(XMUTIL_SRC_ROOT) + "/test/fixtures/simlin-test/land_model/land_model.stmx");
  CHECK(!stmx.empty());
  if (stmx.empty())
    return;
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(stmx, errs);
  CHECK(m != nullptr);
  if (!m)
    return;

  // The invariant AddGroupMember establishes, checked over the whole model
  // rather than one name: no variable is listed by two groups, and no group
  // lists one twice.
  std::set<Variable *> seen;
  int repeats = 0;
  for (ModelGroup *g : m->Groups()) {
    for (Variable *v : g->vVariables) {
      if (!seen.insert(v).second) {
        repeats++;
        printf("  '%s' is listed by more than one group (last: %s)\n", v->GetName().c_str(), g->sName.c_str());
      }
    }
  }
  CHECK(repeats == 0);

  std::vector<std::string> mdlErrs;
  const std::string mdl = m->PrintMDL(mdlErrs);
  CHECK(mdlErrs.empty());
  CHECK(!mdl.empty());
  CHECK(CountOccurrences(mdl, "\r\nAgriculture Land = INTEG(") == 1);
  CHECK(CountOccurrences(mdl, "\r\ngdp deflator = WITH LOOKUP(") == 1);
  delete m;
}

// A group holding no variables is emitted as no <model> at all, so it cannot
// hold another group's <module> -- and the cross-level <connect> loop used to
// dereference the NULL pModule that left behind. This is the shape ordinary
// Vensim input produces: a banner with no equations under it is an empty group,
// and VensimParse chains the NEXT banner to it as its owner, so any model whose
// banners include an empty one puts a real group under an empty owner.
// test-models/tests/conditional_subscripts carries exactly that (an empty
// ".Vector" banner between ".Matrix" and ".Control").
TEST(Group_vensim_empty_banner_owner_does_not_crash_the_module_path) {
  const std::string mdl = std::string("{UTF-8}\r\n") + Banner("GrpOne", EqnBlock("a", "1")) + Banner("GrpEmpty", "") +
                          Banner("GrpThree", EqnBlock("c", "3")) +
                          Banner("GrpFour", EqnBlock("b", "c") + kControlSection);
  Model *m = ParseMDL(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;

  // GrpFour's "b" reads "c" from GrpThree, so the emission has to carry a cross
  // level between two groups that both hang off the empty GrpEmpty.
  std::vector<std::string> errs;
  const std::string doc = m->PrintXMILE(/*isCompact=*/false, errs, 1.0, 1.0);
  for (const std::string &e : errs)
    printf("  err: %s\n", e.c_str());
  CHECK(errs.empty());
  CHECK(!doc.empty());

  // The dependency edge survives -- NULL-guarding the insert instead of
  // re-pointing the owner would have produced a document without it.
  CHECK(doc.find("<connect to=\"GrpFour.c\" from=\"GrpThree.c\"/>") != std::string::npos);

  // ...and every module reference resolves. The empty group contributes no
  // <model>, so it must not be referenced by one either.
  const std::vector<std::string> dangling = DanglingModuleNames(doc);
  for (const std::string &name : dangling)
    printf("  dangling module: %s\n", name.c_str());
  CHECK(dangling.empty());
  CHECK(doc.find("<module name=\"GrpEmpty\"") == std::string::npos);
  CHECK(doc.find("<module name=\"GrpThree\"") != std::string::npos);
  CHECK(doc.find("<module name=\"GrpFour\"") != std::string::npos);
  delete m;
}

// The same defect reached from XMILE, in the three orders the document may
// state it in. `Parent` is declared with no members at all in the first two;
// in the third it is emptied by losing its only member to an earlier claim,
// which is a shape no amount of care in the reader can rule out.
namespace {

// Emit the model and assert the document is whole. An owner holding no
// variables is what used to leave the module path with a NULL pModule to
// dereference; the sectors emission an XMILE-sourced model now takes has no
// such hole -- an empty group is emitted as a <group> like any other, so the
// owner claim resolves rather than needing to be re-pointed -- and this pins
// that. The re-parse is part of the assertion: an owner naming a group the
// document does not contain would come back with the nesting silently gone.
void CheckEmptyOwnerEmission(const std::string &model) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  std::vector<std::string> printErrs;
  const std::string doc = m->PrintXMILE(/*isCompact=*/false, printErrs, 1.0, 1.0);
  for (const std::string &e : printErrs)
    printf("  err: %s\n", e.c_str());
  CHECK(printErrs.empty());
  CHECK(!doc.empty());
  CHECK(CountModelElements(doc) == 1);
  const std::vector<std::string> dangling = DanglingGroupOwners(doc);
  for (const std::string &name : dangling)
    printf("  dangling group owner: %s\n", name.c_str());
  CHECK(dangling.empty());
  // The empty owner survives as a group of its own, so "Kid" keeps the nesting
  // the document stated instead of being flattened to the top level.
  CHECK(doc.find("<group name=\"Kid\" owner=\"Parent\">") != std::string::npos);
  // Every variable still has exactly one definition -- an emission that lost a
  // group would take its equations with it.
  CHECK(CountOccurrences(doc, "<aux name=\"a\">") == 1);
  CHECK(CountOccurrences(doc, "<aux name=\"b\">") == 1);
  CHECK(CountOccurrences(doc, "<aux name=\"c\">") == 1);
  delete m;

  const std::vector<std::string> diffs = SectorRoundTripDiffs(model);
  for (const std::string &d : diffs)
    printf("  diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

const char *const kThreeAuxesChained =
    "      <aux name=\"a\"><eqn>1</eqn></aux>\n"
    "      <aux name=\"b\"><eqn>a</eqn></aux>\n"
    "      <aux name=\"c\"><eqn>2</eqn></aux>\n";

}  // namespace

TEST(Group_empty_owner_declared_after_its_child_does_not_crash) {
  CheckEmptyOwnerEmission(WrapGroups(kThreeAuxesChained,
                                     "      <group name=\"Kid\" owner=\"Parent\"><var>a</var></group>\n"
                                     "      <group name=\"G2\"><var>b</var></group>\n"
                                     "      <group name=\"G3\"><var>c</var></group>\n"
                                     "      <group name=\"Parent\"></group>\n"));
}

TEST(Group_empty_owner_declared_before_its_child_does_not_crash) {
  CheckEmptyOwnerEmission(WrapGroups(kThreeAuxesChained,
                                     "      <group name=\"Parent\"></group>\n"
                                     "      <group name=\"Kid\" owner=\"Parent\"><var>a</var></group>\n"
                                     "      <group name=\"G2\"><var>b</var></group>\n"
                                     "      <group name=\"G3\"><var>c</var></group>\n"));
}

TEST(Group_owner_emptied_by_a_competing_claim_does_not_crash) {
  CheckEmptyOwnerEmission(WrapGroups(kThreeAuxesChained,
                                     "      <group name=\"First\"><var>c</var></group>\n"
                                     "      <group name=\"Parent\"><var>c</var></group>\n"
                                     "      <group name=\"Kid\" owner=\"Parent\"><var>a</var></group>\n"
                                     "      <group name=\"G2\"><var>b</var></group>\n"));
}

// Several empty ancestors in a row. The sectors emission an XMILE-sourced model
// takes keeps every link, because it emits a <group> for the empty ancestors
// too -- so the whole chain has to survive a round trip, not just its endpoints.
TEST(Group_chain_of_empty_owners_survives_xmile_normalization) {
  const std::string model = WrapGroups(kThreeAuxesChained,
                                       "      <group name=\"Top\"><var>c</var></group>\n"
                                       "      <group name=\"E1\" owner=\"Top\"></group>\n"
                                       "      <group name=\"E2\" owner=\"E1\"></group>\n"
                                       "      <group name=\"E3\" owner=\"E2\"></group>\n"
                                       "      <group name=\"Kid\" owner=\"E3\"><var>a</var></group>\n"
                                       "      <group name=\"G2\"><var>b</var></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *kid = GroupNamed(m, "Kid");
  CHECK(kid != nullptr);
  if (kid)
    CHECK(OwnerDepth(kid, 64) == 4);

  std::vector<std::string> printErrs;
  const std::string doc = m->PrintXMILE(/*isCompact=*/false, printErrs, 1.0, 1.0);
  CHECK(printErrs.empty());
  CHECK(!doc.empty());
  CHECK(CountModelElements(doc) == 1);
  CHECK(DanglingGroupOwners(doc).empty());
  CHECK(doc.find("<group name=\"E1\" owner=\"Top\"/>") != std::string::npos);
  CHECK(doc.find("<group name=\"E2\" owner=\"E1\"/>") != std::string::npos);
  CHECK(doc.find("<group name=\"E3\" owner=\"E2\"/>") != std::string::npos);
  CHECK(doc.find("<group name=\"Kid\" owner=\"E3\">") != std::string::npos);
  delete m;

  // Re-reading the emitted document has to rebuild the same 4-deep chain.
  std::vector<std::string> reparseErrs;
  Model *m1 = xmileroundtrip::ParseXMILE(doc, reparseErrs);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  CHECK(OwnerDepth(GroupNamed(m1, "Kid"), 64) == 4);
  delete m1;
}

// The same shape reaching the MODULE emission, which only Vensim input takes
// now. A group holding no variables is emitted as no <model>, so it has no
// <variables> list to hold a child's <module> -- the walk has to pass through
// every empty ancestor and land on the nearest one that really is emitted, not
// merely skip a single level. Vensim states the nesting in the banner path, so
// a run of empty levels between two populated ones is written out longhand
// here; the group's name is that path with the separators folded to '-'.
TEST(Group_vensim_chain_of_empty_banner_owners_resolves_to_the_nearest_emitted_ancestor) {
  const std::string mdl = std::string("{UTF-8}\r\n") + Banner("Top", EqnBlock("c", "2")) + Banner("Top.E1", "") +
                          Banner("Top.E1.E2", "") + Banner("Top.E1.E2.E3", "") +
                          Banner("Top.E1.E2.E3.Kid", EqnBlock("a", "1")) +
                          Banner("G2", EqnBlock("b", "a") + kControlSection);
  Model *m = ParseMDL(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  // The banners really did chain through all three empty groups.
  const char *const kKid = "Top-E1-E2-E3-Kid";
  ModelGroup *kid = GroupNamed(m, kKid);
  CHECK(kid != nullptr);
  if (kid)
    CHECK(OwnerDepth(kid, 64) == 4);

  std::vector<std::string> printErrs;
  const std::string doc = m->PrintXMILE(/*isCompact=*/false, printErrs, 1.0, 1.0);
  for (const std::string &e : printErrs)
    printf("  err: %s\n", e.c_str());
  CHECK(printErrs.empty());
  CHECK(!doc.empty());
  const std::vector<std::string> dangling = DanglingModuleNames(doc);
  for (const std::string &name : dangling)
    printf("  dangling module: %s\n", name.c_str());
  CHECK(dangling.empty());
  CHECK(doc.find(std::string("<connect to=\"G2.a\" from=\"") + kKid + ".a\"/>") != std::string::npos);
  // "Top" is the nearest ancestor that holds a variable, so Kid's module lands
  // inside Top's model rather than at the root.
  const std::string root = RootModelBody(doc);
  CHECK(root.find(std::string("<module name=\"") + kKid + "\"") == std::string::npos);
  CHECK(root.find("<module name=\"Top\"") != std::string::npos);
  const size_t topModel = doc.find("<model name=\"Top\">");
  CHECK(topModel != std::string::npos);
  if (topModel != std::string::npos)
    CHECK(doc.find(std::string("<module name=\"") + kKid + "\"", topModel) != std::string::npos);
  delete m;
}

// The module writer walks the owner chain, so a cyclic Model has to terminate it
// rather than hang. Neither reader can build a cycle -- ResolveGroupOwners
// refuses the link that would close one and VensimParse chains banners linearly
// -- so this one is built by hand, which is how a writer's input arrives
// malformed in practice: from a caller that is not one of our readers. It is
// driven from .mdl because that is the only input the module path serves. This
// is not a red-before-green case (the pre-fix writer read pOwner once and had no
// walk to hang); it pins the property that fix introduced a way to lose.
TEST(Group_cyclic_owner_chain_terminates_the_writer) {
  // The control section rides along with "Other" so that E1 and E2 really do
  // hold no variables -- a banner carrying the controls is a populated group
  // even though every one of them is Unwanted by the time the writer runs.
  const std::string mdl = std::string("{UTF-8}\r\n") + Banner("Real", EqnBlock("a", "1")) +
                          Banner("Other", EqnBlock("b", "a") + kControlSection) + Banner("E1", "") + Banner("E2", "");
  Model *m = ParseMDL(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *real = GroupNamed(m, "Real");
  ModelGroup *e1 = GroupNamed(m, "E1");
  ModelGroup *e2 = GroupNamed(m, "E2");
  CHECK(real != nullptr && e1 != nullptr && e2 != nullptr);
  if (!real || !e1 || !e2) {
    delete m;
    return;
  }
  real->pOwner = e1;
  e1->pOwner = e2;
  e2->pOwner = e1;

  std::vector<std::string> printErrs;
  const std::string doc = m->PrintXMILE(/*isCompact=*/false, printErrs, 1.0, 1.0);
  CHECK(printErrs.empty());
  CHECK(!doc.empty());
  CHECK(DanglingModuleNames(doc).empty());
  // "Other" reads "a" from "Real", so the cross level still has to be written,
  // which means "Real" still has a module to hang it on.
  CHECK(doc.find("<connect to=\"Other.a\" from=\"Real.a\"/>") != std::string::npos);
  CHECK(RootModelBody(doc).find("<module name=\"Real\"") != std::string::npos);
  delete m;
}

// A name that is only whitespace normalizes to the empty string, which the
// reader accepts. The writer used to assert on it (abort in a debug build) and,
// with NDEBUG, emit name="" -- a module nothing can reference. It now names the
// group instead, because dropping the group would drop its equations with it:
// generateModelAsGroups emits equations only through the groups.
TEST(Group_blank_name_is_replaced_rather_than_asserted) {
  const std::string model = WrapGroups(kThreeAuxesChained,
                                       "      <group name=\"   \"><var>a</var></group>\n"
                                       "      <group name=\"Real\"><var>b</var></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;

  std::vector<std::string> printErrs;
  const std::string doc = m->PrintXMILE(/*isCompact=*/false, printErrs, 1.0, 1.0);
  CHECK(printErrs.empty());
  CHECK(!doc.empty());
  CHECK(doc.find("name=\"\"") == std::string::npos);
  CHECK(DanglingGroupOwners(doc).empty());
  // The blank group's variable is still defined somewhere.
  CHECK(doc.find("<aux name=\"a\">") != std::string::npos);
  delete m;

  // The sectors path names groups too, and emitted <group name=""> before.
  Model *m2 = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m2 != nullptr);
  if (!m2)
    return;
  const std::string sectors = EmitSectorXMILE(m2);
  CHECK(!sectors.empty());
  CHECK(sectors.find("name=\"\"") == std::string::npos);
  delete m2;
}

// An <item uid="N"/> that resolves to an <alias> places a GHOST inside the
// group rectangle; where the variable itself lives is what the REAL element
// says. With first-claim-wins membership, a ghost listed first used to take the
// variable from the group its real element sits in -- deciding the answer by
// document order between a drawing and a definition.
TEST(Group_real_item_outranks_a_ghost_item) {
  const std::string model = WrapGeometryView(
      "        <aux name=\"alpha\" uid=\"1\" x=\"500\" y=\"100\"/>\n"
      "        <aux name=\"beta\" uid=\"2\" x=\"520\" y=\"200\"/>\n"
      "        <alias uid=\"3\" x=\"100\" y=\"100\"><of>alpha</of></alias>\n"
      "        <group name=\"GhostSide\" x=\"90\" y=\"90\"><item uid=\"3\"/></group>\n"
      "        <group name=\"RealSide\" x=\"480\" y=\"90\"><item uid=\"1\"/><item uid=\"2\"/></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(GroupOf(m, "alpha"), "RealSide");
  ModelGroup *ghostSide = GroupNamed(m, "GhostSide");
  CHECK(ghostSide != nullptr);
  if (ghostSide)
    CHECK(ghostSide->vVariables.empty());
  // A ghost losing to a real placement is not a contradiction the modeler
  // needs told about: it never claimed the variable in the first place.
  for (const std::string &e : errs)
    printf("  err: %s\n", e.c_str());
  CHECK(!AnyErrContains(errs, "already belongs to group"));
  delete m;
}

// A variable drawn only as a ghost in this view still has its claim honored --
// its real element lives in a view this reader does not process, so the ghost
// is the only thing that says anything about it.
TEST(Group_ghost_item_still_claims_a_variable_no_real_item_placed) {
  const std::string model = WrapGeometryView(
      "        <aux name=\"beta\" uid=\"2\" x=\"520\" y=\"200\"/>\n"
      "        <alias uid=\"3\" x=\"100\" y=\"100\"><of>alpha</of></alias>\n"
      "        <group name=\"GhostSide\" x=\"90\" y=\"90\"><item uid=\"3\"/></group>\n"
      "        <group name=\"RealSide\" x=\"480\" y=\"90\"><item uid=\"2\"/></group>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(GroupOf(m, "alpha"), "GhostSide");
  CHECK_EQ_STR(GroupOf(m, "beta"), "RealSide");
  delete m;
}

// A childless <group name="X" owner="Y"/> inside a <view> states a nesting the
// same way the identical element directly under <views> does. It used to be
// skipped entirely there -- so neither the group nor its owner claim was
// registered -- and the same document meant two different things depending on
// where the element sat.
TEST(Group_childless_view_group_is_registered_with_its_owner) {
  const std::string model = WrapGeometryView(
      "        <aux name=\"alpha\" uid=\"1\" x=\"100\" y=\"100\"/>\n"
      "        <group name=\"Inner\" owner=\"Outer\" x=\"90\" y=\"90\"><item uid=\"1\"/></group>\n"
      "        <group name=\"Outer\" owner=\"Roof\" x=\"80\" y=\"80\"/>\n"
      "        <group name=\"Roof\" x=\"70\" y=\"70\"/>\n");
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(model, errs);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *inner = GroupNamed(m, "Inner");
  ModelGroup *outer = GroupNamed(m, "Outer");
  ModelGroup *roof = GroupNamed(m, "Roof");
  CHECK(inner != nullptr);
  CHECK(outer != nullptr);
  CHECK(roof != nullptr);
  if (inner && outer && roof) {
    CHECK(inner->pOwner == outer);
    CHECK(outer->pOwner == roof);
    CHECK(roof->pOwner == nullptr);
  }
  // Neither owner="..." was left dangling, which is what the skip caused.
  for (const std::string &e : errs)
    printf("  err: %s\n", e.c_str());
  CHECK(!AnyErrContains(errs, "names no group"));
  delete m;
}

// "Holds variables" and "is emitted" were two different questions the module
// writer answered with one test. Emission filters out Unwanted() members --
// the four control variables, which generateSimSpecs marks before any of this
// runs -- so a `.Control` banner counted as populated, earned a <module>
// reference and a <model>, and shipped that model with an empty <variables/>.
// A module whose model declares nothing is a level of structure the source
// never had, and the re-import reads it back as one.
TEST(Group_all_unwanted_members_is_not_an_emitted_group) {
  const std::string mdl = std::string("{UTF-8}\r\n") + Banner("One", EqnBlock("a", "1")) +
                          Banner("Two", EqnBlock("b", "a")) + Banner("Three", EqnBlock("c", "b")) +
                          Banner(".Control", kControlSection);
  Model *m = ParseMDL(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;

  std::vector<std::string> printErrs;
  const std::string doc = m->PrintXMILE(/*isCompact=*/false, printErrs, 1.0, 1.0);
  delete m;
  CHECK(printErrs.empty());
  CHECK(!doc.empty());

  // The user groups are still decomposed into modules...
  CHECK(CountOccurrences(doc, "<module name=\"One\"") == 1);
  // ...and the control-only banner contributes neither half of a module.
  if (CountOccurrences(doc, "<module name=\"Control\"") != 0)
    printf("%s\n", doc.c_str());
  CHECK(CountOccurrences(doc, "<module name=\"Control\"") == 0);
  CHECK(CountOccurrences(doc, "<model name=\"Control\"") == 0);
  // Nothing else acquired an empty variables list either.
  CHECK(CountOccurrences(doc, "<variables/>") == 0);
  CHECK(DanglingModuleNames(doc).empty());
}
