#include "RoundTrip.h"

#include "../../src/Dynamo/DynamoParse.h"
#include "../../src/Model.h"
#include "../../src/Vensim/VensimParse.h"
#include "../TestHarness.h"
#include "ModelComparator.h"

namespace roundtrip {

Model *ParseVensim(const std::string &text) {
  Model *m = new Model();
  VensimParse vp{m};
  if (!vp.ProcessFile("<test>", text.c_str(), text.size())) {
    delete m;
    return nullptr;
  }
  m->RunPostParsePipeline();
  return m;
}

Model *ParseDynamo(const std::string &text) {
  Model *m = new Model();
  DynamoParse dp{m};
  if (!dp.ProcessFile("<test>", text.c_str(), text.size())) {
    delete m;
    return nullptr;
  }
  m->RunPostParsePipeline();
  return m;
}

std::vector<std::string> RoundTripDiffs(const std::string &mdlText) {
  Model *m0 = ParseVensim(mdlText);
  if (!m0)
    return {"failed to parse input .mdl"};

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  if (!errs.empty()) {
    delete m0;
    return {"PrintMDL reported errors: " + errs.front()};
  }

  Model *m1 = ParseVensim(regen);
  if (!m1) {
    delete m0;
    return {"failed to re-parse generated .mdl"};
  }

  std::vector<std::string> diffs = ModelComparator::Compare(m0, m1);
  delete m0;
  delete m1;
  return diffs;
}

void ExpectCleanRoundTrip(const std::string &mdl) {
  std::vector<std::string> diffs = RoundTripDiffs(mdl);
  for (const std::string &d : diffs)
    printf("  round-trip diff: %s\n", d.c_str());
  CHECK(diffs.empty());
}

std::vector<std::string> DynamoToMdlDiffs(const std::string &dynText) {
  Model *m0 = ParseDynamo(dynText);
  if (!m0)
    return {"failed to parse input .dyn"};

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  if (!errs.empty()) {
    delete m0;
    return {"PrintMDL reported errors: " + errs.front()};
  }

  // The emitted text is Vensim .mdl regardless of the Dynamo source, so the
  // re-parse uses the Vensim reader -- the same one a downstream tool would use
  // on the writer's output.
  Model *m1 = ParseVensim(regen);
  if (!m1) {
    delete m0;
    return {"failed to re-parse generated .mdl"};
  }

  std::vector<std::string> diffs = ModelComparator::Compare(m0, m1);
  delete m0;
  delete m1;
  return diffs;
}

}  // namespace roundtrip
