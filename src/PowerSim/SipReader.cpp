#include "SipReader.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>

#include "CompoundFile.h"

// Format notes (reverse-engineered from Powersim Studio 10 files; nothing
// here comes from a published spec):
//
// A .sip is a compound file. Within it, nearly every stream is a tree of
// big-endian IFF-style chunks: 4-char id, u32 length, body, padded to even.
// FORM/LIST/CAT bodies start with a 4-char type and hold more chunks.
//
// Integers inside chunk bodies are big-endian base-128 varints (high bit =
// more bytes follow). Object ids are such varints, and their low nibble names
// the object kind: ...0 a model variable, ...4 a unit.
//
// Names are not stored with the objects. A "NAMS" form maps string ids to
// object ids, and the strings live in a Strings/<storage>/<language> stream.
// The link from a NAMS form to its storage is not decoded (the storage names
// are not derived from any GUID we could find), so the table is picked by
// matching its string-id set, which is unique in every file seen so far.
//
// Equations (DTXT) carry two sections. The second is the token stream: a
// count, then operators (two bytes, first >= 0xE0) and object references
// (varints). The first is the source layout: before each token comes a "gap"
// holding the text between tokens -- whitespace, comments, commas, and the
// numeric literals, which are therefore not tokens at all. Each gap ends with
// 0x00 (or at the end of the section); a token index follows each gap.

namespace powersim {

namespace {

// ---------------------------------------------------------------------------
// Chunk tree
// ---------------------------------------------------------------------------

struct Chunk {
  std::string id;    // FORM, LIST, CAT, or a leaf id
  std::string type;  // container type; empty for leaves
  std::string body;  // leaf payload
  std::vector<Chunk> kids;

  bool IsContainer() const {
    return !type.empty();
  }
  const Chunk *Leaf(const char *leafId) const {
    for (const Chunk &k : kids) {
      if (!k.IsContainer() && k.id == leafId) {
        return &k;
      }
    }
    return nullptr;
  }
  const Chunk *Container(const char *containerType) const {
    for (const Chunk &k : kids) {
      if (k.IsContainer() && k.type == containerType) {
        return &k;
      }
    }
    return nullptr;
  }
  void CollectContainers(const char *containerType, std::vector<const Chunk *> &out) const {
    for (const Chunk &k : kids) {
      if (k.IsContainer()) {
        if (k.type == containerType) {
          out.push_back(&k);
        }
        k.CollectContainers(containerType, out);
      }
    }
  }
};

uint32_t Be32(const std::string &s, size_t off) {
  const unsigned char *u = reinterpret_cast<const unsigned char *>(s.data() + off);
  return (static_cast<uint32_t>(u[0]) << 24) | (static_cast<uint32_t>(u[1]) << 16) |
         (static_cast<uint32_t>(u[2]) << 8) | static_cast<uint32_t>(u[3]);
}

bool ParseChunks(const std::string &d, size_t off, size_t end, int depth, std::vector<Chunk> &out) {
  if (depth > 64) {
    return false;
  }
  while (off + 8 <= end) {
    Chunk c;
    c.id = d.substr(off, 4);
    uint32_t n = Be32(d, off + 4);
    size_t body = off + 8;
    if (n > end - body) {
      return false;
    }
    if (c.id == "FORM" || c.id == "LIST" || c.id == "CAT ") {
      if (n < 4) {
        return false;
      }
      c.type = d.substr(body, 4);
      if (!ParseChunks(d, body + 4, body + n, depth + 1, c.kids)) {
        return false;
      }
    } else {
      c.body = d.substr(body, n);
    }
    out.push_back(std::move(c));
    off = body + n + (n & 1);
  }
  return true;
}

bool ParseRoot(const std::string &d, Chunk &root) {
  root = Chunk();
  root.id = "FILE";
  root.type = "FILE";
  return ParseChunks(d, 0, d.size(), 0, root.kids);
}

// Big-endian base-128 varint.
bool ReadVarint(const std::string &b, size_t &i, uint32_t &v) {
  v = 0;
  for (int n = 0; n < 5; ++n) {
    if (i >= b.size()) {
      return false;
    }
    unsigned char c = static_cast<unsigned char>(b[i++]);
    v = (v << 7) | (c & 0x7F);
    if (!(c & 0x80)) {
      return true;
    }
  }
  return false;
}

std::string Latin1ToUtf8(const std::string &s) {
  std::string out;
  for (unsigned char c : s) {
    if (c < 0x80) {
      out.push_back(static_cast<char>(c));
    } else {
      out.push_back(static_cast<char>(0xC0 | (c >> 6)));
      out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    }
  }
  return out;
}

std::string Utf16BeToUtf8(const std::string &s, size_t off, size_t chars) {
  std::string out;
  for (size_t k = 0; k < chars; ++k) {
    uint32_t c = (static_cast<unsigned char>(s[off + 2 * k]) << 8) | static_cast<unsigned char>(s[off + 2 * k + 1]);
    if (c < 0x80) {
      out.push_back(static_cast<char>(c));
    } else if (c < 0x800) {
      out.push_back(static_cast<char>(0xC0 | (c >> 6)));
      out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    } else {
      out.push_back(static_cast<char>(0xE0 | (c >> 12)));
      out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    }
  }
  return out;
}

std::string XmlEscape(const std::string &s) {
  std::string out;
  for (char c : s) {
    switch (c) {
    case '&':
      out += "&amp;";
      break;
    case '<':
      out += "&lt;";
      break;
    case '>':
      out += "&gt;";
      break;
    case '"':
      out += "&quot;";
      break;
    default:
      out.push_back(c);
    }
  }
  return out;
}

std::string FormatDouble(double v) {
  char buf[64];
  // %.17g round-trips any double; try shorter first for readable output.
  for (int prec = 6; prec <= 17; ++prec) {
    snprintf(buf, sizeof buf, "%.*g", prec, v);
    if (strtod(buf, nullptr) == v) {
      break;
    }
  }
  return buf;
}

// ---------------------------------------------------------------------------
// String tables
// ---------------------------------------------------------------------------

typedef std::map<uint32_t, std::string> StringTable;

// FTXT: entries of { sid varint, 3 bytes (flag + language), char count varint,
// byte count varint, bytes }, ending with a 0 byte.
bool ParseFtxt(const std::string &b, StringTable &tbl) {
  size_t i = 0;
  while (i < b.size() && b[i] != 0) {
    uint32_t sid, chars, bytes;
    if (!ReadVarint(b, i, sid) || i + 3 > b.size()) {
      return false;
    }
    i += 3;
    if (!ReadVarint(b, i, chars) || !ReadVarint(b, i, bytes) || bytes > b.size() - i) {
      return false;
    }
    std::string raw = b.substr(i, bytes);
    i += bytes;
    // Every sample is single-byte text (chars == bytes); anything else is
    // taken to already be UTF-8.
    if (!tbl.count(sid)) {
      tbl[sid] = chars == bytes ? Latin1ToUtf8(raw) : raw;
    }
  }
  return true;
}

// Every Strings/<storage> table, merged across its language streams. The
// language-neutral stream (0000) wins, then English (0009), then the rest:
// unit symbols live in 0000 with only "dimensionless" in 0009.
bool LoadStringTables(const CompoundFile &cf, std::map<std::string, StringTable> &tables, std::string &err) {
  uint32_t strings = cf.Find("Strings");
  if (strings == CompoundFile::kNoStream) {
    err = "no Strings storage";
    return false;
  }
  for (uint32_t st : cf.Children(strings)) {
    if (cf.Get(st).type != 1) {
      continue;
    }
    std::vector<uint32_t> langs;
    for (const char *pref : {"0000", "0009"}) {
      for (uint32_t s : cf.Children(st)) {
        if (cf.Get(s).name == pref) {
          langs.push_back(s);
        }
      }
    }
    for (uint32_t s : cf.Children(st)) {
      const std::string &n = cf.Get(s).name;
      if (cf.Get(s).type == 2 && n != "Contents" && n != "0000" && n != "0009") {
        langs.push_back(s);
      }
    }
    StringTable &tbl = tables[cf.Get(st).name];
    for (uint32_t s : langs) {
      std::string data;
      Chunk root;
      if (!cf.ReadStream(s, data, err) || !ParseRoot(data, root)) {
        err = "unreadable string table " + cf.Get(st).name;
        return false;
      }
      std::vector<const Chunk *> forms;
      root.CollectContainers("LANG", forms);
      for (const Chunk *f : forms) {
        if (const Chunk *ftxt = f->Leaf("FTXT")) {
          if (!ParseFtxt(ftxt->body, tbl)) {
            err = "malformed text in string table " + cf.Get(st).name;
            return false;
          }
        }
      }
    }
  }
  return true;
}

// NAMS leaf: u32 length, count varint, then (string id, object id) varints.
bool ParseNamsLeaf(const std::string &b, std::map<uint32_t, uint32_t> &sidToObj) {
  if (b.size() < 4) {
    return false;
  }
  size_t i = 4;
  uint32_t count;
  if (!ReadVarint(b, i, count)) {
    return false;
  }
  for (uint32_t k = 0; k < count; ++k) {
    uint32_t sid, obj;
    if (!ReadVarint(b, i, sid) || !ReadVarint(b, i, obj)) {
      return false;
    }
    sidToObj[sid] = obj;
  }
  return true;
}

// Resolves (string id -> object id) pairs to (object id -> name) through the
// one string table whose id set matches exactly.
bool ResolveNames(const std::map<uint32_t, uint32_t> &sidToObj, const std::map<std::string, StringTable> &tables,
                  const char *what, std::map<uint32_t, std::string> &out, std::vector<std::string> &errs) {
  if (sidToObj.empty()) {
    return true;
  }
  const StringTable *match = nullptr;
  int matches = 0;
  for (const auto &t : tables) {
    if (t.second.size() != sidToObj.size()) {
      continue;
    }
    bool same = true;
    for (const auto &p : sidToObj) {
      if (!t.second.count(p.first)) {
        same = false;
        break;
      }
    }
    if (same) {
      match = &t.second;
      ++matches;
    }
  }
  if (matches != 1) {
    errs.push_back(std::string("Powersim: could not identify the string table for ") + what +
                   (matches ? " (ambiguous)" : " (none matches)"));
    return false;
  }
  for (const auto &p : sidToObj) {
    out[p.second] = match->at(p.first);
  }
  return true;
}

// ---------------------------------------------------------------------------
// Equations
// ---------------------------------------------------------------------------

enum OpCode : uint32_t {
  kOpMul = 0xEE20,
  kOpDiv = 0xEE1F,
  kOpPlus = 0xEF7F,
  kOpMinus = 0xEF7D,
  kOpGt = 0xEF69,
  kOpLt = 0xEF6A,
  kOpLParen = 0xF041,
  kOpRParen = 0xF042,
  kOpLBrace = 0xF043,
  kOpRBrace = 0xF044,
  kOpUnitOpen = 0xF047,
  kOpUnitClose = 0xF048,
  kOpUnitDiv = 0xF053,  // '/' inside a <<unit>> annotation
  kOpIf = 0xEF2E,
  kOpStep = 0xEE74,
  kOpGraph = 0xEF34,
  kOpStartTime = 0xF05D,
};

const char *SimpleOpText(uint32_t op) {
  switch (op) {
  case kOpMul:
    return "*";
  case kOpDiv:
    return "/";
  case kOpPlus:
    return "+";
  case kOpMinus:
    return "-";
  case kOpGt:
    return ">";
  case kOpLt:
    return "<";
  case kOpLParen:
    return "(";
  case kOpRParen:
    return ")";
  case kOpStep:
    return "STEP";
  case kOpStartTime:
    return "STARTTIME";
  default:
    return nullptr;
  }
}

struct Lexeme {
  enum Kind { kNumber, kOp, kRef, kComma } kind = kNumber;
  uint32_t code = 0;  // operator or object id
  double num = 0;
  std::string text;  // a number's source spelling, when present
};

bool IsBlankOrComment(const std::string &s) {
  size_t i = 0;
  while (i < s.size()) {
    if (isspace(static_cast<unsigned char>(s[i]))) {
      ++i;
    } else if (s.compare(i, 2, "//") == 0) {
      // Powersim comments run //...// ; tolerate an unterminated one.
      size_t close = s.find("//", i + 2);
      i = close == std::string::npos ? s.size() : close + 2;
    } else {
      return false;
    }
  }
  return true;
}

class EquationDecoder {
public:
  EquationDecoder(const std::map<uint32_t, std::string> &vars, const std::map<uint32_t, std::string> &units,
                  const std::string &owner, std::vector<std::string> &errs)
      : vars_(vars), units_(units), owner_(owner), errs_(errs) {
  }

  bool Decode(const std::string &dtxt, std::vector<Lexeme> &out) {
    if (dtxt.size() < 4) {
      return Fail("truncated definition");
    }
    uint32_t n1 = Be32(dtxt, 0);
    if (n1 > dtxt.size() - 8) {
      return Fail("truncated definition");
    }
    std::string layout = dtxt.substr(4, n1);
    uint32_t n2 = Be32(dtxt, 4 + n1);
    if (n2 > dtxt.size() - 8 - n1) {
      return Fail("truncated definition");
    }
    std::string tokenSection = dtxt.substr(8 + n1, n2);

    std::vector<Lexeme> tokens;
    size_t i = 0;
    uint32_t count;
    if (!tokenSection.empty() && !ReadVarint(tokenSection, i, count)) {
      return Fail("bad token count");
    }
    while (i < tokenSection.size()) {
      Lexeme t;
      unsigned char c = static_cast<unsigned char>(tokenSection[i]);
      if (c >= 0xE0) {
        if (i + 2 > tokenSection.size()) {
          return Fail("truncated operator");
        }
        t.kind = Lexeme::kOp;
        t.code = (c << 8) | static_cast<unsigned char>(tokenSection[i + 1]);
        i += 2;
      } else {
        t.kind = Lexeme::kRef;
        if (!ReadVarint(tokenSection, i, t.code)) {
          return Fail("truncated reference");
        }
      }
      tokens.push_back(t);
    }

    i = 0;
    uint32_t header;
    if (!layout.empty() && !ReadVarint(layout, i, header)) {
      return Fail("bad layout header");
    }
    while (i < layout.size()) {
      if (!Gap(layout, i, out)) {
        return false;
      }
      if (i >= layout.size()) {
        break;
      }
      uint32_t idx;
      if (!ReadVarint(layout, i, idx) || idx >= tokens.size()) {
        return Fail("token index out of range");
      }
      out.push_back(tokens[idx]);
    }
    return true;
  }

private:
  bool Fail(const std::string &why) {
    errs_.push_back("Powersim: cannot decode the definition of '" + owner_ + "': " + why);
    return false;
  }

  bool Gap(const std::string &b, size_t &i, std::vector<Lexeme> &out) {
    bool spellingPending = false;
    while (i < b.size()) {
      unsigned char c = static_cast<unsigned char>(b[i]);
      if (c == 0x00) {
        ++i;
        return true;
      }
      if (c < 0x20) {
        return true;  // a token index follows directly
      }
      std::string text;
      bool isText = false;
      if ((c & 0xE0) == 0x20) {
        if (i + 2 > b.size()) {
          return Fail("truncated literal");
        }
        unsigned char type = static_cast<unsigned char>(b[i + 1]);
        i += 2;
        Lexeme n;
        n.kind = Lexeme::kNumber;
        if (type == 0x42) {
          if (i + 2 > b.size()) {
            return Fail("truncated literal");
          }
          n.num = static_cast<int16_t>((static_cast<unsigned char>(b[i]) << 8) | static_cast<unsigned char>(b[i + 1]));
          i += 2;
        } else if (type == 0xC8) {
          if (i + 8 > b.size()) {
            return Fail("truncated literal");
          }
          uint64_t bits = (static_cast<uint64_t>(Be32(b, i)) << 32) | Be32(b, i + 4);
          memcpy(&n.num, &bits, sizeof n.num);
          i += 8;
        } else {
          char buf[64];
          snprintf(buf, sizeof buf, "unknown literal type 0x%02X", type);
          return Fail(buf);
        }
        out.push_back(n);
        spellingPending = true;
        continue;
      } else if (c == 0x40) {
        if (i + 2 > b.size()) {
          return Fail("truncated text");
        }
        text = Latin1ToUtf8(b.substr(i + 1, 1));
        i += 2;
        isText = true;
      } else if (c == 0x60 || c == 0x80) {
        ++i;
        uint32_t chars;
        if (!ReadVarint(b, i, chars) || chars > (b.size() - i) / 2) {
          return Fail("truncated text");
        }
        text = Utf16BeToUtf8(b, i, chars);
        i += 2 * static_cast<size_t>(chars);
        isText = true;
      } else if (c == 0xA0) {
        if (i + 2 > b.size() || b[i + 1] != 0) {
          return Fail("unknown separator");
        }
        Lexeme comma;
        comma.kind = Lexeme::kComma;
        out.push_back(comma);
        i += 2;
      } else {
        char buf[64];
        snprintf(buf, sizeof buf, "unknown layout item 0x%02X", c);
        return Fail(buf);
      }
      if (isText) {
        if (spellingPending) {
          out.back().text = text;
        } else if (!IsBlankOrComment(text)) {
          return Fail("unexpected text '" + text + "'");
        }
      }
      spellingPending = false;
    }
    return true;
  }

  const std::map<uint32_t, std::string> &vars_;
  const std::map<uint32_t, std::string> &units_;
  std::string owner_;
  std::vector<std::string> &errs_;
};

std::string XmileRefName(const std::string &name) {
  std::string out;
  bool plain = !name.empty() && !isdigit(static_cast<unsigned char>(name[0]));
  for (char c : name) {
    if (c == ' ') {
      out.push_back('_');
    } else {
      out.push_back(c);
      if (!isalnum(static_cast<unsigned char>(c)) && c != '_' && !(c & 0x80)) {
        plain = false;
      }
    }
  }
  return plain ? out : "\"" + out + "\"";
}

// Turns a lexeme range into XMILE equation text.
class EquationWriter {
public:
  EquationWriter(const std::vector<Lexeme> &lx, const std::map<uint32_t, std::string> &vars,
                 const std::map<uint32_t, std::string> &units, const std::string &timeUnit, const std::string &owner,
                 std::vector<std::string> &errs)
      : lx_(lx), vars_(vars), units_(units), timeUnit_(timeUnit), owner_(owner), errs_(errs) {
  }

  bool Write(size_t b, size_t e, std::string &out) {
    for (size_t i = b; i < e; ++i) {
      const Lexeme &l = lx_[i];
      switch (l.kind) {
      case Lexeme::kNumber:
        out += NumberText(l);
        break;
      case Lexeme::kComma:
        out += ",";
        break;
      case Lexeme::kRef: {
        auto it = vars_.find(l.code);
        if (it == vars_.end()) {
          return Fail(units_.count(l.code) ? "unit '" + units_.at(l.code) + "' outside a <<unit>> annotation"
                                           : "reference to an unknown object");
        }
        out += XmileRefName(it->second);
        break;
      }
      case Lexeme::kOp: {
        if (l.code == kOpUnitOpen) {
          size_t close = Matching(i, kOpUnitOpen, kOpUnitClose, e);
          if (close == e) {
            return Fail("unterminated <<unit>>");
          }
          if (!UnitIsScaleFree(i, close)) {
            return false;
          }
          i = close;
        } else if (l.code == kOpIf) {
          std::vector<std::pair<size_t, size_t>> args;
          size_t close;
          if (!CallArgs(i, e, args, close) || args.size() != 3) {
            return Fail("IF needs three arguments");
          }
          std::string c, t, f;
          if (!Write(args[0].first, args[0].second, c) || !Write(args[1].first, args[1].second, t) ||
              !Write(args[2].first, args[2].second, f)) {
            return false;
          }
          out += "(IF " + c + " THEN " + t + " ELSE " + f + ")";
          i = close;
        } else if (const char *s = SimpleOpText(l.code)) {
          out += s;
        } else if (l.code == kOpGraph) {
          return Fail("GRAPH is only supported as a variable's whole definition");
        } else {
          char buf[64];
          snprintf(buf, sizeof buf, "unsupported operator 0x%04X", l.code);
          return Fail(buf);
        }
        break;
      }
      }
    }
    return true;
  }

  // Parses `OP ( a , b , ... )` starting at the operator index; args are
  // [begin,end) lexeme ranges split at top-level commas.
  bool CallArgs(size_t op, size_t e, std::vector<std::pair<size_t, size_t>> &args, size_t &close) {
    if (op + 1 >= e || lx_[op + 1].kind != Lexeme::kOp || lx_[op + 1].code != kOpLParen) {
      return false;
    }
    close = Matching(op + 1, kOpLParen, kOpRParen, e);
    if (close == e) {
      return false;
    }
    int depth = 0;
    size_t start = op + 2;
    for (size_t i = op + 2; i < close; ++i) {
      const Lexeme &l = lx_[i];
      if (l.kind == Lexeme::kOp && (l.code == kOpLParen || l.code == kOpLBrace)) {
        ++depth;
      } else if (l.kind == Lexeme::kOp && (l.code == kOpRParen || l.code == kOpRBrace)) {
        --depth;
      } else if (l.kind == Lexeme::kComma && depth == 0) {
        args.push_back({start, i});
        start = i + 1;
      }
    }
    args.push_back({start, close});
    return true;
  }

  // A numeric constant made of an optional sign, a literal, and an optional
  // unit annotation.
  bool Constant(size_t b, size_t e, double &v) {
    bool neg = false;
    if (b < e && lx_[b].kind == Lexeme::kOp && (lx_[b].code == kOpMinus || lx_[b].code == kOpPlus)) {
      neg = lx_[b].code == kOpMinus;
      ++b;
    }
    if (b >= e || lx_[b].kind != Lexeme::kNumber) {
      return false;
    }
    double num;
    if (!NumberValue(lx_[b], num)) {
      num = lx_[b].num;
    }
    v = neg ? -num : num;
    ++b;
    if (b < e && lx_[b].kind == Lexeme::kOp && lx_[b].code == kOpUnitOpen) {
      size_t close = Matching(b, kOpUnitOpen, kOpUnitClose, e);
      if (close == e || !UnitIsScaleFree(b, close)) {
        return false;
      }
      b = close + 1;
    }
    return b == e;
  }

  size_t Matching(size_t open, uint32_t openCode, uint32_t closeCode, size_t e) const {
    int depth = 0;
    for (size_t i = open; i < e; ++i) {
      if (lx_[i].kind != Lexeme::kOp) {
        continue;
      }
      if (lx_[i].code == openCode) {
        ++depth;
      } else if (lx_[i].code == closeCode && --depth == 0) {
        return i;
      }
    }
    return e;
  }

  bool Fail(const std::string &why) {
    errs_.push_back("Powersim: cannot translate the definition of '" + owner_ + "': " + why);
    return false;
  }

private:
  // The source spelling of a literal when it denotes the stored value.
  // Powersim's stored double can be an ulp or two off what was typed (".333"
  // is kept as 0.33299999999999996), so the typed form wins within that slack.
  static bool NumberValue(const Lexeme &l, double &v) {
    if (l.text.empty()) {
      return false;
    }
    char *end = nullptr;
    v = strtod(l.text.c_str(), &end);
    return end && *end == 0 && std::fabs(v - l.num) <= 4 * 2.220446049250313e-16 * std::fabs(l.num);
  }
  static std::string NumberText(const Lexeme &l) {
    double v;
    return NumberValue(l, v) ? l.text : FormatDouble(l.num);
  }

  // Dropping a <<unit>> annotation keeps the number as typed, which is only
  // right when Powersim would not rescale it: the project time unit,
  // dimensionless, and units the model defines itself. A built-in unit with a
  // scale (<<da>> in a model run in years, <<%>>) would change the value.
  // TODO: apply the conversion factors instead of refusing.
  bool UnitIsScaleFree(size_t open, size_t close) {
    static const char *const kScaled[] = {"s",  "min",    "hr",      "da",  "wk",   "mo", "qtr",
                                          "yr", "period", "periods", "deg", "grad", "%"};
    for (size_t i = open + 1; i < close; ++i) {
      if (lx_[i].kind != Lexeme::kRef || !units_.count(lx_[i].code)) {
        continue;
      }
      const std::string &u = units_.at(lx_[i].code);
      if (u == timeUnit_ || (timeUnit_ == "period" && u == "periods")) {
        continue;
      }
      for (const char *k : kScaled) {
        if (u == k) {
          return Fail("<<" + u + ">> needs a unit conversion, which is not supported yet");
        }
      }
    }
    return true;
  }

  const std::vector<Lexeme> &lx_;
  const std::map<uint32_t, std::string> &vars_;
  const std::map<uint32_t, std::string> &units_;
  std::string timeUnit_;
  std::string owner_;
  std::vector<std::string> &errs_;
};

// ---------------------------------------------------------------------------
// Units
// ---------------------------------------------------------------------------
//
// Powersim stores no units on variables (every UNIT form is AUTO): it derives
// them from the definitions, where literals carry <<unit>> annotations, and it
// rejects a model whose units do not balance. Re-deriving them the same way
// recovers the units Powersim shows. A bare literal is dimensionless, but is
// marked weak: where it meets a real unit in +, -, or an IF branch (an
// unannotated 0 for a stock's start) the real unit wins.

struct Dim {
  std::map<std::string, int> exp;  // base unit -> exponent, zeros removed
  bool weak = false;               // only bare literals went into it
};

Dim Combine(const Dim &a, const Dim &b, int sign) {
  Dim r = a;
  for (const auto &p : b.exp) {
    if ((r.exp[p.first] += sign * p.second) == 0) {
      r.exp.erase(p.first);
    }
  }
  r.weak = a.weak && b.weak;
  return r;
}

std::string FormatDim(const Dim &d) {
  std::string num, den;
  int dens = 0;
  for (const auto &p : d.exp) {
    std::string &side = p.second > 0 ? num : den;
    int power = p.second > 0 ? p.second : -p.second;
    if (!side.empty()) {
      side += "*";
    }
    side += p.first + (power > 1 ? "^" + std::to_string(power) : "");
    dens += p.second < 0;
  }
  if (num.empty() && den.empty()) {
    return "dmnl";
  }
  if (den.empty()) {
    return num;
  }
  return (num.empty() ? "1" : num) + "/" + (dens > 1 ? "(" + den + ")" : den);
}

// Evaluates the unit of a lexeme range. Returns false when the unit cannot be
// told yet (a referenced variable's unit is still unknown) or the expression
// holds something the evaluator does not model; inference is advisory, so the
// variable is then simply left without units.
class UnitEvaluator {
public:
  UnitEvaluator(const std::vector<Lexeme> &lx, const std::map<uint32_t, Dim> &known,
                const std::map<uint32_t, std::string> &unitNames, const std::string &timeUnit)
      : lx_(lx), known_(known), unitNames_(unitNames), timeUnit_(timeUnit) {
  }

  bool Eval(size_t b, size_t e, Dim &out) {
    p_ = b;
    e_ = e;
    return Compare(out) && p_ == e_;
  }

private:
  bool IsOp(uint32_t code) const {
    return p_ < e_ && lx_[p_].kind == Lexeme::kOp && lx_[p_].code == code;
  }
  bool Expect(uint32_t code) {
    if (!IsOp(code)) {
      return false;
    }
    ++p_;
    return true;
  }
  bool ExpectComma() {
    if (p_ >= e_ || lx_[p_].kind != Lexeme::kComma) {
      return false;
    }
    ++p_;
    return true;
  }

  bool Compare(Dim &out) {
    if (!Sum(out)) {
      return false;
    }
    while (IsOp(kOpGt) || IsOp(kOpLt)) {
      ++p_;
      Dim rhs;
      if (!Sum(rhs)) {
        return false;
      }
      out = Dim();
    }
    return true;
  }

  // + and - keep the operands' unit; a weak operand defers to a strong one.
  bool Sum(Dim &out) {
    if (!Product(out)) {
      return false;
    }
    while (IsOp(kOpPlus) || IsOp(kOpMinus)) {
      ++p_;
      Dim rhs;
      if (!Product(rhs)) {
        return false;
      }
      out = Pick(out, rhs);
    }
    return true;
  }

  static Dim Pick(const Dim &a, const Dim &b) {
    return a.weak && !b.weak ? b : a;
  }

  bool Product(Dim &out) {
    if (!Unary(out)) {
      return false;
    }
    while (IsOp(kOpMul) || IsOp(kOpDiv)) {
      int sign = lx_[p_].code == kOpMul ? 1 : -1;
      ++p_;
      Dim rhs;
      if (!Unary(rhs)) {
        return false;
      }
      out = Combine(out, rhs, sign);
    }
    return true;
  }

  bool Unary(Dim &out) {
    while (IsOp(kOpPlus) || IsOp(kOpMinus)) {
      ++p_;
    }
    return Primary(out);
  }

  bool Primary(Dim &out) {
    if (p_ >= e_) {
      return false;
    }
    const Lexeme &l = lx_[p_];
    if (l.kind == Lexeme::kNumber) {
      ++p_;
      out = Dim();
      out.weak = true;
      if (IsOp(kOpUnitOpen)) {
        ++p_;
        if (!UnitTerm(out) || !Expect(kOpUnitClose)) {
          return false;
        }
        out.weak = false;
      }
      return true;
    }
    if (l.kind == Lexeme::kRef) {
      auto it = known_.find(l.code);
      if (it == known_.end()) {
        return false;
      }
      out = it->second;
      ++p_;
      return true;
    }
    if (l.kind != Lexeme::kOp) {
      return false;
    }
    switch (l.code) {
    case kOpLParen:
      ++p_;
      return Compare(out) && Expect(kOpRParen);
    case kOpStartTime:
      ++p_;
      out = Dim();
      out.exp[timeUnit_] = 1;
      return true;
    case kOpIf: {
      ++p_;
      Dim cond, a, b;
      if (!Expect(kOpLParen) || !Compare(cond) || !ExpectComma() || !Compare(a) || !ExpectComma() || !Compare(b) ||
          !Expect(kOpRParen)) {
        return false;
      }
      out = Pick(a, b);
      return true;
    }
    case kOpStep: {
      ++p_;
      Dim at;
      return Expect(kOpLParen) && Compare(out) && ExpectComma() && Compare(at) && Expect(kOpRParen);
    }
    case kOpGraph: {
      // The output is the unit of the {y} values: bare numbers in every
      // sample, so dimensionless. Skip the arguments.
      int depth = 0;
      for (++p_; p_ < e_; ++p_) {
        if (IsOp(kOpLParen)) {
          ++depth;
        } else if (IsOp(kOpRParen) && --depth == 0) {
          ++p_;
          out = Dim();
          return true;
        }
      }
      return false;
    }
    default:
      return false;
    }
  }

  // Inside <<...>>: factors joined by * and /, a factor being a unit, the
  // literal 1, or a parenthesized term.
  bool UnitTerm(Dim &out) {
    out = Dim();
    int sign = 1;
    for (;;) {
      Dim f;
      if (p_ >= e_) {
        return false;
      }
      const Lexeme &l = lx_[p_];
      if (l.kind == Lexeme::kNumber && l.num == 1) {
        ++p_;
      } else if (l.kind == Lexeme::kRef && unitNames_.count(l.code)) {
        std::string n = unitNames_.at(l.code);
        if (n == "periods") {
          n = "period";
        }
        if (n != "dimensionless") {
          f.exp[n] = 1;
        }
        ++p_;
      } else if (IsOp(kOpLParen)) {
        ++p_;
        if (!UnitTerm(f) || !Expect(kOpRParen)) {
          return false;
        }
      } else {
        return false;
      }
      out = Combine(out, f, sign);
      if (IsOp(kOpUnitDiv) || IsOp(kOpDiv)) {
        sign = -1;
      } else if (IsOp(kOpMul)) {
        sign = 1;
      } else {
        return true;
      }
      ++p_;
    }
  }

  const std::vector<Lexeme> &lx_;
  const std::map<uint32_t, Dim> &known_;
  const std::map<uint32_t, std::string> &unitNames_;
  std::string timeUnit_;
  size_t p_ = 0, e_ = 0;
};

// ---------------------------------------------------------------------------
// Model
// ---------------------------------------------------------------------------

struct SipFlow {
  uint32_t id;
  bool outflow;
};

struct SipVar {
  uint32_t id = 0;
  std::string name;
  bool isLevel = false;
  std::vector<SipFlow> flows;
  std::string dtxt;
};

// VAR NAME leaf: 3 flag bytes, two FILETIMEs, then the length-prefixed id.
bool VarId(const std::string &b, uint32_t &id) {
  if (b.size() < 20) {
    return false;
  }
  size_t i = 19;
  size_t len = static_cast<unsigned char>(b[i++]);
  size_t start = i;
  return ReadVarint(b, i, id) && i - start == len;
}

bool ReadVars(const Chunk &root, std::vector<SipVar> &vars, std::vector<std::string> &errs) {
  const Chunk *chld = root.Container("CHLD");
  const Chunk *list = chld ? chld->Container("VAR ") : nullptr;
  if (!list) {
    errs.push_back("Powersim: the model has no variables");
    return false;
  }
  for (const Chunk &v : list->kids) {
    if (!v.IsContainer() || v.type != "VAR ") {
      continue;
    }
    SipVar sv;
    const Chunk *name = v.Leaf("NAME");
    const Chunk *typs = v.Leaf("TYPS");
    const Chunk *dtxt = v.Leaf("DTXT");
    if (!name || !VarId(name->body, sv.id) || !typs || typs->body.empty()) {
      errs.push_back("Powersim: malformed variable record");
      return false;
    }
    if (v.Container("CHLD")) {
      errs.push_back("Powersim: models with submodels are not supported yet");
      return false;
    }
    if (const Chunk *dims = v.Container("DIMS")) {
      for (const Chunk &k : dims->kids) {
        if (k.id != "AUTO") {
          errs.push_back("Powersim: arrayed variables are not supported yet");
          return false;
        }
      }
    }
    unsigned char kind = static_cast<unsigned char>(typs->body[0]);
    if (kind > 1) {
      char buf[96];
      snprintf(buf, sizeof buf, "Powersim: unsupported variable kind %u", kind);
      errs.push_back(buf);
      return false;
    }
    sv.isLevel = kind == 1;
    if (const Chunk *flws = v.Container("FLWS")) {
      if (const Chunk *items = flws->Container("ITEM")) {
        for (const Chunk &it : items->kids) {
          const Chunk *rtid = it.Leaf("RTID");
          const Chunk *sign = it.Leaf("SIGN");
          size_t i = 0;
          SipFlow f;
          if (!rtid || !sign || sign->body.empty() || !ReadVarint(rtid->body, i, f.id)) {
            errs.push_back("Powersim: malformed flow record");
            return false;
          }
          f.outflow = sign->body[0] != 0;
          sv.flows.push_back(f);
        }
      }
    }
    if (dtxt) {
      sv.dtxt = dtxt->body;
    }
    vars.push_back(sv);
  }
  return true;
}

// TIM1: mode byte, then start, stop, dt as doubles. Mode 3 is plain numbers
// in Powersim's abstract time unit, "period". Mode 1 is calendar time in
// seconds from year 0 on a 360-day year; it is expressed in years, the unit
// every calendar sample writes its time constants in.
bool ReadTimes(const CompoundFile &cf, std::string &specs, std::string &timeUnit, std::vector<std::string> &errs) {
  uint32_t tasks = cf.Find("Tasks");
  for (uint32_t t : tasks == CompoundFile::kNoStream ? std::vector<uint32_t>() : cf.Children(tasks)) {
    for (uint32_t s : cf.Children(t)) {
      if (cf.Get(s).name != "Simulation") {
        continue;
      }
      std::string data, err;
      Chunk root;
      if (!cf.ReadStream(s, data, err) || !ParseRoot(data, root)) {
        continue;
      }
      for (const Chunk &top : root.kids) {
        const Chunk *tim = top.Leaf("TIM1");
        if (!tim || tim->body.size() < 25) {
          continue;
        }
        double v[3];
        for (int k = 0; k < 3; ++k) {
          uint64_t bits = (static_cast<uint64_t>(Be32(tim->body, 1 + 8 * k)) << 32) | Be32(tim->body, 5 + 8 * k);
          memcpy(&v[k], &bits, sizeof(double));
        }
        unsigned char mode = static_cast<unsigned char>(tim->body[0]);
        if (mode == 1) {
          const double year = 360.0 * 86400.0;
          for (double &x : v) {
            x /= year;
          }
          timeUnit = "yr";
        } else if (mode == 3) {
          timeUnit = "period";
        } else {
          char buf[96];
          snprintf(buf, sizeof buf, "Powersim: unsupported time mode %u", mode);
          errs.push_back(buf);
          return false;
        }
        if (!(v[2] > 0) || !(v[1] >= v[0])) {
          errs.push_back("Powersim: implausible simulation time settings");
          return false;
        }
        specs = "  <sim_specs method=\"Euler\" time_units=\"" + timeUnit + "\">\n    <start>" + FormatDouble(v[0]) +
                "</start>\n    <stop>" + FormatDouble(v[1]) + "</stop>\n    <dt>" + FormatDouble(v[2]) +
                "</dt>\n  </sim_specs>\n";
        return true;
      }
    }
  }
  errs.push_back("Powersim: no simulation time settings found");
  return false;
}

bool ReadStreamChunks(const CompoundFile &cf, uint32_t id, Chunk &root, std::vector<std::string> &errs) {
  std::string data, err;
  if (!cf.ReadStream(id, data, err) || !ParseRoot(data, root)) {
    errs.push_back("Powersim: unreadable stream " + cf.Get(id).name + (err.empty() ? "" : ": " + err));
    return false;
  }
  return true;
}

void CollectUnitNames(const Chunk &tree, const std::map<std::string, StringTable> &tables,
                      std::map<uint32_t, std::string> &units, std::vector<std::string> &errs) {
  std::vector<const Chunk *> forms;
  tree.CollectContainers("UNTS", forms);
  for (const Chunk *f : forms) {
    const Chunk *nams = f->Leaf("NAMS");
    std::map<uint32_t, uint32_t> sidToUnit;
    if (nams && ParseNamsLeaf(nams->body, sidToUnit)) {
      // Unit names only feed the advisory <units> on constants, so an
      // unresolvable table is not fatal.
      std::vector<std::string> ignored;
      ResolveNames(sidToUnit, tables, "units", units, ignored);
    }
  }
}

// The derivable unit of every variable, as XMILE text. Definitions are
// evaluated to a fixpoint; a stock and its flows also inform each other
// through the time unit (flow = stock / time), which is how a stock that
// starts at a bare 0 gets its unit.
std::map<uint32_t, std::string> InferUnits(const std::vector<SipVar> &vars,
                                           const std::map<uint32_t, std::vector<Lexeme>> &lexemes,
                                           const std::set<uint32_t> &flowIds,
                                           const std::map<uint32_t, std::string> &unitNames,
                                           const std::string &timeUnit) {
  Dim time;
  time.exp[timeUnit] = 1;
  std::map<uint32_t, std::vector<uint32_t>> stocksOfFlow;
  for (const SipVar &v : vars) {
    for (const SipFlow &f : v.flows) {
      stocksOfFlow[f.id].push_back(v.id);
    }
  }
  std::map<uint32_t, Dim> known;
  auto strong = [&](uint32_t id) -> const Dim * {
    auto it = known.find(id);
    return it == known.end() || it->second.weak ? nullptr : &it->second;
  };
  // Each round settles at least one more variable or stops, so this bound is
  // never the limit for a consistent model; it only guards a pathological one.
  for (size_t round = 0; round <= vars.size() + 1; ++round) {
    bool changed = false;
    for (const SipVar &v : vars) {
      if (strong(v.id)) {
        continue;
      }
      const std::vector<Lexeme> &lx = lexemes.at(v.id);
      Dim d;
      bool got = UnitEvaluator(lx, known, unitNames, timeUnit).Eval(0, lx.size(), d);
      if (v.isLevel && (!got || d.weak)) {
        for (const SipFlow &f : v.flows) {
          if (const Dim *fd = strong(f.id)) {
            d = Combine(*fd, time, 1);
            got = true;
            break;
          }
        }
      } else if (flowIds.count(v.id) && (!got || d.weak)) {
        for (uint32_t s : stocksOfFlow[v.id]) {
          if (const Dim *sd = strong(s)) {
            d = Combine(*sd, time, -1);
            got = true;
            break;
          }
        }
      }
      if (!got) {
        continue;
      }
      auto it = known.find(v.id);
      if (it == known.end() || it->second.exp != d.exp || it->second.weak != d.weak) {
        known[v.id] = d;
        changed = true;
      }
    }
    if (!changed) {
      break;
    }
  }
  std::map<uint32_t, std::string> out;
  for (const auto &k : known) {
    out[k.first] = FormatDim(k.second);
  }
  return out;
}

// ---------------------------------------------------------------------------
// Diagrams
// ---------------------------------------------------------------------------
//
// Each Components/<c>/\x03Visualizations/<d>/Contents stream is one diagram.
// Its symbols are FORMs in a CAT ITMS, each holding SYM (visible flag, symbol
// id varint, FILETIME) and CNTX (what the symbol shows):
//   VAR  CNTX: variable id. VAR leaf: 9 bytes, length, then the bounding box
//        x0 y0 x1 y1.
//   FLW  CNTX: from and to symbols (0x7F = cloud). FLW leaf: style byte,
//        length, flag byte, flow variable id. CON: the pipe's points.
//   VLV  CNTX: its FLW symbol. VLV leaf: a byte, then the valve's position
//        along the pipe as a big-endian float fraction.
//   LNK  CNTX: from and to symbols. CON: start, Bezier controls, end.
//   NAM  CNTX: the labelled symbol. NAM leaf: 3 bytes, then the label's
//        direction from the symbol as a float angle (radians, y down).
// A flow variable is drawn twice in Powersim -- valve on the pipe and a
// separate rate circle tied to it by a hidden link; XMILE has only the valve.
// Coordinates are varints in thousandths of a point.

struct Pt {
  double x, y;
};

struct ViewVar {
  std::string name;
  bool stock = false;
  bool flow = false;
};

std::string ViewName(const std::string &name) {
  std::string s = name;
  std::replace(s.begin(), s.end(), ' ', '_');
  return XmlEscape(s);
}

float BeFloat(const std::string &b, size_t off) {
  uint32_t bits = Be32(b, off);
  float f;
  memcpy(&f, &bits, sizeof f);
  return f;
}

// CON: 8 bytes, length varint, then a point count and segments of (type,
// flag, point); type 1 is followed by its two Bezier control points.
bool ConPoints(const std::string &b, std::vector<Pt> &pts) {
  size_t i = 8;
  uint32_t len, n;
  if (!ReadVarint(b, i, len) || len > b.size() - i) {
    return false;
  }
  const size_t end = i + len;
  std::string s = b.substr(0, end);
  if (!ReadVarint(s, i, n)) {
    return false;
  }
  while (pts.size() < n) {
    if (i + 2 > end) {
      return false;
    }
    unsigned char type = static_cast<unsigned char>(s[i]);
    i += 2;
    if (type > 1) {
      return false;
    }
    for (int k = 0; k < (type == 1 ? 3 : 1); ++k) {
      uint32_t x, y;
      if (!ReadVarint(s, i, x) || !ReadVarint(s, i, y)) {
        return false;
      }
      pts.push_back({x / 1000.0, y / 1000.0});
    }
  }
  return i == end && pts.size() == n;
}

Pt AlongPolyline(const std::vector<Pt> &pts, double fraction) {
  double total = 0;
  for (size_t k = 1; k < pts.size(); ++k) {
    total += std::hypot(pts[k].x - pts[k - 1].x, pts[k].y - pts[k - 1].y);
  }
  double want = total * std::min(1.0, std::max(0.0, fraction));
  for (size_t k = 1; k < pts.size(); ++k) {
    double seg = std::hypot(pts[k].x - pts[k - 1].x, pts[k].y - pts[k - 1].y);
    if (want <= seg && seg > 0) {
      double t = want / seg;
      return {pts[k - 1].x + t * (pts[k].x - pts[k - 1].x), pts[k - 1].y + t * (pts[k].y - pts[k - 1].y)};
    }
    want -= seg;
  }
  return pts.back();
}

const char *LabelSide(float angle) {
  static const char *const kSides[] = {"right", "bottom", "left", "top"};
  const double quarter = 3.14159265358979323846 / 2;
  long k = std::lround(angle / quarter);
  return kSides[((k % 4) + 4) % 4];
}

struct DiagramSymbols {
  std::map<uint32_t, uint32_t> varOf;  // VAR symbol -> variable id
  std::map<uint32_t, std::vector<uint32_t>> box;
  std::map<uint32_t, uint32_t> flowOf;  // FLW symbol -> flow variable id
  std::map<uint32_t, std::vector<Pt>> pipe;
  std::map<uint32_t, uint32_t> valveFlw;  // VLV symbol -> FLW symbol
  std::map<uint32_t, float> valveAt;
  std::map<uint32_t, float> labelAngle;  // labelled symbol -> angle
  struct Link {
    uint32_t from, to;
    std::vector<Pt> pts;
  };
  std::vector<Link> links;
};

bool ReadDiagramSymbols(const Chunk &itms, DiagramSymbols &d) {
  for (const Chunk &f : itms.kids) {
    const Chunk *sym = f.Leaf("SYM ");
    const Chunk *cntx = f.Leaf("CNTX");
    if (!f.IsContainer() || !sym || !cntx) {
      continue;  // controls, frames, and other decorations
    }
    size_t i = 1;
    uint32_t id, a = 0, b = 0;
    if (sym->body.size() < 2 || !ReadVarint(sym->body, i, id)) {
      return false;
    }
    i = 0;
    if (!ReadVarint(cntx->body, i, a)) {
      return false;
    }
    if (f.type == "VAR ") {
      const Chunk *v = f.Leaf("VAR ");
      size_t j = 9;
      uint32_t len;
      if (!v || !ReadVarint(v->body, j, len) || len > v->body.size() - j) {
        return false;
      }
      std::vector<uint32_t> r(4);
      for (uint32_t &x : r) {
        if (!ReadVarint(v->body, j, x)) {
          return false;
        }
      }
      d.varOf[id] = a;
      d.box[id] = r;
    } else if (f.type == "FLW ") {
      const Chunk *flw = f.Leaf("FLW ");
      const Chunk *con = f.Leaf("CON ");
      size_t j = 3;
      uint32_t var;
      std::vector<Pt> pts;
      if (!flw || flw->body.size() < 4 || !ReadVarint(flw->body, j, var) || !con || !ConPoints(con->body, pts) ||
          pts.size() < 2) {
        return false;
      }
      d.flowOf[id] = var;
      d.pipe[id] = pts;
    } else if (f.type == "VLV ") {
      const Chunk *vlv = f.Leaf("VLV ");
      if (!vlv || vlv->body.size() < 5) {
        return false;
      }
      d.valveFlw[id] = a;
      d.valveAt[id] = BeFloat(vlv->body, 1);
    } else if (f.type == "LNK ") {
      const Chunk *con = f.Leaf("CON ");
      DiagramSymbols::Link l;
      if (!ReadVarint(cntx->body, i, b) || !con || !ConPoints(con->body, l.pts) || l.pts.size() < 2) {
        return false;
      }
      l.from = a;
      l.to = b;
      d.links.push_back(l);
    } else if (f.type == "NAM ") {
      const Chunk *nam = f.Leaf("NAM ");
      if (!nam || nam->body.size() < 7) {
        return false;
      }
      d.labelAngle[a] = BeFloat(nam->body, 3);
    }
  }
  return true;
}

// One <view> per diagram that shows any variables. Each variable is placed
// once, in the first diagram that shows it.
std::string WriteDiagram(const DiagramSymbols &d, const std::map<uint32_t, ViewVar> &vars, std::set<uint32_t> &placed) {
  std::string out;
  // The rate circle of each flow, so its label direction can go to the valve.
  std::map<uint32_t, uint32_t> rateSymbol;
  for (const auto &v : d.varOf) {
    rateSymbol[v.second] = v.first;
  }
  auto label = [&](uint32_t symbol) -> std::string {
    auto it = d.labelAngle.find(symbol);
    return it == d.labelAngle.end() ? "" : std::string(" label_side=\"") + LabelSide(it->second) + "\"";
  };
  for (const auto &v : d.varOf) {
    auto var = vars.find(v.second);
    if (var == vars.end() || var->second.flow || placed.count(v.second)) {
      continue;
    }
    placed.insert(v.second);
    const std::vector<uint32_t> &r = d.box.at(v.first);
    double x0 = r[0] / 1000.0, y0 = r[1] / 1000.0, x1 = r[2] / 1000.0, y1 = r[3] / 1000.0;
    if (var->second.stock) {
      out += "        <stock name=\"" + ViewName(var->second.name) + "\" x=\"" + FormatDouble(x0) + "\" y=\"" +
             FormatDouble(y0) + "\" width=\"" + FormatDouble(x1 - x0) + "\" height=\"" + FormatDouble(y1 - y0) + "\"" +
             label(v.first) + "/>\n";
    } else {
      out += "        <aux name=\"" + ViewName(var->second.name) + "\" x=\"" + FormatDouble((x0 + x1) / 2) + "\" y=\"" +
             FormatDouble((y0 + y1) / 2) + "\"" + label(v.first) + "/>\n";
    }
  }
  std::map<uint32_t, float> valveFraction;  // FLW symbol -> fraction
  for (const auto &v : d.valveFlw) {
    valveFraction[v.second] = d.valveAt.at(v.first);
  }
  for (const auto &f : d.flowOf) {
    auto var = vars.find(f.second);
    if (var == vars.end() || placed.count(f.second)) {
      continue;
    }
    placed.insert(f.second);
    const std::vector<Pt> &pts = d.pipe.at(f.first);
    auto frac = valveFraction.find(f.first);
    Pt valve = AlongPolyline(pts, frac == valveFraction.end() ? 0.5 : frac->second);
    auto rate = rateSymbol.find(f.second);
    out += "        <flow name=\"" + ViewName(var->second.name) + "\" x=\"" + FormatDouble(valve.x) + "\" y=\"" +
           FormatDouble(valve.y) + "\"" + (rate == rateSymbol.end() ? "" : label(rate->second)) +
           ">\n          <pts>\n";
    for (const Pt &p : pts) {
      out += "            <pt x=\"" + FormatDouble(p.x) + "\" y=\"" + FormatDouble(p.y) + "\"/>\n";
    }
    out += "          </pts>\n        </flow>\n";
  }
  // Connector ends: a variable's own symbol, or a flow's pipe or valve.
  auto endpoint = [&](uint32_t symbol) -> const ViewVar * {
    uint32_t var;
    if (d.varOf.count(symbol)) {
      var = d.varOf.at(symbol);
    } else if (d.flowOf.count(symbol)) {
      var = d.flowOf.at(symbol);
    } else if (d.valveFlw.count(symbol) && d.flowOf.count(d.valveFlw.at(symbol))) {
      var = d.flowOf.at(d.valveFlw.at(symbol));
    } else {
      return nullptr;
    }
    auto it = vars.find(var);
    return it == vars.end() ? nullptr : &it->second;
  };
  std::set<std::pair<std::string, std::string>> seen;
  int uid = 1;
  for (const DiagramSymbols::Link &l : d.links) {
    const ViewVar *from = endpoint(l.from);
    const ViewVar *to = endpoint(l.to);
    if (!from || !to || from == to || !seen.insert({from->name, to->name}).second) {
      continue;  // a flow's rate tied to its own valve, or an undecoded end
    }
    // Take-off direction toward the first control point (or the far end of
    // a straight link), as XMILE degrees: counter-clockwise from east, y up.
    const Pt &a = l.pts[0];
    const Pt &b = l.pts[1];
    double angle = std::atan2(-(b.y - a.y), b.x - a.x) * 180.0 / 3.14159265358979323846;
    if (angle < 0) {
      angle += 360;
    }
    out += "        <connector uid=\"" + std::to_string(uid++) + "\" angle=\"" +
           FormatDouble(std::round(angle * 100) / 100) + "\">\n          <from>" + ViewName(from->name) +
           "</from>\n          <to>" + ViewName(to->name) + "</to>\n        </connector>\n";
  }
  return out;
}

// The <views> element for a component, or "" (with a note in errs) when no
// diagram decodes. Geometry is never fatal: the model converts without it.
std::string ReadViews(const CompoundFile &cf, uint32_t component, const std::map<uint32_t, ViewVar> &vars,
                      std::vector<std::string> &errs) {
  std::string views;
  std::set<uint32_t> placed;
  for (uint32_t s : cf.Children(component)) {
    if (cf.Get(s).type != 1 || cf.Get(s).name != "\x03Visualizations") {
      continue;
    }
    for (uint32_t diagram : cf.Children(s)) {
      if (cf.Get(diagram).type != 1) {
        continue;
      }
      uint32_t contents = CompoundFile::kNoStream;
      for (uint32_t c : cf.Children(diagram)) {
        if (cf.Get(c).name == "Contents") {
          contents = c;
        }
      }
      Chunk root;
      std::vector<std::string> ignored;
      if (contents == CompoundFile::kNoStream || !ReadStreamChunks(cf, contents, root, ignored)) {
        errs.push_back("Powersim: a diagram could not be read; it is left out");
        continue;
      }
      std::vector<const Chunk *> itms;
      root.CollectContainers("ITMS", itms);
      DiagramSymbols d;
      if (itms.empty()) {
        continue;  // an empty diagram (or a non-diagram visualization)
      }
      if (!ReadDiagramSymbols(*itms[0], d)) {
        errs.push_back("Powersim: a diagram could not be decoded; it is left out");
        continue;
      }
      std::string body = WriteDiagram(d, vars, placed);
      if (!body.empty()) {
        views += "      <view>\n" + body + "      </view>\n";
      }
    }
  }
  return views.empty() ? "" : "    <views>\n" + views + "    </views>\n";
}

}  // namespace

bool LooksLikeSip(const char *data, size_t len) {
  static const unsigned char kSig[8] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
  return len >= 8 && memcmp(data, kSig, 8) == 0;
}

bool SipToXmile(const char *data, size_t len, std::string &xmile, std::vector<std::string> &errs) {
  CompoundFile cf;
  std::string err;
  if (!cf.Open(data, len, err)) {
    errs.push_back("Powersim: " + err);
    return false;
  }
  std::map<std::string, StringTable> tables;
  if (!LoadStringTables(cf, tables, err)) {
    errs.push_back("Powersim: " + err);
    return false;
  }

  uint32_t comps = cf.Find("Components");
  std::vector<uint32_t> modules;
  uint32_t component = CompoundFile::kNoStream;
  for (uint32_t c : comps == CompoundFile::kNoStream ? std::vector<uint32_t>() : cf.Children(comps)) {
    if (cf.Get(c).type == 1) {
      for (uint32_t s : cf.Children(c)) {
        if (cf.Get(s).name == "MetaModule") {
          modules.push_back(s);
          component = c;
        }
      }
    }
  }
  if (modules.size() != 1) {
    errs.push_back(modules.empty() ? "Powersim: no model found (is this a Powersim Studio project?)"
                                   : "Powersim: projects with several components are not supported yet");
    return false;
  }
  Chunk meta;
  if (!ReadStreamChunks(cf, modules[0], meta, errs)) {
    return false;
  }
  std::vector<const Chunk *> mmod;
  meta.CollectContainers("MMOD", mmod);
  if (mmod.size() != 1) {
    errs.push_back("Powersim: unexpected model layout");
    return false;
  }

  // Variable names: NAMS form -> CHLD items, each a CNTS (string id, var id).
  std::map<uint32_t, uint32_t> sidToVar;
  if (const Chunk *nams = mmod[0]->Container("NAMS")) {
    std::vector<const Chunk *> chld;
    nams->CollectContainers("CHLD", chld);
    for (const Chunk *c : chld) {
      if (const Chunk *cnts = c->Leaf("CNTS")) {
        size_t i = 0;
        uint32_t sid, var;
        if (ReadVarint(cnts->body, i, sid) && ReadVarint(cnts->body, i, var)) {
          sidToVar[sid] = var;
        }
      }
    }
  }
  std::map<uint32_t, std::string> varNames;
  if (!ResolveNames(sidToVar, tables, "variable names", varNames, errs)) {
    return false;
  }

  std::map<uint32_t, std::string> unitNames;
  uint32_t catalog = cf.Find("CatalogContents");
  if (catalog != CompoundFile::kNoStream) {
    Chunk cat;
    if (ReadStreamChunks(cf, catalog, cat, errs)) {
      CollectUnitNames(cat, tables, unitNames, errs);
    }
  }
  CollectUnitNames(*mmod[0], tables, unitNames, errs);

  const Chunk *root = mmod[0]->Container("ROOT");
  std::vector<SipVar> vars;
  if (!root || !ReadVars(*root, vars, errs)) {
    if (!root) {
      errs.push_back("Powersim: the model has no root");
    }
    return false;
  }

  std::set<uint32_t> flowIds;
  for (SipVar &v : vars) {
    auto it = varNames.find(v.id);
    if (it == varNames.end()) {
      errs.push_back("Powersim: a variable has no name");
      return false;
    }
    v.name = it->second;
    for (const SipFlow &f : v.flows) {
      flowIds.insert(f.id);
    }
  }

  std::string specs, timeUnit;
  if (!ReadTimes(cf, specs, timeUnit, errs)) {
    return false;
  }

  bool ok = true;
  std::map<uint32_t, std::vector<Lexeme>> lexemes;
  for (const SipVar &v : vars) {
    if (v.dtxt.empty()) {
      errs.push_back("Powersim: '" + v.name + "' has no definition");
      ok = false;
      continue;
    }
    EquationDecoder dec(varNames, unitNames, v.name, errs);
    if (!dec.Decode(v.dtxt, lexemes[v.id])) {
      ok = false;
    }
  }
  if (!ok) {
    return false;
  }
  std::map<uint32_t, std::string> inferredUnits = InferUnits(vars, lexemes, flowIds, unitNames, timeUnit);

  std::string body;
  for (const SipVar &v : vars) {
    const std::vector<Lexeme> &lx = lexemes[v.id];
    EquationWriter w(lx, varNames, unitNames, timeUnit, v.name, errs);
    std::string eqn, gf;

    std::vector<std::pair<size_t, size_t>> args;
    size_t close = 0;
    if (!lx.empty() && lx[0].kind == Lexeme::kOp && lx[0].code == kOpGraph) {
      // GRAPH(x, x0, dx, {y1, y2, ...}): evenly spaced points from x0.
      double x0, dx;
      if (!w.CallArgs(0, lx.size(), args, close) || close != lx.size() - 1 || args.size() != 4 ||
          !w.Constant(args[1].first, args[1].second, x0) || !w.Constant(args[2].first, args[2].second, dx)) {
        w.Fail("GRAPH needs (input, start, step, {values})");
        ok = false;
        continue;
      }
      size_t yb = args[3].first, ye = args[3].second;
      if (yb >= ye || lx[yb].kind != Lexeme::kOp || lx[yb].code != kOpLBrace || lx[ye - 1].kind != Lexeme::kOp ||
          lx[ye - 1].code != kOpRBrace) {
        w.Fail("GRAPH values must be a {list}");
        ok = false;
        continue;
      }
      std::string ys;
      size_t n = 0, start = yb + 1;
      for (size_t i = yb + 1; i <= ye - 1; ++i) {
        if (i == ye - 1 || lx[i].kind == Lexeme::kComma) {
          double y;
          if (!w.Constant(start, i, y)) {
            w.Fail("GRAPH values must be numbers");
            ok = false;
            break;
          }
          ys += (n++ ? "," : "") + FormatDouble(y);
          start = i + 1;
        }
      }
      if (!ok) {
        continue;
      }
      if (!w.Write(args[0].first, args[0].second, eqn)) {
        ok = false;
        continue;
      }
      gf = "      <gf>\n        <xscale min=\"" + FormatDouble(x0) + "\" max=\"" +
           FormatDouble(x0 + dx * static_cast<double>(n ? n - 1 : 0)) + "\"/>\n        <ypts>" + ys +
           "</ypts>\n      </gf>\n";
    } else if (!w.Write(0, lx.size(), eqn)) {
      ok = false;
      continue;
    }

    const char *tag = v.isLevel ? "stock" : (flowIds.count(v.id) ? "flow" : "aux");
    body += std::string("    <") + tag + " name=\"" + XmlEscape(v.name) + "\">\n      <eqn>" + XmlEscape(eqn) +
            "</eqn>\n" + gf;
    for (const SipFlow &f : v.flows) {
      auto it = varNames.find(f.id);
      if (it == varNames.end()) {
        errs.push_back("Powersim: stock '" + v.name + "' has an unknown flow");
        return false;
      }
      std::string ref = it->second;
      std::replace(ref.begin(), ref.end(), ' ', '_');
      body += std::string("      <") + (f.outflow ? "outflow" : "inflow") + ">" + XmlEscape(ref) + "</" +
              (f.outflow ? "outflow" : "inflow") + ">\n";
    }
    auto units = inferredUnits.find(v.id);
    if (units != inferredUnits.end()) {
      body += "      <units>" + XmlEscape(units->second) + "</units>\n";
    }
    body += std::string("    </") + tag + ">\n";
  }
  if (!ok) {
    return false;
  }

  std::map<uint32_t, ViewVar> viewVars;
  for (const SipVar &v : vars) {
    ViewVar &vv = viewVars[v.id];
    vv.name = v.name;
    vv.stock = v.isLevel;
    vv.flow = !v.isLevel && flowIds.count(v.id);
  }
  std::string views = ReadViews(cf, component, viewVars, errs);

  xmile =
      "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
      "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\">\n"
      "  <header>\n    <vendor>Powersim</vendor>\n    <product version=\"1.0\">xmutil</product>\n  </header>\n" +
      specs + "  <model>\n    <variables>\n" + body + "    </variables>\n" + views + "  </model>\n</xmile>\n";
  return true;
}

}  // namespace powersim
