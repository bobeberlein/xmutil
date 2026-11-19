#include "CompoundFile.h"

#include <algorithm>
#include <cstring>
#include <functional>

namespace powersim {

namespace {

const uint32_t kEndOfChain = 0xFFFFFFFE;
const uint32_t kMaxRegular = 0xFFFFFFFA;  // larger values are special markers

uint16_t Le16(const char *p) {
  const unsigned char *u = reinterpret_cast<const unsigned char *>(p);
  return static_cast<uint16_t>(u[0] | (u[1] << 8));
}
uint32_t Le32(const char *p) {
  const unsigned char *u = reinterpret_cast<const unsigned char *>(p);
  return static_cast<uint32_t>(u[0]) | (static_cast<uint32_t>(u[1]) << 8) | (static_cast<uint32_t>(u[2]) << 16) |
         (static_cast<uint32_t>(u[3]) << 24);
}

void AppendUtf8(std::string &s, uint32_t c) {
  if (c < 0x80) {
    s.push_back(static_cast<char>(c));
  } else if (c < 0x800) {
    s.push_back(static_cast<char>(0xC0 | (c >> 6)));
    s.push_back(static_cast<char>(0x80 | (c & 0x3F)));
  } else {
    s.push_back(static_cast<char>(0xE0 | (c >> 12)));
    s.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
    s.push_back(static_cast<char>(0x80 | (c & 0x3F)));
  }
}

}  // namespace

bool CompoundFile::SectorBytes(uint32_t sector, const char *&p) const {
  // Sector 0 starts right after the header, which occupies one sector slot.
  uint64_t off = (static_cast<uint64_t>(sector) + 1) * sectorSize_;
  if (sector > kMaxRegular || off + sectorSize_ > len_) {
    return false;
  }
  p = data_ + off;
  return true;
}

bool CompoundFile::ReadChain(const std::vector<uint32_t> &fat, uint32_t start, uint64_t size, bool mini,
                             std::string &out, std::string &err) const {
  out.clear();
  const uint32_t unit = mini ? miniSectorSize_ : sectorSize_;
  uint32_t sector = start;
  // A chain can never be longer than the table that describes it; anything
  // more is a cycle.
  for (size_t steps = 0; out.size() < size; ++steps) {
    if (sector == kEndOfChain || sector >= fat.size() || steps > fat.size()) {
      err = "compound file: truncated or cyclic sector chain";
      return false;
    }
    size_t take = static_cast<size_t>(std::min<uint64_t>(unit, size - out.size()));
    if (mini) {
      uint64_t off = static_cast<uint64_t>(sector) * miniSectorSize_;
      if (off + take > miniStream_.size()) {
        err = "compound file: mini sector out of range";
        return false;
      }
      out.append(miniStream_, static_cast<size_t>(off), take);
    } else {
      const char *p;
      if (!SectorBytes(sector, p)) {
        err = "compound file: sector out of range";
        return false;
      }
      out.append(p, take);
    }
    sector = fat[sector];
  }
  return true;
}

bool CompoundFile::Open(const char *data, size_t len, std::string &err) {
  static const unsigned char kSig[8] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
  data_ = data;
  len_ = len;
  fat_.clear();
  miniFat_.clear();
  miniStream_.clear();
  entries_.clear();
  if (len < 512 || memcmp(data, kSig, 8) != 0) {
    err = "not a compound file (bad signature)";
    return false;
  }
  uint16_t sectorShift = Le16(data + 0x1E);
  uint16_t miniShift = Le16(data + 0x20);
  if ((sectorShift != 9 && sectorShift != 12) || miniShift != 6) {
    err = "compound file: unsupported sector size";
    return false;
  }
  sectorSize_ = 1u << sectorShift;
  miniSectorSize_ = 1u << miniShift;
  uint32_t numFatSectors = Le32(data + 0x2C);
  uint32_t firstDir = Le32(data + 0x30);
  miniCutoff_ = Le32(data + 0x38);
  uint32_t firstMiniFat = Le32(data + 0x3C);
  uint32_t firstDifat = Le32(data + 0x44);
  uint32_t numDifat = Le32(data + 0x48);

  // Collect the FAT sector list: 109 slots in the header, then the DIFAT chain.
  std::vector<uint32_t> fatSectors;
  for (int i = 0; i < 109 && fatSectors.size() < numFatSectors; ++i) {
    fatSectors.push_back(Le32(data + 0x4C + 4 * i));
  }
  uint32_t difat = firstDifat;
  const uint32_t perDifat = sectorSize_ / 4 - 1;
  for (uint32_t n = 0; n < numDifat && fatSectors.size() < numFatSectors; ++n) {
    const char *p;
    if (!SectorBytes(difat, p)) {
      err = "compound file: bad DIFAT sector";
      return false;
    }
    for (uint32_t i = 0; i < perDifat && fatSectors.size() < numFatSectors; ++i) {
      fatSectors.push_back(Le32(p + 4 * i));
    }
    difat = Le32(p + 4 * perDifat);
  }
  if (fatSectors.size() != numFatSectors) {
    err = "compound file: FAT sector list is incomplete";
    return false;
  }
  for (uint32_t s : fatSectors) {
    const char *p;
    if (!SectorBytes(s, p)) {
      err = "compound file: bad FAT sector";
      return false;
    }
    for (uint32_t i = 0; i < sectorSize_ / 4; ++i) {
      fat_.push_back(Le32(p + 4 * i));
    }
  }

  // The directory's size is not recorded (v3), so follow its chain to the end.
  std::string dir;
  {
    uint32_t sector = firstDir;
    for (size_t steps = 0; sector != kEndOfChain; ++steps) {
      const char *p;
      if (sector >= fat_.size() || steps > fat_.size() || !SectorBytes(sector, p)) {
        err = "compound file: bad directory chain";
        return false;
      }
      dir.append(p, sectorSize_);
      sector = fat_[sector];
    }
  }
  for (size_t off = 0; off + 128 <= dir.size(); off += 128) {
    const char *e = dir.data() + off;
    Entry ent;
    ent.type = static_cast<unsigned char>(e[66]);
    uint16_t nameBytes = Le16(e + 64);
    for (uint16_t i = 0; i + 1 < nameBytes && i < 62; i += 2) {
      uint16_t c = Le16(e + i);
      if (c == 0) {
        break;
      }
      AppendUtf8(ent.name, c);
    }
    ent.left = Le32(e + 68);
    ent.right = Le32(e + 72);
    ent.child = Le32(e + 76);
    ent.start = Le32(e + 116);
    // Version 3 files only define the low 32 bits of the size.
    ent.size = Le32(e + 120);
    entries_.push_back(ent);
  }
  if (entries_.empty() || entries_[0].type != 5) {
    err = "compound file: missing root entry";
    return false;
  }

  if (firstMiniFat != kEndOfChain && Le32(data + 0x40) != 0) {
    std::string raw;
    uint64_t bytes = static_cast<uint64_t>(Le32(data + 0x40)) * sectorSize_;
    if (!ReadChain(fat_, firstMiniFat, bytes, false, raw, err)) {
      return false;
    }
    for (size_t i = 0; i + 4 <= raw.size(); i += 4) {
      miniFat_.push_back(Le32(raw.data() + i));
    }
  }
  const Entry &root = entries_[0];
  if (root.size > 0 && !ReadChain(fat_, root.start, root.size, false, miniStream_, err)) {
    return false;
  }
  return true;
}

std::vector<uint32_t> CompoundFile::Children(uint32_t storage) const {
  std::vector<uint32_t> out;
  if (storage >= entries_.size()) {
    return out;
  }
  std::vector<bool> seen(entries_.size(), false);
  // Siblings form a (red-black) binary tree; an in-order walk gives directory
  // order. The seen[] guard stops a malformed file from looping forever.
  std::function<void(uint32_t)> visit = [&](uint32_t id) {
    if (id >= entries_.size() || seen[id]) {
      return;
    }
    seen[id] = true;
    visit(entries_[id].left);
    out.push_back(id);
    visit(entries_[id].right);
  };
  visit(entries_[storage].child);
  return out;
}

uint32_t CompoundFile::Find(const std::string &path) const {
  if (entries_.empty()) {
    return kNoStream;
  }
  uint32_t cur = 0;
  size_t pos = 0;
  while (pos < path.size()) {
    size_t slash = path.find('/', pos);
    std::string part = path.substr(pos, slash == std::string::npos ? std::string::npos : slash - pos);
    pos = slash == std::string::npos ? path.size() : slash + 1;
    uint32_t next = kNoStream;
    for (uint32_t c : Children(cur)) {
      if (entries_[c].name == part) {
        next = c;
        break;
      }
    }
    if (next == kNoStream) {
      return kNoStream;
    }
    cur = next;
  }
  return cur;
}

bool CompoundFile::ReadStream(uint32_t id, std::string &out, std::string &err) const {
  if (id >= entries_.size() || entries_[id].type != 2) {
    err = "compound file: not a stream";
    return false;
  }
  const Entry &e = entries_[id];
  if (e.size < miniCutoff_) {
    return ReadChain(miniFat_, e.start, e.size, true, out, err);
  }
  return ReadChain(fat_, e.start, e.size, false, out, err);
}

}  // namespace powersim
