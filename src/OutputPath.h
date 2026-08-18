#ifndef _XMUTIL_OUTPUTPATH_H
#define _XMUTIL_OUTPUTPATH_H

#include <functional>
#include <string>

// What the filesystem says about a candidate output path relative to the input
// file being converted. "Different" deliberately covers the candidate not
// existing at all: a file that is not there cannot be clobbered.
enum class FileIdentity {
  Same,
  Different,
  Unknown,
};

// Answers FileIdentity for a candidate output path against the input path.
// Injectable so the derivation policy can be exercised without touching disk.
typedef std::function<FileIdentity(const std::string &candidate, const std::string &input)> FileIdentityFn;

// The production predicate: asks the operating system whether the two paths
// resolve to the same file. This is the only test that is correct on
// case-insensitive (macOS, Windows) and case-preserving filesystems, where
// "Model.XMILE" and "Model.xmile" are one file, and on case-sensitive ones,
// where they are two.
FileIdentity QueryFileIdentity(const std::string &candidate, const std::string &input);

// Replaces the final extension of path with extension (given without a leading
// period), following std::filesystem::path::replace_extension: "." and ".." are
// left alone, and a leading period in the filename introduces a dotfile rather
// than an extension.
std::string ReplaceFinalExtension(const std::string &path, const std::string &extension);

// The output path to write when converting inputPath to a document of the given
// extension ("mdl", "xmile"), given without a leading period.
//
// Normally that is inputPath with its extension swapped, but a conversion whose
// output extension matches its input's would destroy the model it is reading
// from -- so when the derived path would name the input file itself, ".regen"
// is interposed ("foo.mdl" --to-mdl becomes "foo.regen.mdl"). Whether the two
// paths name the same file is a filesystem question, not a string question; see
// QueryFileIdentity.
std::string DeriveOutputPath(const std::string &inputPath, const std::string &extension,
                             const FileIdentityFn &identity);
std::string DeriveOutputPath(const std::string &inputPath, const std::string &extension);

#endif  // _XMUTIL_OUTPUTPATH_H
