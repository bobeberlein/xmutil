// The sketch survives .mdl -> XMILE -> .mdl.
//
// XMILE has no place for most of a Vensim sketch record (UIDs, the shape and
// bits words, the font line, valve and cloud records, a name's text extent, the
// point an arc was dragged through), so the trip back rebuilds those from
// Vensim's defaults and from XMILE's standard geometry. What has to survive is
// the drawing: every view under its own name, every element where it was in
// its view, stocks at their own size, ghosts as ghosts, flows with their valve,
// name and pipes, clouds at the pipe ends, and every arrow leaving its source
// at the same angle. The checks below hold the regenerated sketch to that,
// view by view and record by record, against the original Vensim file.
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/SymbolNameSpace.h"
#include "../../src/Symbol/Variable.h"
#include "../../src/Vensim/VensimView.h"
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

// Read from upstream's test_models/ directory, like the C-LEARN stress test.
const char *kSimplePopulationPath = "/test_models/SimplePopulation.mdl";
const char *kPopulationResourcesPath = "/test_models/PopulationResources.mdl";
const char *kNameSizingPath = "/test_models/NameSizing.mdl";

// sectors: write a multi-view model as sectors of one XMILE model (--sectors).
std::string MdlToXmile(const std::string &mdl, bool sectors) {
  char *out = convert_mdl_to_xmile(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "model.mdl", false, -1, sectors);
  if (!out)
    return std::string();
  std::string s(out);
  free(out);
  return s;
}

std::string XmileToMdl(const std::string &xmile) {
  char *out = convert_xmile_to_mdl(xmile.c_str(), static_cast<uint32_t>(xmile.size()), "model.xmile", -1);
  if (!out)
    return std::string();
  std::string s(out);
  free(out);
  return s;
}

// The sketch section: every view's lines from the first opener up to (not
// including) the terminator, without line endings.
std::vector<std::string> SketchLines(const std::string &mdl) {
  std::vector<std::string> lines;
  std::istringstream in(mdl);
  std::string line;
  bool inSketch = false;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    if (line.compare(0, 9, "\\\\\\---///") == 0)
      inSketch = true;
    else if (line.compare(0, 9, "///---\\\\\\") == 0)
      break;
    if (inSketch)
      lines.push_back(line);
  }
  return lines;
}

// Split a record at commas, except inside a quoted name ("a, b" is one field).
std::vector<std::string> Fields(const std::string &line) {
  std::vector<std::string> f;
  std::string cur;
  bool quoted = false;
  for (char c : line) {
    if (c == '"')
      quoted = !quoted;
    if (c == ',' && !quoted) {
      f.push_back(cur);
      cur.clear();
    } else {
      cur += c;
    }
  }
  f.push_back(cur);
  return f;
}

// One sketch record, with the fields the comparison reads.
struct Record {
  int type = 0;
  int uid = 0;
  std::string name;
  int x = 0, y = 0, w = 0, h = 0;
  int shape = 0, bits = 0, tpos = 0;
  int from = 0, to = 0;  // connectors
};

// One view.
struct Sketch {
  std::vector<std::string> header;  // opener, version, *title, font line
  std::map<int, Record> byUid;
};

Record ParseRecord(const std::string &line) {
  std::vector<std::string> f = Fields(line);
  Record r;
  if (f.size() < 7)
    return r;
  r.type = std::atoi(f[0].c_str());
  r.uid = std::atoi(f[1].c_str());
  if (r.type == 1) {
    r.from = std::atoi(f[2].c_str());
    r.to = std::atoi(f[3].c_str());
    r.shape = std::atoi(f[4].c_str());
    // The point list "np|(x,y)|" is split by the comma inside the point:
    // "1|(x" then "y)|".
    for (size_t k = 0; k + 1 < f.size(); k++) {
      size_t p = f[k].find("|(");
      if (p != std::string::npos) {
        r.x = std::atoi(f[k].c_str() + p + 2);
        r.y = std::atoi(f[k + 1].c_str());
      }
    }
  } else {
    r.name = f[2];
    r.x = std::atoi(f[3].c_str());
    r.y = std::atoi(f[4].c_str());
    r.w = std::atoi(f[5].c_str());
    r.h = std::atoi(f[6].c_str());
    if (f.size() > 7)
      r.shape = std::atoi(f[7].c_str());
    if (f.size() > 8)
      r.bits = std::atoi(f[8].c_str());
    if (f.size() > 11)
      r.tpos = std::atoi(f[11].c_str());
  }
  return r;
}

std::vector<Sketch> ParseSketches(const std::string &mdl) {
  std::vector<Sketch> views;
  for (const std::string &line : SketchLines(mdl)) {
    if (line.compare(0, 9, "\\\\\\---///") == 0)
      views.emplace_back();
    if (views.empty())
      continue;
    Sketch &s = views.back();
    if (s.header.size() < 4) {
      s.header.push_back(line);
      continue;
    }
    Record r = ParseRecord(line);
    if (r.type != 0)
      s.byUid[r.uid] = r;
  }
  return views;
}

const Record *Find(const Sketch &s, int uid) {
  auto it = s.byUid.find(uid);
  return it == s.byUid.end() ? nullptr : &it->second;
}

// The variable records, by name.
std::map<std::string, const Record *> Variables(const Sketch &s) {
  std::map<std::string, const Record *> out;
  for (const auto &kv : s.byUid) {
    if (kv.second.type == 10)
      out[kv.second.name] = &kv.second;
  }
  return out;
}

// What a record stands for in the drawing: a variable (or a valve, which is
// its flow -- the record after it) by name, or a cloud.
std::string Identity(const Sketch &s, int uid) {
  const Record *r = Find(s, uid);
  if (!r)
    return "?";
  if (r->type == 10)
    return r->name;
  if (r->type == 11) {
    const Record *flow = Find(s, uid + 1);
    return flow ? flow->name : "?";
  }
  if (r->type == 12)
    return "cloud";
  return "?";
}

// The valve record of the flow named `name`.
const Record *ValveOf(const Sketch &s, const std::string &name) {
  for (const auto &kv : s.byUid) {
    if (kv.second.type == 11 && Identity(s, kv.first) == name)
      return &kv.second;
  }
  return nullptr;
}

double AngleDiff(double a, double b) {
  double d = std::fmod(std::fabs(a - b), 360.0);
  return d > 180 ? 360 - d : d;
}

// What CompareView found, so a caller can check the comparison was not vacuous.
struct ViewCounts {
  size_t variables = 0, ghosts = 0, valves = 0, clouds = 0, pipes = 0, arrows = 0;
};

// Hold the regenerated view b to the original view a. compareNameSizes is off
// when b's names are not a's (module-qualified ones are longer, so sized
// differently).
ViewCounts CompareView(const Sketch &a, const Sketch &b, bool compareNameSizes = true) {
  ViewCounts counts;

  // The frame header comes back verbatim: opener, version, title, font line.
  CHECK(a.header.size() == 4 && b.header.size() == 4);
  for (size_t i = 0; i < a.header.size() && i < b.header.size(); i++)
    CHECK_EQ_STR(b.header[i], a.header[i]);

  // Every variable record comes back at the same place, with the same shape,
  // bits and name placement. A stock keeps its size (XMILE carries it). A
  // name's text extent is re-estimated -- Vensim wraps long names, so the
  // estimate can be well off -- and only has to be there.
  std::map<std::string, const Record *> va = Variables(a);
  std::map<std::string, const Record *> vb = Variables(b);
  counts.variables = va.size();
  CHECK(va.size() == vb.size());
  for (const auto &kv : va) {
    auto it = vb.find(kv.first);
    CHECK(it != vb.end());
    if (it == vb.end()) {
      printf("  variable record missing: %s\n", kv.first.c_str());
      continue;
    }
    const Record &ra = *kv.second;
    const Record &rb = *it->second;
    if ((ra.bits & 1) == 0)
      counts.ghosts++;
    // A flow's name sits beside its valve at a distance that depends on the
    // name's own size, so it can only be compared when the names are the same.
    const bool placedBySize = (ra.shape & 32) != 0 && !compareNameSizes;
    if (!placedBySize) {
      if (ra.x != rb.x || ra.y != rb.y)
        printf("  %s moved: (%d,%d) -> (%d,%d)\n", kv.first.c_str(), ra.x, ra.y, rb.x, rb.y);
      CHECK(ra.x == rb.x && ra.y == rb.y);
    }
    if (ra.shape != rb.shape || ra.bits != rb.bits || ra.tpos != rb.tpos)
      printf("  %s: shape/bits/tpos %d/%d/%d -> %d/%d/%d\n", kv.first.c_str(), ra.shape, ra.bits, ra.tpos, rb.shape,
             rb.bits, rb.tpos);
    CHECK(ra.shape == rb.shape);
    CHECK(ra.bits == rb.bits);
    CHECK(ra.tpos == rb.tpos);
    if (ra.shape == 3) {
      CHECK(ra.w == rb.w && ra.h == rb.h);
    } else if (!compareNameSizes) {
      CHECK(rb.w > 0 && rb.h > 0);
    } else {
      // A name comes back at the size Vensim gives a new one, which matches a
      // name Vensim sized itself to within a few pixels (and exactly in
      // height: the same number of lines).
      if (std::abs(ra.w - rb.w) > 4 || ra.h != rb.h)
        printf("  %s sized %dx%d, Vensim had %dx%d\n", kv.first.c_str(), rb.w, rb.h, ra.w, ra.h);
      CHECK(std::abs(ra.w - rb.w) <= 4);
      CHECK(ra.h == rb.h);
    }
  }

  // Each flow's valve comes back exactly.
  for (const auto &kv : va) {
    const Record *valveA = ValveOf(a, kv.first);
    if (!valveA)
      continue;
    counts.valves++;
    const Record *valveB = ValveOf(b, kv.first);
    CHECK(valveB != nullptr);
    if (!valveB)
      continue;
    CHECK(valveA->x == valveB->x && valveA->y == valveB->y);
    CHECK(valveA->w == valveB->w && valveA->h == valveB->h);
    CHECK(valveA->shape == valveB->shape && valveA->tpos == valveB->tpos);
  }

  // Clouds: the same number, each within a pixel of an original one (a pipe is
  // straight along its valve's row, which can move a cloud drawn a pixel off
  // it), at the same size.
  std::vector<const Record *> cloudsA, cloudsB;
  for (const auto &kv : a.byUid)
    if (kv.second.type == 12)
      cloudsA.push_back(&kv.second);
  for (const auto &kv : b.byUid)
    if (kv.second.type == 12)
      cloudsB.push_back(&kv.second);
  counts.clouds = cloudsA.size();
  CHECK(cloudsA.size() == cloudsB.size());
  for (const Record *ca : cloudsA) {
    bool matched = false;
    for (const Record *cb : cloudsB) {
      if (std::abs(ca->x - cb->x) <= 1 && std::abs(ca->y - cb->y) <= 1 && ca->w == cb->w && ca->h == cb->h &&
          ca->name == cb->name && ca->shape == cb->shape)
        matched = true;
    }
    if (!matched)
      printf("  no cloud near (%d,%d)\n", ca->x, ca->y);
    CHECK(matched);
  }

  // Pipes: each flow keeps its two segments, with the same roles (4 at the
  // downstream end, 100 upstream) running to the same stock or cloud.
  auto pipes = [](const Sketch &s) {
    std::set<std::string> out;
    for (const auto &kv : s.byUid) {
      const Record &r = kv.second;
      if (r.type == 1 && (r.shape == 4 || r.shape == 100))
        out.insert(Identity(s, r.from) + " " + std::to_string(r.shape) + " " + Identity(s, r.to));
    }
    return out;
  };
  std::set<std::string> pa = pipes(a);
  std::set<std::string> pb = pipes(b);
  counts.pipes = pa.size();
  CHECK(pa == pb);

  // Arrows: the same source and target, now always ending on a flow's valve,
  // and leaving the source at the same angle. The arc point itself is rebuilt
  // as the middle of the arc, so it is not compared.
  auto arrows = [](const Sketch &s) {
    std::map<std::string, const Record *> out;
    for (const auto &kv : s.byUid) {
      const Record &r = kv.second;
      if (r.type == 1 && r.shape == 1)
        out[Identity(s, r.from) + " -> " + Identity(s, r.to)] = &r;
    }
    return out;
  };
  std::map<std::string, const Record *> aa = arrows(a);
  std::map<std::string, const Record *> ab = arrows(b);
  counts.arrows = aa.size();
  CHECK(aa.size() == ab.size());
  for (const auto &kv : aa) {
    auto it = ab.find(kv.first);
    CHECK(it != ab.end());
    if (it == ab.end()) {
      printf("  arrow missing: %s\n", kv.first.c_str());
      continue;
    }
    const Record &ra = *kv.second;
    const Record &rb = *it->second;
    const Record *toB = Find(b, rb.to);
    // An arrow into a flow ends on its valve, never on its name.
    if (ValveOf(b, Identity(b, rb.to))) {
      if (!toB || toB->type != 11)
        printf("  %s does not end on the flow's valve\n", kv.first.c_str());
      CHECK(toB && toB->type == 11);
    }
    const Record *fa = Find(a, ra.from);
    const Record *ta = Find(a, ra.to);
    const Record *fb = Find(b, rb.from);
    if (!fa || !ta || !fb || !toB)
      continue;
    double angleA = AngleFromPoints(fa->x, fa->y, ra.x, ra.y, ta->x, ta->y);
    double angleB = AngleFromPoints(fb->x, fb->y, rb.x, rb.y, toB->x, toB->y);
    if (AngleDiff(angleA, angleB) > 1.0)
      printf("  %s takeoff %.2f -> %.2f\n", kv.first.c_str(), angleA, angleB);
    CHECK(AngleDiff(angleA, angleB) <= 1.0);
  }

  // Nothing is left over: the regenerated view has exactly these records.
  CHECK(b.byUid.size() == va.size() + counts.valves + cloudsA.size() + pa.size() + aa.size());
  return counts;
}

// The variable records' names without their module qualification
// ("Population View.births" -> "births", ".x" -> "x"), so a view read from
// modules can be compared with the view it came from.
Sketch StripModulePrefixes(Sketch s) {
  for (auto &kv : s.byUid) {
    std::string &name = kv.second.name;
    if (kv.second.type != 10)
      continue;
    if (name.size() >= 2 && name.front() == '"' && name.back() == '"')
      name = name.substr(1, name.size() - 2);
    size_t dot = name.find('.');
    if (dot != std::string::npos)
      name = name.substr(dot + 1);
  }
  return s;
}

}  // namespace

TEST(SketchMdlXmile_point_from_angle_inverts_angle_from_points) {
  // A point on an arc, then its takeoff angle, then the arc rebuilt from that
  // angle: the rebuilt point must lie on the same arc, so it yields the same
  // takeoff angle again -- for arcs bending either way, minor and major, and a
  // straight connector.
  struct Case {
    double sx, sy, px, py, ex, ey;
  };
  const Case cases[] = {
      {200, 299, 163, 251, 191, 189},  // SimplePopulation's birth rate -> births
      {288, 178, 338, 124, 426, 178},  // bends up
      {288, 178, 338, 232, 426, 178},  // bends down
      {100, 100, 50, 150, 100, 200},   // vertical chord, bulging left
      {100, 100, 300, 150, 120, 110},  // a major arc (more than a half circle)
  };
  for (const Case &c : cases) {
    double angle = AngleFromPoints(c.sx, c.sy, c.px, c.py, c.ex, c.ey);
    double px, py;
    PointFromAngle(c.sx, c.sy, c.ex, c.ey, angle, px, py);
    double again = AngleFromPoints(c.sx, c.sy, px, py, c.ex, c.ey);
    if (AngleDiff(angle, again) > 1e-6)
      printf("  angle %.9f came back as %.9f (point %.3f,%.3f)\n", angle, again, px, py);
    CHECK(AngleDiff(angle, again) < 1e-6);
  }
  // Straight: the angle along the chord gives the chord's midpoint.
  double px, py;
  PointFromAngle(0, 0, 100, 0, 0, px, py);
  CHECK(std::fabs(px - 50) < 1e-9 && std::fabs(py) < 1e-9);
}

// Names Vensim laid out itself, from one character to a few hundred: the
// default sizing reproduces each to within 2px wide and exactly in height.
TEST(SketchMdlXmile_default_name_size_matches_vensim) {
  const std::string mdl = ReadFile(std::string(XMUTIL_SRC_ROOT) + kNameSizingPath);
  CHECK(!mdl.empty());
  if (mdl.empty()) {
    printf("  fixture missing: %s\n", kNameSizingPath);
    return;
  }
  std::vector<Sketch> views = ParseSketches(mdl);
  CHECK(views.size() == 1);
  size_t names = 0;
  for (const Sketch &view : views) {
    for (const auto &kv : view.byUid) {
      const Record &r = kv.second;
      // Only a variable drawn as its name alone (shape 8) is sized by its
      // name; a stock's box is 40x20 whatever it is called.
      if (r.type != 10 || r.shape != 8)
        continue;
      std::string name = r.name;
      if (name.size() >= 2 && name.front() == '"' && name.back() == '"')
        name = name.substr(1, name.size() - 2);
      int hw, hh;
      VensimDefaultNameSize(name, false, hw, hh);
      if (std::abs(hw - r.w) > 2 || hh != r.h)
        printf("  '%.30s' sized %dx%d, Vensim %dx%d\n", name.c_str(), hw, hh, r.w, r.h);
      CHECK(std::abs(hw - r.w) <= 2);
      CHECK(hh == r.h);
      names++;
    }
  }
  CHECK(names == 5);
}

TEST(SketchMdlXmile_simple_population_round_trips) {
  const std::string original = ReadFile(std::string(XMUTIL_SRC_ROOT) + kSimplePopulationPath);
  CHECK(!original.empty());
  if (original.empty()) {
    printf("  fixture missing: %s\n", kSimplePopulationPath);
    return;
  }
  const std::string xmile = MdlToXmile(original, false);
  CHECK(!xmile.empty());
  const std::string back = XmileToMdl(xmile);
  CHECK(!back.empty());
  if (xmile.empty() || back.empty())
    return;

  std::vector<Sketch> a = ParseSketches(original);
  std::vector<Sketch> b = ParseSketches(back);
  CHECK(a.size() == 1 && b.size() == 1);
  if (a.size() != 1 || b.size() != 1)
    return;
  ViewCounts counts = CompareView(a[0], b[0]);
  CHECK(counts.variables == 5 && counts.valves == 2 && counts.clouds == 2);
  CHECK(counts.pipes == 4 && counts.arrows == 4);

  // After the first trip the .mdl sketch is a fixpoint: going around again
  // reproduces it exactly.
  const std::string again = XmileToMdl(MdlToXmile(back, false));
  CHECK(SketchLines(again) == SketchLines(back));
}

// Two sectors, with arrows from `a` (sector A) to `b` and to the flow `f`
// (both sector B). A Vensim arrow cannot cross views, so sector B's view gets a
// shadow of `a` next to each target -- on the side facing where `a` is drawn
// -- and a straight arrow from it.
TEST(SketchMdlXmile_arrow_across_sectors_gets_a_shadow_source) {
  const char *kXmile = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables>
      <aux name="a"><eqn>1</eqn></aux>
      <aux name="b"><eqn>a</eqn></aux>
      <flow name="f"><eqn>a</eqn></flow>
    </variables>
    <views><view>
      <group name="A" x="0" y="0" width="400" height="200"/>
      <aux name="a" x="100" y="100"/>
      <group name="B" x="0" y="300" width="400" height="200"/>
      <aux name="b" x="100" y="400"/>
      <flow name="f" x="250" y="450"><pts><pt x="200" y="450"/><pt x="300" y="450"/></pts></flow>
      <connector uid="1" angle="45" polarity="+"><from>a</from><to>b</to></connector>
      <connector uid="2" angle="-30"><from>a</from><to>f</to></connector>
    </view></views>
  </model>
</xmile>
)";
  const std::string mdl = XmileToMdl(kXmile);
  CHECK(!mdl.empty());
  std::vector<Sketch> views = ParseSketches(mdl);
  CHECK(views.size() == 2);
  if (views.size() != 2)
    return;
  CHECK_EQ_STR(views[0].header[2], "*A");
  CHECK_EQ_STR(views[1].header[2], "*B");

  // Sector A keeps `a` and draws no arrow: both arrows end in B.
  for (const auto &kv : views[0].byUid)
    CHECK(kv.second.type != 1);

  const Sketch &b = views[1];
  // `a` as seen from sector B's view: sector B's corner is (0,300).
  const double ax = 100, ay = 100 - 300;
  size_t shadows = 0, arrows = 0;
  for (const auto &kv : b.byUid) {
    const Record &r = kv.second;
    if (r.type == 10 && r.name == "a") {
      CHECK((r.bits & 1) == 0);  // a ghost: a is defined in sector A
      shadows++;
    }
    if (r.type != 1 || r.shape != 1)
      continue;
    arrows++;
    const Record *from = Find(b, r.from);
    const Record *to = Find(b, r.to);
    CHECK(from && from->type == 10 && from->name == "a" && (from->bits & 1) == 0);
    CHECK(to != nullptr);
    if (!from || !to)
      continue;
    // The flow's arrow ends on its valve.
    if (Identity(b, r.to) == "f")
      CHECK(to->type == 11);
    // Straight: the arrow's point is the midpoint (to within rounding).
    CHECK(std::abs(r.x - (from->x + to->x) / 2) <= 1 && std::abs(r.y - (from->y + to->y) / 2) <= 1);
    // The shadow sits on the line from the target toward a, on a's side, and
    // close to the target.
    double tx = ax - to->x, ty = ay - to->y;            // target -> a
    double sx = from->x - to->x, sy = from->y - to->y;  // target -> shadow
    double sl = std::hypot(sx, sy), tl = std::hypot(tx, ty);
    CHECK(sl > 0 && sl < 120);
    double cosine = (sx * tx + sy * ty) / (sl * tl);
    if (cosine < 0.999)
      printf("  shadow at (%d,%d) is off the line toward a from (%d,%d)\n", from->x, from->y, to->x, to->y);
    CHECK(cosine > 0.999);
  }
  CHECK(shadows == 2);
  CHECK(arrows == 2);
  // The polarity survives on the redrawn arrow.
  CHECK(mdl.find(",0,43,0,0,64,") != std::string::npos);
}

// A model with two views, written with --sectors: each view becomes a sector
// (an XMILE <group> drawn around it), and on the way back each sector becomes a
// Vensim view again, named after the sector, holding the elements drawn inside
// it at their own view's coordinates. Each view shows a ghost of a variable
// defined in the other, so this also holds ghosts and real placements apart.
TEST(SketchMdlXmile_population_resources_sectors_round_trip) {
  const std::string original = ReadFile(std::string(XMUTIL_SRC_ROOT) + kPopulationResourcesPath);
  CHECK(!original.empty());
  if (original.empty()) {
    printf("  fixture missing: %s\n", kPopulationResourcesPath);
    return;
  }
  const std::string xmile = MdlToXmile(original, true);
  CHECK(!xmile.empty());
  CHECK(xmile.find("<group name=\"Population View\"") != std::string::npos);
  CHECK(xmile.find("<group name=\"Resource View\"") != std::string::npos);
  const std::string back = XmileToMdl(xmile);
  CHECK(!back.empty());
  if (xmile.empty() || back.empty())
    return;

  std::vector<Sketch> a = ParseSketches(original);
  std::vector<Sketch> b = ParseSketches(back);
  CHECK(a.size() == 2);
  CHECK(a.size() == b.size());
  if (a.size() != b.size())
    return;
  ViewCounts total;
  for (size_t i = 0; i < a.size(); i++) {
    ViewCounts c = CompareView(a[i], b[i]);
    total.variables += c.variables;
    total.ghosts += c.ghosts;
    total.pipes += c.pipes;
    total.arrows += c.arrows;
  }
  CHECK(total.variables == 14);
  CHECK(total.ghosts == 2);
  CHECK(total.pipes == 6);
  CHECK(total.arrows == 13);

  // After the first trip the .mdl sketch is a fixpoint.
  const std::string again = XmileToMdl(MdlToXmile(back, true));
  CHECK(SketchLines(again) == SketchLines(back));
}

namespace {

// The names of the variables a variable's equations refer to, from a .mdl.
std::set<std::string> InputsOf(Model *m, const std::string &name) {
  std::set<std::string> out;
  Symbol *sym = m->GetNameSpace()->Find(name);
  if (!sym || sym->isType() != Symtype_Variable)
    return out;
  // A quoted .mdl name keeps its quotes in the parsed model.
  for (Variable *in : static_cast<Variable *>(sym)->GetInputVars()) {
    std::string n = in->GetName();
    if (n.size() >= 2 && n.front() == '"' && n.back() == '"')
      n = n.substr(1, n.size() - 2);
    out.insert(n);
  }
  return out;
}

bool Defines(Model *m, const std::string &name) {
  Symbol *sym = m->GetNameSpace()->Find(name);
  return sym && sym->isType() == Symtype_Variable && !static_cast<Variable *>(sym)->GetAllEquations().empty();
}

}  // namespace

// A model with two views, written without --sectors: each view becomes an XMILE
// module, and an input a module takes from the other is a <connect>ed shadow.
// On the way back every variable is entered under its module-qualified name
// (the .mdl namespace is flat), each module becomes a view named after it, and a
// connected input is a ghost of the variable it comes from -- which the
// equations of its module refer to directly.
TEST(SketchMdlXmile_population_resources_modules_to_mdl) {
  const std::string original = ReadFile(std::string(XMUTIL_SRC_ROOT) + kPopulationResourcesPath);
  CHECK(!original.empty());
  if (original.empty())
    return;
  const std::string xmile = MdlToXmile(original, false);
  CHECK(xmile.find("<module name=\"Population View\">") != std::string::npos);
  CHECK(xmile.find("<connect to=\"Population_View.food_adequacy\" from=\"Resource_View.food_adequacy\"/>") !=
        std::string::npos);
  const std::string back = XmileToMdl(xmile);
  CHECK(!back.empty());
  if (back.empty())
    return;

  // The equations: every variable is module-qualified, and a connected input
  // is not defined twice but referred to where it is defined.
  Model *m = roundtrip::ParseVensim(back);
  CHECK(m != nullptr);
  if (m) {
    CHECK(Defines(m, "Population View.Population"));
    CHECK(Defines(m, "Resource View.food adequacy"));
    CHECK(!Defines(m, "Population View.food adequacy"));
    CHECK(!Defines(m, "Resource View.Population"));
    CHECK(InputsOf(m, "Population View.effect food deaths").count("Resource View.food adequacy") == 1);
    CHECK(InputsOf(m, "Resource View.inidcated consumption").count("Population View.Population") == 1);
    delete m;
  }

  // The sketch: one view per module, named after it, holding what the module's
  // view held at the original coordinates.
  std::vector<Sketch> a = ParseSketches(original);
  std::vector<Sketch> b = ParseSketches(back);
  CHECK(a.size() == 2);
  CHECK(a.size() == b.size());
  if (a.size() != b.size())
    return;
  ViewCounts total;
  for (size_t i = 0; i < a.size(); i++) {
    ViewCounts c = CompareView(a[i], StripModulePrefixes(b[i]), /*compareNameSizes=*/false);
    total.variables += c.variables;
    total.ghosts += c.ghosts;
    total.arrows += c.arrows;
  }
  CHECK(total.variables == 14);
  CHECK(total.ghosts == 2);
  CHECK(total.arrows == 13);
  // The ghosts are of the qualified variables they stand for.
  CHECK(back.find("10,14,\"Resource View.food adequacy\",329,374,") != std::string::npos);
  CHECK(back.find("\"Population View.Population\",187,274,") != std::string::npos);
}

// A base-model variable fed into a module: base-model names get a leading '.',
// the module's input resolves to it, and the module's view shows a ghost of it.
// Writing the flattened model back to XMILE is refused rather than done wrong.
TEST(SketchMdlXmile_base_model_variable_feeds_a_module) {
  const char *kXmile = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model>
    <variables>
      <aux name="rate"><eqn>0.5</eqn></aux>
      <module name="growth"><connect to="growth.rate" from=".rate"/></module>
    </variables>
  </model>
  <model name="growth">
    <variables>
      <aux name="rate" access="input"><eqn>{unused when connected}</eqn></aux>
      <stock name="level"><eqn>1</eqn><inflow>gain</inflow></stock>
      <flow name="gain"><eqn>level * rate</eqn></flow>
    </variables>
    <views><view>
      <stock name="level" x="160" y="80" width="80" height="40"/>
      <flow name="gain" x="100" y="100"><pts><pt x="40" y="100"/><pt x="160" y="100"/></pts></flow>
      <aux name="rate" x="100" y="200"/>
      <connector uid="1" angle="90"><from>rate</from><to>gain</to></connector>
    </view></views>
  </model>
</xmile>
)";
  const std::string mdl = XmileToMdl(kXmile);
  CHECK(!mdl.empty());
  if (mdl.empty())
    return;
  Model *m = roundtrip::ParseVensim(mdl);
  CHECK(m != nullptr);
  if (m) {
    CHECK(Defines(m, ".rate"));
    CHECK(Defines(m, "growth.level"));
    CHECK(Defines(m, "growth.gain"));
    CHECK(!Defines(m, "growth.rate"));
    std::set<std::string> in = InputsOf(m, "growth.gain");
    CHECK(in.count(".rate") == 1 && in.count("growth.level") == 1);
    delete m;
  }
  std::vector<Sketch> views = ParseSketches(mdl);
  CHECK(views.size() == 1);
  if (views.size() == 1) {
    CHECK_EQ_STR(views[0].header[2], "*growth");
    // The module's view draws the input as `.rate` and its arrow from it. (The
    // base model has no view of its own here, so this drawing is the only one
    // and CheckGhostOwners makes it .rate's home rather than a ghost.)
    int rateUid = -1;
    for (const auto &kv : views[0].byUid)
      if (kv.second.type == 10 && kv.second.name == "\".rate\"")
        rateUid = kv.first;
    CHECK(rateUid > 0);
    bool arrow = false;
    for (const auto &kv : views[0].byUid)
      arrow = arrow || (kv.second.type == 1 && kv.second.shape == 1 && kv.second.from == rateUid);
    CHECK(arrow);
  }
  char *xmile = convert_xmile_to_xmile(kXmile, static_cast<uint32_t>(std::strlen(kXmile)), "m.xmile", -1, false);
  CHECK(xmile == nullptr);
  free(xmile);
}
