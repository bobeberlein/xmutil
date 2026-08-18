#include "TestHarness.h"

#include "../src/Unicode.h"

int g_test_failures = 0;
int g_current_test_failures = 0;

std::vector<TestCase> &AllTests() {
  static std::vector<TestCase> tests;
  return tests;
}

int RegisterTest(const char *name, std::function<void()> fn) {
  AllTests().push_back({name, fn});
  return 0;
}

int main() {
  // The ICU-backed utf8ToLower requires the global case map opened by
  // OpenUnicode; Main.cpp does this for the CLI, the harness must do it here.
  if (!OpenUnicode()) {
    fprintf(stderr, "error: could not initialize unicode state\n");
    return 1;
  }
  int failed_tests = 0;
  for (const TestCase &t : AllTests()) {
    g_current_test_failures = 0;
    printf("[ RUN  ] %s\n", t.name.c_str());
    t.fn();
    if (g_current_test_failures == 0) {
      printf("[ PASS ] %s\n", t.name.c_str());
    } else {
      printf("[ FAIL ] %s (%d checks failed)\n", t.name.c_str(), g_current_test_failures);
      failed_tests++;
    }
  }
  printf("\n%zu tests, %d failed (%d total check failures)\n", AllTests().size(), failed_tests, g_test_failures);
  CloseUnicode();
  return g_test_failures == 0 ? 0 : 1;
}
