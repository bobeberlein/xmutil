#ifndef _XMUTIL_POWERSIM_COMPOUNDFILE_H
#define _XMUTIL_POWERSIM_COMPOUNDFILE_H

#include <cstdint>
#include <string>
#include <vector>

namespace powersim {

// Read-only view of a Microsoft Compound File Binary ("OLE structured
// storage") image held in memory -- the container Powersim .sip files use.
// Only what the .sip reader needs: walk the storage tree and read streams.
// Every sector chain is bounds- and cycle-checked, since the input is an
// arbitrary user file.
class CompoundFile {
public:
  struct Entry {
    std::string name;  // UTF-8; may start with a control char (\x01, \x03, \x05)
    int type = 0;      // 1 storage, 2 stream, 5 root
    uint32_t left = kNoStream;
    uint32_t right = kNoStream;
    uint32_t child = kNoStream;
    uint32_t start = 0;
    uint64_t size = 0;
  };
  static const uint32_t kNoStream = 0xFFFFFFFF;

  // False with a reason in err when data is not a well-formed compound file.
  bool Open(const char *data, size_t len, std::string &err);

  // The entry at a '/'-separated path from the root ("" is the root), or
  // kNoStream. Matching is exact; CFB's own lookup is case-insensitive but
  // Powersim writes consistent names.
  uint32_t Find(const std::string &path) const;
  // Direct children of a storage (or the root), in directory order.
  std::vector<uint32_t> Children(uint32_t storage) const;
  const Entry &Get(uint32_t id) const {
    return entries_[id];
  }
  bool ReadStream(uint32_t id, std::string &out, std::string &err) const;

private:
  bool ReadChain(const std::vector<uint32_t> &fat, uint32_t start, uint64_t size, bool mini, std::string &out,
                 std::string &err) const;
  bool SectorBytes(uint32_t sector, const char *&p) const;

  const char *data_ = nullptr;
  size_t len_ = 0;
  uint32_t sectorSize_ = 512;
  uint32_t miniSectorSize_ = 64;
  uint32_t miniCutoff_ = 4096;
  std::vector<uint32_t> fat_;
  std::vector<uint32_t> miniFat_;
  std::string miniStream_;
  std::vector<Entry> entries_;
};

}  // namespace powersim

#endif
