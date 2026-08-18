#include "OutputPath.h"

#include <cerrno>

#include "XMUtil.h"

// libc++ marks all of <filesystem> unavailable below macOS 10.15, and
// build/common.gypi sets MACOSX_DEPLOYMENT_TARGET to 10.9, so this file cannot
// reach for std::filesystem::equivalent there. (Commit 8e7c300 "build for Mac
// OS 10.14" hand-rolled the same avoidance in Main.cpp for this reason.) The
// POSIX stat() device+inode pair answers the same question, and macOS is the
// platform where the case-insensitive-filesystem hazard actually bites, so keep
// the two implementations of the query -- and only the query -- apart.
#ifdef __APPLE__
#include <sys/stat.h>
#else
#include <filesystem>
#endif

namespace {

bool IsPathSeparator(char c) {
#ifdef WIN32
  // ':' separates a drive letter (or an alternate data stream) from the rest of
  // the path; on POSIX it is an ordinary filename character.
  return c == '/' || c == '\\' || c == ':';
#else
  return c == '/';
#endif
}

// Offset of the first character of path's filename component.
size_t FilenameOffset(const std::string &path) {
  for (size_t i = path.size(); i-- > 0;) {
    if (IsPathSeparator(path[i]))
      return i + 1;
  }
  return 0;
}

// Offset of the period that introduces path's final extension, or npos when it
// has none.
size_t ExtensionOffset(const std::string &path) {
  const size_t start = FilenameOffset(path);
  const std::string filename = path.substr(start);
  if (filename == "." || filename == "..")
    return std::string::npos;
  const size_t dot = filename.rfind('.');
  // A filename whose only period is its first character is a dotfile
  // (".vensimrc"), not a bare extension.
  if (dot == std::string::npos || dot == 0)
    return std::string::npos;
  return start + dot;
}

// True when writing to candidate would destroy the input we are converting.
bool WouldClobberInput(const std::string &candidate, const std::string &input, const FileIdentityFn &identity) {
  if (candidate == input)
    return true;
  switch (identity(candidate, input)) {
  case FileIdentity::Same:
    return true;
  case FileIdentity::Different:
    return false;
  case FileIdentity::Unknown:
    break;
  }
  // The filesystem could not answer, so fall back to the one way two different
  // path strings realistically name one file here: an extension that differs
  // only in case on a case-insensitive volume. This over-triggers on a
  // case-sensitive volume, which costs a surprising ".regen" in the output
  // filename; guessing the other way costs the user their input model.
  return StringMatch(candidate, input);
}

}  // namespace

FileIdentity QueryFileIdentity(const std::string &candidate, const std::string &input) {
#ifdef __APPLE__
  struct stat candidateInfo;
  if (::stat(candidate.c_str(), &candidateInfo) != 0)
    return (errno == ENOENT || errno == ENOTDIR) ? FileIdentity::Different : FileIdentity::Unknown;
  struct stat inputInfo;
  if (::stat(input.c_str(), &inputInfo) != 0)
    return (errno == ENOENT || errno == ENOTDIR) ? FileIdentity::Different : FileIdentity::Unknown;
  return (candidateInfo.st_dev == inputInfo.st_dev && candidateInfo.st_ino == inputInfo.st_ino)
             ? FileIdentity::Same
             : FileIdentity::Different;
#else
  std::error_code ec;
  // equivalent() requires BOTH operands to exist, so the candidate's existence
  // has to be settled first -- and a candidate that does not exist is exactly
  // the common case where there is nothing to clobber. A missing *input* is not
  // special-cased: equivalent() then fails and this returns Unknown where the
  // stat() branch above would say Different. The CLI has already opened and read
  // the input by the time it derives an output name, so that asymmetry is
  // unreachable in production and only shows up in direct unit-test calls.
  if (!std::filesystem::exists(std::filesystem::path(candidate), ec))
    return ec ? FileIdentity::Unknown : FileIdentity::Different;
  const bool same = std::filesystem::equivalent(std::filesystem::path(candidate), std::filesystem::path(input), ec);
  if (ec)
    return FileIdentity::Unknown;
  return same ? FileIdentity::Same : FileIdentity::Different;
#endif
}

std::string ReplaceFinalExtension(const std::string &path, const std::string &extension) {
  const size_t dot = ExtensionOffset(path);
  return (dot == std::string::npos ? path : path.substr(0, dot)) + "." + extension;
}

std::string DeriveOutputPath(const std::string &inputPath, const std::string &extension,
                             const FileIdentityFn &identity) {
  const std::string candidate = ReplaceFinalExtension(inputPath, extension);
  if (!WouldClobberInput(candidate, inputPath, identity))
    return candidate;
  // Built from the input again rather than patched into the candidate so the
  // result cannot depend on the candidate extension's length.
  return ReplaceFinalExtension(inputPath, "regen." + extension);
}

std::string DeriveOutputPath(const std::string &inputPath, const std::string &extension) {
  return DeriveOutputPath(inputPath, extension, QueryFileIdentity);
}
