// Byte-identity check: MDL -> XMILE writer output is stable (Phase 8, Task 4).
//
// Pins down xmile-reader.AC6.2: the existing convert_mdl_to_xmile code path
// must produce byte-identical XMILE for a representative MDL fixture before
// and after the XMILE reader landed. The new reader code paths
// (XmileReader, Model::ParseXMILE, convert_xmile_to_xmile, convert_xmile_to_mdl)
// are orthogonal to the writer; this test makes any unintended side effect on
// the writer's output -- e.g., a change in function-table registration order
// from Phase 4's refactor that leaked through into Print sequencing --
// surface as a deterministic failure rather than as a subtle corpus drift.
//
// If a future writer change is intentional, regenerate the golden by running
// XMUtil on test/fixtures/simlin/teacup.mdl and
// moving the resulting teacup.xmile into test/xmile/fixtures/teacup.golden.xmile,
// then commit the new golden alongside the writer change.

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

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

// XMUTIL_SRC_ROOT is the repo root, injected via -D in XMUtil.gyp so the test
// binary can locate fixtures regardless of its CWD. The leading slash here
// joins it as a path; the format mirrors CorpusRoundTripTest's convention.
const char *kTeacupMdlPath = "/test/fixtures/simlin/teacup.mdl";
const char *kTeacupXmileGoldenPath = "/test/xmile/fixtures/teacup.golden.xmile";

}  // namespace

// AC6.2: convert_mdl_to_xmile(teacup.mdl) byte-identical to the committed
// golden. The call uses the same default-CLI flag values that produced the
// golden (isCompact=false, isLongName=-1, isAsSectors=false) -- any change
// here must be matched by regenerating the golden, otherwise the byte-identity
// guarantee is unverifiable.
TEST(MdlXmileByteIdentity_teacup) {
  std::string mdl = ReadFile(std::string(XMUTIL_SRC_ROOT) + kTeacupMdlPath);
  std::string golden = ReadFile(std::string(XMUTIL_SRC_ROOT) + kTeacupXmileGoldenPath);
  CHECK(!mdl.empty());
  CHECK(!golden.empty());
  if (mdl.empty() || golden.empty())
    return;

  char *out = convert_mdl_to_xmile(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "teacup.mdl",
                                   /*isCompact=*/false, /*isLongName=*/-1, /*isAsSectors=*/false);
  CHECK(out != nullptr);
  if (!out)
    return;

  std::string actual(out);
  free(out);

  CHECK_EQ_STR(actual, golden);
}
