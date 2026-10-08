// Pins ShortestDouble (src/XMUtil.cpp) to std::to_chars(double) output. The
// function reimplements to_chars because the Intel Mac build targets a macOS
// that predates libc++'s floating-point to_chars; every expected string below
// was produced by libc++'s to_chars, so a drift in the reimplementation (or a
// platform printf/strtod quirk it leans on) shows up here on every platform.

#include <cmath>
#include <string>

#include "../src/XMUtil.h"
#include "TestHarness.h"

TEST(ShortestDouble_matches_to_chars) {
  struct Case {
    double val;
    const char *expected;
  };
  const Case cases[] = {
      {0.1, "0.1"},
      {-2.5, "-2.5"},
      {100, "100"},
      {10000, "10000"},  // fixed and scientific tie at 5 chars; fixed wins
      {1e5, "1e+05"},
      {123456, "123456"},
      {1e21, "1e+21"},
      {1e22, "1e+22"},
      {1e23, "1e+23"},
      {0.001, "0.001"},
      {1e-4, "1e-04"},
      {1e-5, "1e-05"},
      {1.0 / 3, "0.3333333333333333"},
      {0.1 + 0.2, "0.30000000000000004"},
      {5e-324, "5e-324"},
      {1.7976931348623157e308, "1.7976931348623157e+308"},
      // Power of two: the nearest 16-digit decimal does not round-trip, the
      // next one up does.
      {std::ldexp(1.0, -1017), "7.120236347223045e-307"},
      // Fixed notation that would zero-pad the shortest digits prints the exact
      // integer value instead.
      {std::ldexp(1.0, 56) - 8, "72057594037927928"},
      {std::ldexp(1.0, 62) - 1024, "4611686018427386880"},
      {9007199254740993.0, "9007199254740992"},
      {-0.0, "-0"},
      {0.0, "0"},
  };
  for (const Case &c : cases)
    CHECK_EQ_STR(ShortestDouble(c.val), std::string(c.expected));
}
