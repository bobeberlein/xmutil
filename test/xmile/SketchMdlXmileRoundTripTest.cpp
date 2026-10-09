// The sketch survives .mdl -> XMILE -> .mdl.
//
// XMILE has no place for most of a Vensim sketch record (UIDs, the shape and
// bits words, the font line, valve and cloud records, a name's text extent, the
// point an arc was dragged through), so the trip back rebuilds those from
// Vensim's defaults and from XMILE's standard geometry. What has to survive is
// the drawing: every element where it was, stocks at their own size, flows with
// their valve, name and pipes, clouds at the pipe ends, and every arrow leaving
// its source at the same angle. The checks below hold the regenerated sketch to
// that, record by record, against the original Vensim file.
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

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

// Read from upstream's test_models/ directory, like the C-LEARN stress test.
const char *kSimplePopulationPath = "/test_models/SimplePopulation.mdl";

std::string MdlToXmile(const std::string &mdl) {
  char *out = convert_mdl_to_xmile(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "model.mdl", false, -1, false);
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

// The sketch section: the lines from the opener up to (not including) the
// terminator, without line endings.
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

std::vector<std::string> Fields(const std::string &line) {
  std::vector<std::string> f;
  std::string cur;
  for (char c : line) {
    if (c == ',') {
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

struct Sketch {
  std::vector<std::string> header;  // opener, version, *title, font line
  std::map<int, Record> byUid;
};

Sketch ParseSketch(const std::string &mdl) {
  Sketch s;
  std::vector<std::string> lines = SketchLines(mdl);
  for (size_t i = 0; i < lines.size(); i++) {
    if (i < 4) {
      s.header.push_back(lines[i]);
      continue;
    }
    std::vector<std::string> f = Fields(lines[i]);
    if (f.size() < 7)
      continue;
    Record r;
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
    s.byUid[r.uid] = r;
  }
  return s;
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
// its flow -- the record after it) by name, or a cloud by position.
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

TEST(SketchMdlXmile_simple_population_round_trips) {
  const std::string original = ReadFile(std::string(XMUTIL_SRC_ROOT) + kSimplePopulationPath);
  CHECK(!original.empty());
  if (original.empty()) {
    printf("  fixture missing: %s\n", kSimplePopulationPath);
    return;
  }
  const std::string xmile = MdlToXmile(original);
  CHECK(!xmile.empty());
  const std::string back = XmileToMdl(xmile);
  CHECK(!back.empty());
  if (xmile.empty() || back.empty())
    return;

  const Sketch a = ParseSketch(original);
  const Sketch b = ParseSketch(back);

  // The frame header comes back verbatim: opener, version, title, font line.
  CHECK(a.header.size() == 4 && b.header.size() == 4);
  for (size_t i = 0; i < a.header.size() && i < b.header.size(); i++)
    CHECK_EQ_STR(b.header[i], a.header[i]);

  // Every variable record comes back at the same place, with the same shape,
  // bits and name placement. A stock keeps its size (XMILE carries it); a
  // name's text extent is re-estimated, so it only has to be close.
  std::map<std::string, const Record *> va = Variables(a);
  std::map<std::string, const Record *> vb = Variables(b);
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
    if (ra.x != rb.x || ra.y != rb.y)
      printf("  %s moved: (%d,%d) -> (%d,%d)\n", kv.first.c_str(), ra.x, ra.y, rb.x, rb.y);
    CHECK(ra.x == rb.x && ra.y == rb.y);
    CHECK(ra.shape == rb.shape);
    CHECK(ra.bits == rb.bits);
    CHECK(ra.tpos == rb.tpos);
    if (ra.shape == 3) {
      CHECK(ra.w == rb.w && ra.h == rb.h);
    } else {
      CHECK(std::abs(ra.w - rb.w) <= 4);
      CHECK(ra.h == rb.h);
    }
  }

  // Each flow's valve comes back exactly.
  for (const auto &kv : va) {
    const Record *valveA = ValveOf(a, kv.first);
    if (!valveA)
      continue;
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
  CHECK(pa.size() == 4);
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
  CHECK(aa.size() == 4);
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

  // Nothing is left over: the regenerated sketch has exactly these records.
  size_t valves = 0;
  for (const auto &kv : a.byUid)
    if (kv.second.type == 11)
      valves++;
  CHECK(b.byUid.size() == va.size() + valves + cloudsA.size() + pa.size() + aa.size());

  // After the first trip the .mdl sketch is a fixpoint: going around again
  // reproduces it exactly.
  const std::string again = XmileToMdl(MdlToXmile(back));
  CHECK(SketchLines(again) == SketchLines(back));
}
