// Vensim states group nesting in the banner NAME: a nested group's banner
// carries its whole path, '.'-separated, with a leading '.' on the outermost
// level (`.Physics`, `.Physics.Forcing`, `.Physics.Forcing.Aerosols`). xmutil
// folds those separators to '-' for the ModelGroup name, because that name also
// becomes an XMILE <module name=...>, which cannot carry a '.'.
//
// Two halves used to be missing. The reader ignored the path and owned each new
// banner by the PREVIOUS one, so every model came out as one linear chain --
// including `.Control`, which ended up nested inside the last user group. The
// writer emitted only the group's own name, so whatever nesting a model did
// have was discarded on the way out and re-invented as that chain on the way
// back in. `C-LEARN v77` really does declare a three-level forest this way.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Symbol.h"
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

std::string Banner(const char *name) {
  const std::string bar(56, '*');
  return "\r\n" + bar + "\r\n\t" + name + "\r\n" + bar + "~\r\n\t\t\r\n\t|\r\n";
}

// Three groups: Inner nested under Outer, Third alongside them.
std::string NestedModel() {
  return "{UTF-8}\r\n" + Banner(".Outer") + "a = 1\r\n\t~~|\r\n" + Banner(".Outer.Inner") + "b = 2\r\n\t~~|\r\n" +
         Banner(".Third") + "c = 3\r\n\t~~|\r\n";
}

// The owner NAME of a group, or "<none>" when it is a root, or "<missing>"
// when the group itself was never created.
std::string OwnerOf(Model *m, const std::string &name) {
  ModelGroup *g = GroupNamed(m, name);
  if (!g)
    return "<missing>";
  return g->pOwner ? g->pOwner->sName : "<none>";
}

}  // namespace

TEST(GroupNesting_banner_path_sets_the_owner) {
  Model *m = roundtrip::ParseVensim(NestedModel());
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(OwnerOf(m, "Outer"), "<none>");
  CHECK_EQ_STR(OwnerOf(m, "Outer-Inner"), "Outer");
  CHECK_EQ_STR(OwnerOf(m, "Third"), "<none>");
  delete m;
}

// The defect the round trip actually shipped: a flat model came back nested.
TEST(GroupNesting_flat_banners_stay_flat) {
  const std::string mdl = "{UTF-8}\r\n" + Banner(".One") + "a = 1\r\n\t~~|\r\n" + Banner(".Two") +
                          "b = 2\r\n\t~~|\r\n" + Banner(".Three") + "c = 3\r\n\t~~|\r\n";
  Model *m = roundtrip::ParseVensim(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  CHECK_EQ_STR(OwnerOf(m, "One"), "<none>");
  CHECK_EQ_STR(OwnerOf(m, "Two"), "<none>");
  CHECK_EQ_STR(OwnerOf(m, "Three"), "<none>");
  delete m;
}

// .Control is written last by the writer, so under the previous "the last
// banner owns me" rule it was always adopted by the final user group.
TEST(GroupNesting_control_group_is_not_adopted_by_the_last_user_group) {
  Model *m = roundtrip::ParseVensim(NestedModel());
  CHECK(m != nullptr);
  if (!m)
    return;
  std::vector<std::string> errs;
  const std::string out = m->PrintMDL(errs);
  delete m;
  CHECK(!out.empty());

  Model *m2 = roundtrip::ParseVensim(out);
  CHECK(m2 != nullptr);
  if (!m2)
    return;
  CHECK_EQ_STR(OwnerOf(m2, "Control"), "<none>");
  delete m2;
}

// The writer's half: nesting must survive the emission, not just the read.
TEST(GroupNesting_survives_a_round_trip) {
  Model *m = roundtrip::ParseVensim(NestedModel());
  CHECK(m != nullptr);
  if (!m)
    return;
  std::vector<std::string> errs;
  const std::string out = m->PrintMDL(errs);
  delete m;
  CHECK(!out.empty());

  Model *m2 = roundtrip::ParseVensim(out);
  CHECK(m2 != nullptr);
  if (!m2)
    return;
  CHECK_EQ_STR(OwnerOf(m2, "Outer"), "<none>");
  CHECK_EQ_STR(OwnerOf(m2, "Outer-Inner"), "Outer");
  CHECK_EQ_STR(OwnerOf(m2, "Third"), "<none>");
  delete m2;
}

// A '.' inside a group's own name is not a nesting separator. The writer has to
// fold it before emitting, or the banner would read back as one more level of
// nesting than the model has -- an invented parent, and a renamed group.
TEST(GroupNesting_dot_inside_a_group_name_is_not_a_separator) {
  const std::string mdl = "{UTF-8}\r\n" + Banner(".Plain") + "a = 1\r\n\t~~|\r\n";
  Model *m = roundtrip::ParseVensim(mdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  ModelGroup *g = GroupNamed(m, "Plain");
  CHECK(g != nullptr);
  if (g)
    g->sName = "has.a.dot";
  std::vector<std::string> errs;
  const std::string out = m->PrintMDL(errs);
  delete m;

  Model *m2 = roundtrip::ParseVensim(out);
  CHECK(m2 != nullptr);
  if (!m2)
    return;
  CHECK_EQ_STR(OwnerOf(m2, "has-a-dot"), "<none>");
  delete m2;
}
