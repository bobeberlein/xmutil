// XMUtil.cpp : Defines the entry point for the console application.
//
#include "XMUtil.h"

#include <charconv>
#include <cstring>
#include <exception>

#include "Dynamo/DynamoParse.h"
#include "Model.h"
#include "Unicode.h"
#include "Vensim/VensimParse.h"

#ifdef WITH_UI
#include <QApplication>

#include "UI/Main_Window.h"
#endif

std::string StringFromDouble(double val) {
  char buf[128];
  sprintf(buf, "%g", val);
  return std::string(buf);
}

std::string ShortestDouble(double val) {
  char buf[64];
  auto res = std::to_chars(buf, buf + sizeof(buf), val);
  return std::string(buf, res.ptr);
}

std::string SpaceToUnderBar(const std::string &s) {
  std::string rval;
  for (const char *tv = s.c_str(); *tv; tv++) {
    if (*tv == ' ')
      rval.push_back('_');
    else
      rval.push_back(*tv);
  }
  return rval;
}

std::string QuotedSpaceToUnderBar(const std::string &s) {
  std::string rval;
  bool needquote = false;
  for (const char *tv = s.c_str(); *tv; tv++) {
    if (*tv == ' ')
      rval.push_back('_');
    else {
      if (*tv == '.')
        needquote = true;
      rval.push_back(*tv);
    }
  }
  if (needquote)
    rval = "\"" + rval + "\"";
  return rval;
}

bool StringMatch(const std::string &f, const std::string &s) {
  if (f.size() != s.size())
    return false;
  const char *tv1 = f.c_str();
  const char *tv2 = s.c_str();
  char c1, c2;
  for (; (c1 = *tv1); tv1++, tv2++) {
    c2 = *tv2;
    if (c1 != c2) {
      if (c1 >= 'A' && c1 <= 'Z')
        c1 += ('a' - 'A');
      if (c2 >= 'A' && c2 <= 'Z')
        c2 += ('a' - 'A');
      if (c1 != c2)
        return false;
    }
  }
  return true;
}

double AngleFromPoints(double startx, double starty, double pointx, double pointy, double endx, double endy) {
  double thetax;
  if (endx > startx)
    thetax = -atan((endy - starty) / (endx - startx)) * 180 / 3.14159265358979;
  else if (endx < startx)
    thetax = 180 - atan((starty - endy) / (startx - endx)) * 180 / 3.14159265358979;
  else if (endy > starty)
    thetax = 270;
  else
    thetax = 90;
  // straight line connector- use this if geometry fails
  if (pointx == 0 && pointy == 0)
    return thetax;

  // first take the start and end point - the center of the circle is on a line perpindicular
  // to the line between them and intersects it at its midpoint
  double line1x = (startx + endx) / 2;
  double line1y = (starty + endy) / 2;
  double slope1x, slope1y;
  if (startx == endx) {
    slope1x = 1;
    slope1y = 0;
  } else if (starty == endy) {
    slope1x = 0;
    slope1y = 1;
  } else {
    slope1x = endy - starty;  // perpindicular - flip xy
    slope1y = startx - endx;  // flip the sign
  }
  // next do point and end - most likely to have good numerics
  double line2x = (pointx + endx) / 2;
  double line2y = (pointy + endy) / 2;
  double slope2x, slope2y;
  if (pointx == endx) {
    slope2x = 1;
    slope2y = 0;
  } else if (pointy == endy) {
    slope2x = 0;
    slope2y = 1;
  } else {
    slope2x = endy - pointy;  // perpindicular - flip xy
    slope2y = pointx - endx;  // flip the sign
  }
  /* now we solve for delta1 and delta2 such that
     line1y + delta1 * slope1y = line2y + delta2 * slope2y
     line1x + delta1 * slope1x = line2x + delta2 * slope2x
     */
  double delta1, delta2;
  if (slope1y == 0) {
    if (slope2y == 0 || slope1x == 0)
      return thetax;
    delta2 = (line1y - line2y) / slope2y;
    delta1 = (line2x + delta2 * slope2x - line1x) / slope1x;
  } else if (slope1x == 0) {
    if (slope2x == 0)
      return thetax;
    delta2 = (line1x - line2x) / slope2x;
    delta1 = (line2y + delta2 * slope2y - line1y) / slope1y;
  } else if (slope2y == 0) {
    if (slope2x == 0)
      return thetax;
    delta1 = (line2y - line1y) / slope1y;
    delta2 = (line1x + delta1 * slope1x - line2x) / slope2x;
  } else {
    /* now we solve for delta1 and delta2 such that
    line1y + delta1 * slope1y = line2y + delta2 * slope2y
       -> delta1 = (line2y + delta2 * slope2y - line1y)/slope1y
    line1x + delta1 * slope1x = line2x + delta2 * slope2x
       -> line1x + (line2y + delta2 * slope2y - line1y)/slope1y * slope1x = line2x + delta2 * slope2x
       -> line1x + (line2y - line1y)/slope1y * slope1x - line2x =  delta2 * (slope2x - slope1x*slope2y/slope1y)
       ->
    */
    if (abs(slope2x - slope1x * slope2y / slope1y) < 1e-8)
      return thetax;
    delta2 = (line1x + (line2y - line1y) / slope1y * slope1x - line2x) / (slope2x - slope1x * slope2y / slope1y);
    delta1 = (line2y + delta2 * slope2y - line1y) / slope1y;
  }
  double centerx = line1x + delta1 * slope1x;
  double centery = line1y + delta1 * slope1y;
  assert(line2x + delta2 * slope2x - centerx < 1e-8);
  assert(line2y + delta2 * slope2y - centery < 1e-8);
  // arc tan of slope perpeindicular to center start line
  if (abs(centery - starty) < 1e-6) {
    if (pointy > starty)
      return 90;
    return 270;
  }
  if (abs(centerx - startx) < 1e-6) {
    if (pointx > startx)
      return 0;
    return 180;
  }
  thetax = atan2(-(starty - centery), (startx - centerx)) * 180 / 3.14159265358979;
  // this needs to go through the point - so add or subtract 90 to get o
  // find the angle closest to the angle from start to point

  double direct = atan2(-(pointy - starty), (pointx - startx)) * 180 / 3.14159265358979;
  double diff1 = direct - (thetax - 90);
  while (diff1 < 0)
    diff1 += 360;
  while (diff1 > 180)
    diff1 -= 360;
  double diff2 = direct - (thetax + 90);
  while (diff2 < 0)
    diff2 += 360;
  while (diff2 > 180)
    diff2 -= 360;
  if (abs(diff1) < abs(diff2))
    thetax -= 90;
  else
    thetax += 90;
  return thetax;

  if (abs(pointx - startx) > abs(pointy - starty)) {
    if (pointx < startx) {
      // need to end up in quadrant 2 or 3
      if (thetax >= 0)  // in 1 or 2
        thetax += 90;
      else  // in 3 or 4
        thetax -= 90;
    } else  // need to end up in quadrant 1 or 4
    {
      if (thetax >= 0)  // in 1 or 2
        thetax -= 90;
      else  // in 3 or 4
        thetax += 90;
    }
  } else {
    if (pointy < starty) {
      // need to end up in quadrant 3 or 4
      if (thetax >= 0)  // in 1 or 2
      {
        if (thetax < 90)  // 1
          thetax -= 90;
        else
          thetax += 90;
      } else  // in 3 or 4
      {
        if (thetax < -90)
          thetax -= 90;
        else
          thetax += 90;
      }
    } else  // need to end up in quadrant 1 or 2
    {
      if (thetax >= 0)  // in 1 or 2
      {
        if (thetax < 90)  // 1
          thetax += 90;
        else
          thetax -= 90;
      } else  // in 3 or 4
      {
        if (thetax < -90)
          thetax += 90;
        else
          thetax -= 90;
      }
    }
  }
  return thetax;

  // below is wrong - we need to triangulate to get the center then pull out the tangent at the start point

  // case point between start and end
  double a2 = (pointx - startx) * (pointx - startx) + (pointy - starty) * (pointy - starty);
  double b2 = (pointx - endx) * (pointx - endx) + (pointy - endy) * (pointy - endy);
  double c2 = (startx - endx) * (startx - endx) + (starty - endy) * (starty - endy);
  double x = (c2 + (a2 - b2)) / (2 * sqrt(c2));
  double y2 = a2 - x * x;
  double theta = atan(sqrt(y2) / x);
  if (!std::isnan(theta))
    return theta * 180 / 3.141592676;
  theta = atan((endy - starty) / (endx - startx));
  if (!std::isnan(theta))
    return theta * 180 / 3.141592676;
  if (endy < starty)
    return 90;
  return 270;
  return 33;
}

namespace {

// File-extension dispatch for the C entry points: case-insensitive match
// anchored at the last '.'. The old length-based slice (strlen > 5, last
// three bytes) misrouted the shortest legal names like "x.dyn" to the
// Vensim parser and missed mixed-case extensions entirely.
bool HasExtension(const char *fileName, const char *ext) {
  const char *dot = strrchr(fileName, '.');
  return dot != nullptr && StringMatch(dot + 1, ext);
}

// The engine reports some failures by throwing (string literals out of
// Variable::AddEq and the subscript machinery, plus std::bad_alloc from any
// allocation). An exception unwinding across the extern "C" boundary is
// undefined behavior, so every conversion entry point catches near the
// boundary and funnels through here (called from a catch block) to log the
// reason and turn it into the documented NULL return.
char *LogConversionFailure() {
  try {
    throw;
  } catch (const char *msg) {
    log("error: %s\n", msg);
  } catch (const std::exception &e) {
    log("error: %s\n", e.what());
  } catch (...) {
    log("error: unexpected exception during conversion\n");
  }
  return nullptr;
}

void LogAll(const std::vector<std::string> &msgs) {
  for (const std::string &m : msgs) {
    log("%s\n", m.c_str());
  }
}

// Parse native (Vensim or Dynamo) source into m, dispatching on the .dyn
// extension, and report the parser's sketch scale ratios. isLongName keeps its
// historical per-parser interpretation -- any non-zero value for Dynamo, but
// exactly 1 for Vensim -- because both C entry points have always behaved that
// way and it is part of the extern "C" contract.
bool ParseNativeInput(Model &m, const char *source, uint32_t len, const char *fileName, int isLongName,
                      bool isAsSectors, double &xscale, double &yscale) {
  if (HasExtension(fileName, "dyn")) {
    DynamoParse dp{&m};
    dp.SetLongName(isLongName != 0);
    m.SetAsSectors(isAsSectors);
    if (!dp.ProcessFile(fileName, source, len)) {
      return false;
    }
    xscale = dp.Xratio();
    yscale = dp.Yratio();
  } else {
    VensimParse vp{&m};
    vp.SetLongName(isLongName == 1);
    m.SetAsSectors(isAsSectors);
    if (!vp.ProcessFile(fileName, source, len)) {
      return false;
    }
    xscale = vp.Xratio();
    yscale = vp.Yratio();
  }
  return true;
}

// Parse XMILE source into m, logging any diagnostics. A successful parse can
// still leave advisory diagnostics in errs (e.g. <gf type="discrete"> mapped
// to continuous); they are surfaced here and not carried forward, so the
// post-Print checks in FinishXmile/FinishMdl only see genuine serialization
// errors. There is no sectors argument to pass along: Model::ParseXMILE marks
// the Model as XMILE-sourced and PrintXMILE emits those as sectors outright.
bool ParseXmileInput(Model &m, const char *source, uint32_t len, const char *fileName) {
  std::vector<std::string> errs;
  bool ok = m.ParseXMILE(fileName, source, len, errs);
  LogAll(errs);
  return ok;
}

// Serialization epilogues: print the model, and on any reported error log and
// return NULL; otherwise return the strdup'd text the caller now owns.
// TODO: expose errs through the C API instead of only logging them.
char *FinishXmile(Model &m, bool isCompact, double xscale, double yscale) {
  std::vector<std::string> errs;
  std::string xmile = m.PrintXMILE(isCompact, errs, xscale, yscale);
  if (!errs.empty()) {
    LogAll(errs);
    return nullptr;
  }
  return strdup(xmile.c_str());
}

char *FinishMdl(Model &m) {
  std::vector<std::string> errs;
  std::string mdl = m.PrintMDL(errs);
  if (!errs.empty()) {
    LogAll(errs);
    return nullptr;
  }
  return strdup(mdl.c_str());
}

}  // namespace

extern "C" {
// returns NULL on error or a string containing XMILE that the caller now owns
char *convert_mdl_to_xmile(const char *mdlSource, uint32_t mdlSourceLen, const char *fileName, bool isCompact,
                           int isLongName, bool isAsSectors) try {
  Model m{};
  if (fileName == nullptr) {
    fileName = "<in memory>";
  }

  double xscale = 1.0;
  double yscale = 1.0;
  if (!ParseNativeInput(m, mdlSource, mdlSourceLen, fileName, isLongName, isAsSectors, xscale, yscale)) {
    return nullptr;
  }

  m.RunPostParsePipeline();

  // if there is a view then try to make sure everything is defined in
  // the views put unknowns in a heap in the first view at 20,20 but
  // for things that have connections try to put them in the right
  // place
  bool want_complete = false;  // could pass this as an option - but let the reader handle this stuff
  if (want_complete) {
    m.AttachStragglers();
  }

  return FinishXmile(m, isCompact, xscale, yscale);
} catch (...) {
  return LogConversionFailure();
}

// returns NULL on error or a string containing XMILE that the caller now owns
char *convert_xmile_to_xmile(const char *source, uint32_t len, const char *fileName, int isLongName,
                             bool isAsSectors) try {
  Model m{};
  if (fileName == nullptr) {
    fileName = "<in memory>";
  }
  // Neither trailing option is consumed on the XMILE path; both are accepted
  // for API symmetry with convert_mdl_to_xmile so embedders can use a
  // consistent call shape.
  //
  // isLongName: v1 of the XMILE reader has no use for it -- XMILE has no
  // long/short name distinction at the envelope level and PrintXMILE reads
  // naming state from the populated Model.
  //
  // isAsSectors: the sector form is the only form an XMILE-sourced model is
  // emitted in. What `false` selects is the module decomposition, which emits
  // one <model> per group or per view -- a document this project's own reader
  // rejects ("multiple <model> elements are not supported"), so honoring it
  // would mean normalizing XMILE into XMILE nothing here can read back. See
  // Model::PrintXMILE.
  (void)isLongName;
  (void)isAsSectors;

  if (!ParseXmileInput(m, source, len, fileName)) {
    return nullptr;
  }

  m.RunPostParsePipeline();

  // XMILE input does not carry the Vensim sketch x/y scaling factors; emit at
  // unit scale. Embedders that need scale awareness can layer it on top.
  return FinishXmile(m, /*isCompact=*/false, /*xscale=*/1.0, /*yscale=*/1.0);
} catch (...) {
  return LogConversionFailure();
}

// returns NULL on error or a string containing Vensim .mdl that the caller now owns
char *convert_xmile_to_mdl(const char *source, uint32_t len, const char *fileName, int isLongName) try {
  Model m{};
  if (fileName == nullptr) {
    fileName = "<in memory>";
  }
  (void)isLongName;  // see convert_xmile_to_xmile

  if (!ParseXmileInput(m, source, len, fileName)) {
    return nullptr;
  }

  m.RunPostParsePipeline();

  return FinishMdl(m);
} catch (...) {
  return LogConversionFailure();
}

// returns NULL on error or a string containing Vensim .mdl that the caller now owns
char *convert_to_mdl(const char *mdlSource, uint32_t mdlSourceLen, const char *fileName, int isLongName) try {
  Model m{};
  if (fileName == nullptr) {
    fileName = "<in memory>";
  }

  // .mdl output carries no sketch scaling, so the parsed ratios are unused.
  double xscale = 1.0;
  double yscale = 1.0;
  if (!ParseNativeInput(m, mdlSource, mdlSourceLen, fileName, isLongName, /*isAsSectors=*/false, xscale, yscale)) {
    return nullptr;
  }

  m.RunPostParsePipeline();

  return FinishMdl(m);
} catch (...) {
  return LogConversionFailure();
}
}  // extern "C"
