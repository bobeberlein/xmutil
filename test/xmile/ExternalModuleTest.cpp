// Stella-isms met reading FRIDA: modules saved as files of their own, {comments}
// inside equations, and a macro whose <eqn> is its output variable's displayed
// name.
//
// A module file is a whole XMILE document; the <module url> that names it is
// either Stella's relative form (`r../Modules/A.itmx`, written as if the
// referring file were a directory) or an absolute path from the machine that
// saved it. Each module file repeats the dimensions it shares with the model.
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include "../../src/XMUtil.h"
#include "../TestHarness.h"

namespace {

namespace fs = std::filesystem;

bool Contains(const std::string &hay, const std::string &needle) {
  return hay.find(needle) != std::string::npos;
}

size_t Count(const std::string &hay, const std::string &needle) {
  size_t n = 0;
  for (size_t at = hay.find(needle); at != std::string::npos; at = hay.find(needle, at + 1))
    n++;
  return n;
}

// A fresh directory of its own per test, so the tests cannot see each other's
// files.
fs::path ScratchDir(const char *name) {
  fs::path dir = fs::temp_directory_path() / "xmutil_test_external_modules" / name;
  std::error_code ec;
  fs::remove_all(dir, ec);
  fs::create_directories(dir);
  return dir;
}

void WriteText(const fs::path &path, const std::string &text) {
  fs::create_directories(path.parent_path());
  std::ofstream(path, std::ios::binary) << text;
}

std::string Convert(const fs::path &path, const std::string &text) {
  char *out = convert_xmile_to_mdl(text.c_str(), static_cast<uint32_t>(text.size()), path.u8string().c_str(), -1);
  const std::string mdl(out ? out : "");
  free(out);
  return mdl;
}

const char *kDims = R"(<dimensions><dim name="D"><elem name="a"/><elem name="b"/></dim></dimensions>)";

std::string BaseModel(const std::string &url) {
  return std::string(R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  )") + kDims +
         R"(
  <model><variables>
    <aux name="y"><dimensions><dim name="D"/></dimensions><eqn>2</eqn></aux>
    <module name="Sector A" url=")" +
         url + R"("/>
  </variables></model>
</xmile>
)";
}

// Module A carries its own copy of D and a sub-module B, saved beside it.
std::string ModuleA() {
  return std::string(R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  )") + kDims +
         R"(
  <model name="Sector_A"><variables>
    <aux name="x"><dimensions><dim name="D"/></dimensions><eqn>5 {was 4 {and 3 before that}}</eqn></aux>
    <module name="Sector B" url="r../B.itmx"/>
  </variables></model>
</xmile>
)";
}

const char *kModuleB = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <model name="Sector_B"><variables>
    <aux name="w"><eqn>7</eqn></aux>
  </variables></model>
</xmile>
)";

void CheckAllThreeModels(const std::string &mdl) {
  CHECK(Contains(mdl, "y[D] = 2"));
  CHECK(Contains(mdl, "x[D] = 5"));
  CHECK(Contains(mdl, "w = 7"));
  // Each file's copy of D is the same dimension, defined once.
  CHECK(Count(mdl, "D: a, b") == 1);
}

}  // namespace

TEST(ExternalModule_relative_url_and_nested_module_file) {
  const fs::path dir = ScratchDir("relative");
  WriteText(dir / "Modules" / "A.itmx", ModuleA());
  WriteText(dir / "Modules" / "B.itmx", kModuleB);
  const std::string mdl = Convert(dir / "Base.stmx", BaseModel("r../Modules/A.itmx"));
  CHECK(!mdl.empty());
  CheckAllThreeModels(mdl);
}

// The url records where the file was when the model was saved; a model moved
// together with its module files still finds them beside the referring file.
TEST(ExternalModule_stale_absolute_url_found_beside_referrer) {
  const fs::path dir = ScratchDir("absolute");
  WriteText(dir / "A.itmx", ModuleA());
  WriteText(dir / "B.itmx", kModuleB);
  const std::string mdl = Convert(dir / "Base.stmx", BaseModel("C:/no/such/place/A.itmx"));
  CHECK(!mdl.empty());
  CheckAllThreeModels(mdl);
}

TEST(ExternalModule_missing_file_fails_naming_it) {
  const fs::path dir = ScratchDir("missing");
  const std::string text = BaseModel("r../Modules/A.itmx");
  char *out = convert_xmile_to_mdl(text.c_str(), static_cast<uint32_t>(text.size()),
                                   (dir / "Base.stmx").u8string().c_str(), -1);
  CHECK(out == nullptr);
  free(out);
}

// Stella names the macro's output as it is displayed, spaces and all, which is
// no equation; it is the body variable of that name.
TEST(ExternalModule_macro_eqn_names_its_output_variable) {
  const std::string text = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <macro name="TWICE">
    <eqn>twice the input</eqn>
    <parm>input</parm>
    <variables><aux name="twice the input"><eqn>input*2</eqn></aux></variables>
  </macro>
  <model><variables><aux name="y"><eqn>TWICE(3)</eqn></aux></variables></model>
</xmile>
)";
  const std::string mdl = Convert("m.xmile", text);
  CHECK(Contains(mdl, "TWICE = twice the input"));
  CHECK(Contains(mdl, "twice the input = input * 2"));
}
