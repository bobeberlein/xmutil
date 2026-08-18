#include "RoundTrip.h"

#include "../../src/Model.h"
#include "../../src/Vensim/VensimParse.h"
#include "../mdl/ModelComparator.h"

namespace xmileroundtrip {

Model *ParseXMILE(const std::string &text, std::vector<std::string> &errs) {
  Model *m = new Model();
  if (!m->ParseXMILE("<test>", text.c_str(), text.size(), errs)) {
    delete m;
    return nullptr;
  }
  // A successful parse can leave advisory diagnostics in errs (dropped
  // <non_negative> clamps, arity notes, discrete <gf>, ...). They are preserved
  // here so tests that assert on advisory text can inspect them; the round-trip
  // helpers below clear errs before the Print phase, mirroring the extern-C
  // drivers in XMUtil.cpp that log-and-clear parse advisories before serializing.
  m->RunPostParsePipeline();
  return m;
}

std::vector<std::string> RoundTripDiffs(const std::string &xmileText) {
  std::vector<std::string> errs;
  Model *m0 = ParseXMILE(xmileText, errs);
  if (!m0)
    return {errs.empty() ? std::string("failed to parse input XMILE")
                         : ("failed to parse input XMILE: " + errs.front())};

  // Non-fatal parse advisories must not be mistaken for serialization errors by
  // the post-Print emptiness check below (mirrors XMUtil.cpp's log-and-clear).
  errs.clear();
  double xscale = 1.0;
  double yscale = 1.0;
  std::string regen = m0->PrintXMILE(/*isCompact=*/false, errs, xscale, yscale);
  if (!errs.empty()) {
    delete m0;
    return {"PrintXMILE reported errors: " + errs.front()};
  }

  std::vector<std::string> reparseErrs;
  Model *m1 = ParseXMILE(regen, reparseErrs);
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

std::string NormalizeXMILE(const std::string &xmileText, std::vector<std::string> &errs) {
  Model *m = ParseXMILE(xmileText, errs);
  if (!m) {
    if (errs.empty())
      errs.push_back("failed to parse XMILE");
    return std::string();
  }
  // See RoundTripDiffs: drop non-fatal parse advisories before the Print check.
  errs.clear();
  std::string out = m->PrintXMILE(/*isCompact=*/false, errs, /*xscale=*/1.0, /*yscale=*/1.0);
  delete m;
  if (!errs.empty())
    return std::string();
  if (out.empty())
    errs.push_back("PrintXMILE produced an empty document");
  return out;
}

std::string FixpointFailure(const std::string &xmileText) {
  std::vector<std::string> errs;
  const std::string once = NormalizeXMILE(xmileText, errs);
  if (once.empty())
    return "first normalization failed: " + (errs.empty() ? std::string("(no diagnostic)") : errs.front());

  errs.clear();
  const std::string twice = NormalizeXMILE(once, errs);
  if (twice.empty())
    return "second normalization failed: " + (errs.empty() ? std::string("(no diagnostic)") : errs.front());

  if (once == twice)
    return std::string();

  // Report the first differing line from each side rather than the whole
  // document: the failures this guards against (an extra paren layer, a
  // permuted variable order) are legible from one line, and dumping two full
  // models per failing fixture buries them.
  size_t a = 0;
  size_t b = 0;
  int line = 1;
  while (a < once.size() && b < twice.size()) {
    const size_t ae = once.find('\n', a);
    const size_t be = twice.find('\n', b);
    const std::string al = once.substr(a, ae == std::string::npos ? std::string::npos : ae - a);
    const std::string bl = twice.substr(b, be == std::string::npos ? std::string::npos : be - b);
    if (al != bl)
      return "pass 1 and pass 2 differ at line " + std::to_string(line) + ":\n    pass1: " + al + "\n    pass2: " + bl;
    if (ae == std::string::npos || be == std::string::npos)
      break;
    a = ae + 1;
    b = be + 1;
    line++;
  }
  return "pass 1 and pass 2 differ in length (" + std::to_string(once.size()) + " vs " + std::to_string(twice.size()) +
         " bytes) with no differing line before the end";
}

std::vector<std::string> XmileToMdlDiffs(const std::string &xmileText) {
  std::vector<std::string> errs;
  Model *m0 = ParseXMILE(xmileText, errs);
  if (!m0)
    return {errs.empty() ? std::string("failed to parse input XMILE")
                         : ("failed to parse input XMILE: " + errs.front())};

  // See RoundTripDiffs: drop non-fatal parse advisories before the Print check.
  errs.clear();
  std::string regen = m0->PrintMDL(errs);
  if (!errs.empty()) {
    delete m0;
    return {"PrintMDL reported errors: " + errs.front()};
  }

  // The emitted text is Vensim .mdl, so the re-parse goes through the Vensim
  // reader -- the same one a downstream tool would use on the writer's output.
  Model *m1 = new Model();
  VensimParse vp{m1};
  if (!vp.ProcessFile("<test>", regen.c_str(), regen.size())) {
    delete m0;
    delete m1;
    return {"failed to re-parse generated .mdl"};
  }
  m1->RunPostParsePipeline();

  std::vector<std::string> diffs = ModelComparator::Compare(m0, m1);
  delete m0;
  delete m1;
  return diffs;
}

}  // namespace xmileroundtrip
