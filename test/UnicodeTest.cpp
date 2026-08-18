// Tests pinning utf8ToLower (src/Unicode.cpp) and its primary consumer
// SymbolNameSpace::ToLowerSpace for names that start with (or contain)
// non-ASCII UTF-8 bytes. The lowercased name is the namespace hash key, so
// any backend bug that mangles or empties such a name silently aliases
// distinct variables onto one key; these tests hold whichever unicode
// backend is linked to the same observable behavior.

#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include "../src/Symbol/SymbolNameSpace.h"
#include "../src/Unicode.h"
#include "../src/XMUtil.h"
#include "TestHarness.h"

namespace {

std::string Lower(const char *s) {
  char *r = utf8ToLower(s, strlen(s));
  std::string out(r);
  delete[] r;
  return out;
}

std::string LowerSpace(const char *s) {
  std::unique_ptr<std::string> r(SymbolNameSpace::ToLowerSpace(s));
  return *r;
}

}  // namespace

TEST(Unicode_ascii_lowercases) {
  CHECK_EQ_STR(Lower("MiXeD Case 42"), "mixed case 42");
}

// "Élevation" -- the leading byte is 0xC3, which reads as negative through a
// signed char; a byte-sign bug in a lowercasing loop turns the whole name
// into "" instead of lowercasing it.
TEST(Unicode_leading_non_ascii_lowercases) {
  CHECK_EQ_STR(Lower("\xC3\x89levation"), "\xC3\xA9levation");
}

TEST(Unicode_interior_non_ascii_lowercases) {
  CHECK_EQ_STR(Lower("El\xC3\x89vation"), "el\xC3\xA9vation");
}

TEST(Unicode_empty_string_ok) {
  CHECK_EQ_STR(Lower(""), "");
}

// Distinct non-ASCII-leading names must produce distinct, non-empty hash
// keys -- if both collapse to "" the second Insert aliases (release) or
// assert-aborts (debug).
TEST(Unicode_tolowerspace_non_ascii_keys_distinct) {
  std::string a = LowerSpace("\xC3\x89levation");  // Élevation
  std::string b = LowerSpace("\xC3\x84rger");      // Ärger
  CHECK(!a.empty());
  CHECK(!b.empty());
  CHECK(a != b);
  CHECK_EQ_STR(a, "\xC3\xA9levation");
}

// End to end: an XMILE model whose variable names start with non-ASCII
// letters converts successfully and keeps both names distinct in the output.
TEST(Unicode_xmile_round_trip_non_ascii_names) {
  const char *kModel =
      "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
      "<xmile version=\"1.0\" xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\">\n"
      "  <sim_specs method=\"euler\"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>\n"
      "  <model><variables>\n"
      "    <aux name=\"\xC3\x89"
      "levation\"><eqn>1</eqn></aux>\n"
      "    <aux name=\"\xC3\x89"
      "cart\"><eqn>2</eqn></aux>\n"
      "  </variables></model>\n"
      "</xmile>\n";
  char *out = convert_xmile_to_xmile(kModel, static_cast<uint32_t>(strlen(kModel)), "unicode.xmile",
                                     /*isLongName=*/-1, /*isAsSectors=*/false);
  CHECK(out != nullptr);
  if (!out)
    return;
  std::string s(out);
  CHECK(s.find("\xC3\x89"
               "levation") != std::string::npos);
  CHECK(s.find("\xC3\x89"
               "cart") != std::string::npos);
  free(out);
}
