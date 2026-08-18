// The equation parser's bison action shims see the active reader only through
// the process-global XPObject (mirroring VensimParse's VPObject), so exactly one
// reader may be live at a time.
//
// That invariant was carried by an `assert` -- which compiles out under NDEBUG,
// the build an embedder parsing two documents from nested scopes would actually
// be running -- and by a destructor that cleared the global unconditionally. The
// pair meant a second reader silently stole XPObject and then, on the way out,
// left it null: every later shim call in the still-live OUTER reader
// (xpyy_resolve_symbol, xpyylex, PushErr) took its defensive null branch and
// answered with a 0-placeholder, so the parse "succeeded" and produced a
// different model, with no diagnostic anywhere.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Xmile/XmileReader.h"
#include "../TestHarness.h"

namespace {

const char *const kDoc = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="a"><eqn>1</eqn></aux>
    <aux name="b"><eqn>a * 2</eqn></aux>
  </variables></model>
</xmile>
)";

}  // namespace

// Explicit at runtime, in every build -- not an assert that evaporates.
TEST(ReaderLifetime_second_concurrent_reader_is_refused) {
  Model outerModel;
  XmileReader outer{&outerModel};
  bool threw = false;
  {
    Model innerModel;
    try {
      XmileReader inner{&innerModel};
      (void)inner;
    } catch (...) {
      threw = true;
    }
  }
  CHECK(threw);
}

// The consequence that made the missing check dangerous rather than merely
// untidy: whatever the second reader did, the first must still be able to
// resolve names when control returns to it.
TEST(ReaderLifetime_outer_reader_survives_a_refused_inner_one) {
  Model outerModel;
  XmileReader outer{&outerModel};
  {
    Model innerModel;
    try {
      XmileReader inner{&innerModel};
      (void)inner;
    } catch (...) {
    }
  }

  std::vector<std::string> errs;
  const bool ok = outer.ProcessFile("<test>", kDoc, std::string(kDoc).size(), errs);
  for (const std::string &e : errs)
    printf("  err: %s\n", e.c_str());
  CHECK(ok);
  outerModel.RunPostParsePipeline();

  // `b`'s equation has to reference `a` rather than the 0-placeholder a null
  // XPObject hands back.
  std::vector<std::string> mdlErrs;
  const std::string mdl = outerModel.PrintMDL(mdlErrs);
  CHECK(mdl.find("b = a * 2") != std::string::npos);
}
