// XMILE's `pi` is a reserved constant UNTIL a document declares a variable by
// that name.
//
// Vensim has no PI builtin, so the reader lowers `pi` / `pi()` to a numeric
// literal at read time. Doing that unconditionally meant a document declaring
// its own `pi` still emitted `pi = 7` but rewrote every REFERENCE to 3.14159 --
// the declared variable survived with nothing pointing at it, and every
// dependent equation computed a different number than the source model. No
// diagnostic, exit status 0.
//
// The declaration is a property of the whole document, so the answer must not
// depend on whether the declaration precedes the reference: XMILE fixes no
// order between them, and the ordered pair of tests below is what pins that.

#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

namespace {

const char *kPiDeclaredBeforeUse = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="pi"><eqn>7</eqn></aux>
    <aux name="area"><eqn>pi * 2</eqn></aux>
  </variables></model>
</xmile>
)";

const char *kPiDeclaredAfterUse = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="area"><eqn>pi * 2</eqn></aux>
    <aux name="pi"><eqn>7</eqn></aux>
  </variables></model>
</xmile>
)";

// No declaration anywhere: `pi` is the XMILE constant and must still lower to
// the full-precision literal.
const char *kPiUndeclared = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="area"><eqn>pi * 2</eqn></aux>
  </variables></model>
</xmile>
)";

const char *kPiCallUndeclared = R"(<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="area"><eqn>pi() * 2</eqn></aux>
  </variables></model>
</xmile>
)";

std::string EmitMDL(const char *xmile) {
  std::vector<std::string> errs;
  Model *m = xmileroundtrip::ParseXMILE(xmile, errs);
  if (!m)
    return std::string();
  std::string out = m->PrintMDL(errs);
  delete m;
  return out;
}

// `area`'s equation must name the variable, not carry the constant inline.
void CheckAreaReferencesDeclaredPi(const char *xmile) {
  std::string mdl = EmitMDL(xmile);
  CHECK(!mdl.empty());
  CHECK(mdl.find("area = pi * 2") != std::string::npos);
  CHECK(mdl.find("3.14159") == std::string::npos);
  if (mdl.find("area = pi * 2") == std::string::npos)
    printf("%s\n", mdl.c_str());
}

}  // namespace

TEST(PiKeyword_declared_variable_wins_when_declared_first) {
  CheckAreaReferencesDeclaredPi(kPiDeclaredBeforeUse);
}

TEST(PiKeyword_declared_variable_wins_when_referenced_first) {
  CheckAreaReferencesDeclaredPi(kPiDeclaredAfterUse);
}

TEST(PiKeyword_undeclared_bare_pi_is_still_the_constant) {
  std::string mdl = EmitMDL(kPiUndeclared);
  CHECK(!mdl.empty());
  CHECK(mdl.find("3.141592653589793") != std::string::npos);
}

TEST(PiKeyword_undeclared_pi_call_is_still_the_constant) {
  std::string mdl = EmitMDL(kPiCallUndeclared);
  CHECK(!mdl.empty());
  CHECK(mdl.find("3.141592653589793") != std::string::npos);
}
