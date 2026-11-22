// Under --longnames a Vensim variable takes its documentation as its name, in
// the SD capitalization convention: a stock with each word capitalized
// (the rest lower case), everything else all lower case. Variables that keep
// their own name (no usable documentation) are left as written.

#include <cstdlib>
#include <string>

#include "../../src/XMUtil.h"
#include "../TestHarness.h"

namespace {

std::string Take(char *p) {
  std::string s = p ? p : "";
  free(p);
  return s;
}

const char *kModel =
    "{UTF-8}\r\n"
    "CAP=INTEG(INV, 10)\r\n\t~\tWidgets\r\n\t~\tCAPITAL STOCK\r\n\t|\r\n"
    "INV=CAP*FRAC\r\n\t~\tWidgets/Year\r\n\t~\tInvestment RATE\r\n\t|\r\n"
    "DEBT=INTEG(INV*FRAC - DEBT/10, 0)\r\n\t~\tWidgets\r\n\t~\tOutstanding Debt\r\n\t|\r\n"
    "FRAC=0.1\r\n\t~\t1/Year\r\n\t~\tFraction Invested\r\n\t|\r\n"
    "GTCIT=1\r\n\t~\tYear\r\n\t~\t\r\n\t|\r\n"
    "INITIAL TIME=0\r\n\t~\tYear\r\n\t~\t\r\n\t|\r\n"
    "FINAL TIME=10\r\n\t~\tYear\r\n\t~\t\r\n\t|\r\n"
    "TIME STEP=1\r\n\t~\tYear\r\n\t~\t\r\n\t|\r\n"
    "SAVEPER=TIME STEP\r\n\t~\tYear\r\n\t~\t\r\n\t|\r\n"
    "\\\\\\---/// Sketch information - do not modify anything except names\r\n";

}  // namespace

TEST(LongNameCase_stock_words_capitalized_rest_lower) {
  const std::string mdl = kModel;
  std::string xmile =
      Take(convert_mdl_to_xmile(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "c.mdl", false, 1, true));
  CHECK(xmile.find("<stock name=\"Capital_Stock\">") != std::string::npos);
  CHECK(xmile.find("<flow name=\"investment_rate\">") != std::string::npos);
  CHECK(xmile.find("<aux name=\"fraction_invested\">") != std::string::npos);
  CHECK(xmile.find("<eqn>Capital_Stock*fraction_invested</eqn>") != std::string::npos);
  // A flow made up for a stock whose net flow is not a sum of named flows is
  // named after the stock, but lower case like every other flow.
  CHECK(xmile.find("<flow name=\"outstanding_debt net flow\">") != std::string::npos);
  // No documentation: the name stays as written.
  CHECK(xmile.find("<aux name=\"GTCIT\">") != std::string::npos);
}
