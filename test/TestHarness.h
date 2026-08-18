#pragma once
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

struct TestCase {
  std::string name;
  std::function<void()> fn;
};

// Defined in TestHarness.cpp.
std::vector<TestCase> &AllTests();
int RegisterTest(const char *name, std::function<void()> fn);
extern int g_test_failures;          // total failed checks across all tests
extern int g_current_test_failures;  // failed checks in the running test

#define TEST(name)                                    \
  static void name();                                 \
  static int _reg_##name = RegisterTest(#name, name); \
  static void name()

#define CHECK(cond)                                                      \
  do {                                                                   \
    if (!(cond)) {                                                       \
      g_test_failures++;                                                 \
      g_current_test_failures++;                                         \
      printf("  CHECK failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
    }                                                                    \
  } while (0)

#define CHECK_EQ_STR(a, b)                                                                                   \
  do {                                                                                                       \
    std::string _va = (a), _vb = (b);                                                                        \
    if (_va != _vb) {                                                                                        \
      g_test_failures++;                                                                                     \
      g_current_test_failures++;                                                                             \
      printf("  CHECK_EQ failed: \"%s\" != \"%s\" (%s:%d)\n", _va.c_str(), _vb.c_str(), __FILE__, __LINE__); \
    }                                                                                                        \
  } while (0)
