// XMILE C-entry contract checks (Phase 8, Task 3).
//
// Mirrors test/mdl/CEntryTest.cpp for the XMILE-input entry points
// convert_xmile_to_xmile and convert_xmile_to_mdl. Pins down the ownership
// contract (xmile-reader.AC1.2, AC1.3): the returned buffer is strdup'd, so
// callers free() it (NOT delete), and a NULL fileName argument is accepted
// (mirrors convert_mdl_to_xmile, where Main.cpp and external callers may pass
// NULL for stdin input).

#include <cstdlib>
#include <cstring>
#include <string>

#include "../../src/XMUtil.h"
#include "../TestHarness.h"

namespace {

// Minimal but real XMILE: one aux. Small enough that the success assertions
// (non-empty output that starts with the expected marker) are the load-bearing
// checks, not a model-content reassertion -- the corpus tests cover content.
const char *const kMinimal =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\">\n"
    "  <sim_specs method=\"euler\"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>\n"
    "  <model><variables><aux name=\"x\"><eqn>1</eqn></aux></variables></model>\n"
    "</xmile>\n";

}  // namespace

// AC1.2 (success): convert_xmile_to_xmile on a valid model returns a non-null,
// caller-owned string that begins with the <xmile root element. (XMILEGenerator
// does not emit a leading <?xml...?> declaration -- the writer goes straight
// into the document root.) free() (not delete) releases it -- the contract
// this test pins down for callers.
TEST(CEntry_xmile_to_xmile_returns_owned_buffer) {
  char *out = convert_xmile_to_xmile(kMinimal, static_cast<uint32_t>(strlen(kMinimal)), "test.xmile",
                                     /*isLongName=*/-1, /*isAsSectors=*/false);
  CHECK(out != nullptr);
  if (!out)
    return;

  std::string s(out);
  CHECK(!s.empty());
  // Starts with the XMILE root (rfind at offset 0 == 0 means "starts with").
  CHECK(s.rfind("<xmile", 0) == 0);

  // strdup'd buffer -- release with free, not delete.
  free(out);
}

// AC1.3 (success): convert_xmile_to_mdl on a valid model returns a non-null,
// caller-owned string that begins with the {UTF-8} marker.
TEST(CEntry_xmile_to_mdl_returns_owned_buffer) {
  char *out = convert_xmile_to_mdl(kMinimal, static_cast<uint32_t>(strlen(kMinimal)), "test.xmile",
                                   /*isLongName=*/-1);
  CHECK(out != nullptr);
  if (!out)
    return;

  std::string s(out);
  CHECK(!s.empty());
  CHECK(s.rfind("{UTF-8}", 0) == 0);

  free(out);
}

// Soft reader diagnostics must not fail the conversion. ParseXMILE pushes
// advisory messages (e.g. <gf type="discrete"> mapped to continuous) while
// still returning success; a conversion that then treats any leftover
// message as fatal rejects valid, common Stella exports outright.
TEST(CEntry_soft_warning_does_not_fail_conversion) {
  const char *kDiscreteGf =
      "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
      "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\">\n"
      "  <sim_specs method=\"euler\"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>\n"
      "  <model><variables>\n"
      "    <aux name=\"steps\">\n"
      "      <gf type=\"discrete\"><xscale min=\"0\" max=\"2\"/><ypts>0,1,2</ypts></gf>\n"
      "    </aux>\n"
      "  </variables></model>\n"
      "</xmile>\n";

  char *xmile = convert_xmile_to_xmile(kDiscreteGf, static_cast<uint32_t>(strlen(kDiscreteGf)), "discrete.xmile",
                                       /*isLongName=*/-1, /*isAsSectors=*/false);
  CHECK(xmile != nullptr);
  if (xmile)
    free(xmile);

  char *mdl = convert_xmile_to_mdl(kDiscreteGf, static_cast<uint32_t>(strlen(kDiscreteGf)), "discrete.xmile",
                                   /*isLongName=*/-1);
  CHECK(mdl != nullptr);
  if (mdl)
    free(mdl);
}

// AC1.2 (NULL-fileName contract): both XMILE C entries accept NULL for fileName
// (mirrors convert_mdl_to_xmile, which substitutes a default "<in memory>"-style
// placeholder so error messages remain attributable). A future change that
// started dereferencing fileName unconditionally would crash this test rather
// than silently break stdin callers.
TEST(CEntry_null_filename_is_accepted) {
  char *out = convert_xmile_to_xmile(kMinimal, static_cast<uint32_t>(strlen(kMinimal)), /*fileName=*/nullptr,
                                     /*isLongName=*/-1, /*isAsSectors=*/false);
  CHECK(out != nullptr);
  if (out)
    free(out);
}
