// Tests for the CLI's output-path derivation (src/OutputPath.cpp).
//
// The interesting behavior is the clobber guard: a conversion whose output
// extension matches its input's must not truncate the model it is reading, so
// the derived name gets ".regen" interposed. The test that matters is the one
// the string comparison this code used to do gets wrong -- an input named
// "Model.XMILE" derives the output "Model.xmile", which is a different string
// but the same file on a case-insensitive volume. The derivation policy is
// exercised through an injected FileIdentityFn (a case-insensitive filesystem
// cannot be conjured on Linux), and the real QueryFileIdentity is exercised
// against a hard link, which is the same "two names, one file" situation.

#include <cstdio>
#include <string>
#include <vector>

#include "../src/OutputPath.h"
#include "../src/XMUtil.h"
#include "TestHarness.h"

#ifndef WIN32
#include <stdlib.h>
#include <unistd.h>
#endif

namespace {

// Stands in for a case-sensitive volume: distinct path strings are distinct
// files, and nothing else is on disk.
FileIdentity AlwaysDifferent(const std::string &, const std::string &) {
  return FileIdentity::Different;
}

// Stands in for a case-insensitive volume: paths that differ only in ASCII case
// name one file. StringMatch is the engine's ASCII case-insensitive compare.
FileIdentity CaseInsensitiveVolume(const std::string &candidate, const std::string &input) {
  return StringMatch(candidate, input) ? FileIdentity::Same : FileIdentity::Different;
}

FileIdentity AlwaysUnknown(const std::string &, const std::string &) {
  return FileIdentity::Unknown;
}

}  // namespace

TEST(OutputPath_replace_final_extension) {
  CHECK_EQ_STR(ReplaceFinalExtension("foo.mdl", "xmile"), "foo.xmile");
  CHECK_EQ_STR(ReplaceFinalExtension("foo", "xmile"), "foo.xmile");
  CHECK_EQ_STR(ReplaceFinalExtension("/a/b/foo.stmx", "mdl"), "/a/b/foo.mdl");
  CHECK_EQ_STR(ReplaceFinalExtension("foo.MDL", "regen.mdl"), "foo.regen.mdl");
  CHECK_EQ_STR(ReplaceFinalExtension("a.b.c", "xmile"), "a.b.xmile");
}

// A period in a directory name, or a filename that is entirely dots, must not
// be mistaken for the extension -- getting this wrong writes to a path outside
// the directory the user named.
TEST(OutputPath_replace_final_extension_edge_cases) {
  CHECK_EQ_STR(ReplaceFinalExtension("/a.b/model", "xmile"), "/a.b/model.xmile");
  CHECK_EQ_STR(ReplaceFinalExtension("/a.b/model.mdl", "xmile"), "/a.b/model.xmile");
  CHECK_EQ_STR(ReplaceFinalExtension(".", "xmile"), "..xmile");
  CHECK_EQ_STR(ReplaceFinalExtension("..", "xmile"), "...xmile");
  // A dotfile is a name, not an extension.
  CHECK_EQ_STR(ReplaceFinalExtension(".mdl", "xmile"), ".mdl.xmile");
  CHECK_EQ_STR(ReplaceFinalExtension("/a/.mdl", "xmile"), "/a/.mdl.xmile");
}

// The documented CLI conversions, with nothing on disk to collide with.
TEST(OutputPath_ordinary_conversions) {
  CHECK_EQ_STR(DeriveOutputPath("foo.mdl", "xmile", AlwaysDifferent), "foo.xmile");
  CHECK_EQ_STR(DeriveOutputPath("foo.stmx", "xmile", AlwaysDifferent), "foo.xmile");
  CHECK_EQ_STR(DeriveOutputPath("foo.dyn", "mdl", AlwaysDifferent), "foo.mdl");
  // Same name in and out: caught without consulting the filesystem at all.
  CHECK_EQ_STR(DeriveOutputPath("foo.mdl", "mdl", AlwaysDifferent), "foo.regen.mdl");
  CHECK_EQ_STR(DeriveOutputPath("foo.xmile", "xmile", AlwaysDifferent), "foo.regen.xmile");
  CHECK_EQ_STR(DeriveOutputPath("/a/b/foo.xmile", "xmile", AlwaysDifferent), "/a/b/foo.regen.xmile");
}

// The defect: an extension that differs only in case is the same file on macOS
// and Windows, so the output must be renamed rather than written over the input.
TEST(OutputPath_case_insensitive_volume_renames) {
  CHECK_EQ_STR(DeriveOutputPath("Model.XMILE", "xmile", CaseInsensitiveVolume), "Model.regen.xmile");
  CHECK_EQ_STR(DeriveOutputPath("Model.MDL", "mdl", CaseInsensitiveVolume), "Model.regen.mdl");
  CHECK_EQ_STR(DeriveOutputPath("/a/b/Model.Xmile", "xmile", CaseInsensitiveVolume), "/a/b/Model.regen.xmile");
  // A .STMX input still derives a plainly different filename, so no rename.
  CHECK_EQ_STR(DeriveOutputPath("Model.STMX", "xmile", CaseInsensitiveVolume), "Model.xmile");
  CHECK_EQ_STR(DeriveOutputPath("Model.MDL", "xmile", CaseInsensitiveVolume), "Model.xmile");
}

// On a case-sensitive volume "Model.XMILE" and "Model.xmile" really are two
// files, so the rename must NOT fire -- the fix must not turn into a blanket
// case-folded comparison.
TEST(OutputPath_case_sensitive_volume_does_not_rename) {
  CHECK_EQ_STR(DeriveOutputPath("Model.XMILE", "xmile", AlwaysDifferent), "Model.xmile");
  CHECK_EQ_STR(DeriveOutputPath("Model.MDL", "mdl", AlwaysDifferent), "Model.mdl");
}

// When the filesystem cannot answer, guessing wrong toward "different" destroys
// the input, so the case-folded comparison is the fallback.
TEST(OutputPath_unknown_identity_falls_back_to_case_fold) {
  CHECK_EQ_STR(DeriveOutputPath("Model.XMILE", "xmile", AlwaysUnknown), "Model.regen.xmile");
  CHECK_EQ_STR(DeriveOutputPath("Model.STMX", "xmile", AlwaysUnknown), "Model.xmile");
}

#ifndef WIN32

namespace {

std::string MakeTempDir() {
  const char *base = getenv("TMPDIR");
  std::string tmpl = std::string(base && *base ? base : "/tmp") + "/xmutil_outputpath_XXXXXX";
  std::vector<char> buf(tmpl.begin(), tmpl.end());
  buf.push_back('\0');
  const char *dir = mkdtemp(buf.data());
  return dir ? std::string(dir) : std::string();
}

bool WriteEmptyFile(const std::string &path) {
  FILE *f = fopen(path.c_str(), "wb");
  if (!f)
    return false;
  fputs("x", f);
  fclose(f);
  return true;
}

}  // namespace

// Exercises the real QueryFileIdentity against an actual filesystem. A hard
// link gives one file two names, which is the same hazard as a case-insensitive
// volume and is reproducible on a case-sensitive one.
TEST(OutputPath_query_file_identity_on_disk) {
  const std::string dir = MakeTempDir();
  CHECK(!dir.empty());
  if (dir.empty())
    return;

  const std::string model = dir + "/model.xmile";
  const std::string alias = dir + "/model.mdl";
  const std::string other = dir + "/other.xmile";
  const std::string otherSource = dir + "/other.mdl";
  const std::string absent = dir + "/absent.xmile";
  const std::string linked = dir + "/linked.xmile";
  const std::string linkedSource = dir + "/linked.mdl";

  CHECK(WriteEmptyFile(model));
  CHECK(WriteEmptyFile(other));
  CHECK(WriteEmptyFile(otherSource));
  CHECK(WriteEmptyFile(linkedSource));
  CHECK(link(model.c_str(), alias.c_str()) == 0);
  // A symlink is the other way two names reach one file. QueryFileIdentity must
  // resolve through it, so a reimplementation that switched to lstat() or
  // symlink_status() would fail here while the hard-link case above stayed green.
  CHECK(symlink(linkedSource.c_str(), linked.c_str()) == 0);

  CHECK(QueryFileIdentity(model, model) == FileIdentity::Same);
  CHECK(QueryFileIdentity(model, alias) == FileIdentity::Same);
  CHECK(QueryFileIdentity(linked, linkedSource) == FileIdentity::Same);
  CHECK(QueryFileIdentity(other, model) == FileIdentity::Different);
  CHECK(QueryFileIdentity(absent, model) == FileIdentity::Different);

  // Converting model.mdl to XMILE would land on model.xmile, which is the same
  // file, so the guard must fire even though the two path strings differ.
  CHECK_EQ_STR(DeriveOutputPath(alias, "xmile"), dir + "/model.regen.xmile");
  CHECK_EQ_STR(DeriveOutputPath(linkedSource, "xmile"), dir + "/linked.regen.xmile");
  // ...while a stale, unrelated output file is still overwritten as before. Both
  // paths must exist for this to exercise the Different branch rather than the
  // Unknown fallback (see QueryFileIdentity).
  CHECK_EQ_STR(DeriveOutputPath(otherSource, "xmile"), other);

  unlink(model.c_str());
  unlink(alias.c_str());
  unlink(other.c_str());
  unlink(otherSource.c_str());
  unlink(linked.c_str());
  unlink(linkedSource.c_str());
  rmdir(dir.c_str());
}

#endif  // WIN32
