#include "XmileFunctions.h"

#include <algorithm>
#include <cctype>

#include "../Function/Function.h"
#include "../Symbol/ExpressionList.h"
#include "../Symbol/Symbol.h"
#include "../Symbol/SymbolNameSpace.h"
#include "../Symbol/Variable.h"

namespace xmile {

namespace {

struct FuncMapping {
  const char *xmile;
  const char *vensim;
};

// XMILE spellings that resolve to different Vensim builtins depending on the
// call-site argument count. A name that appears here but is called with an
// arity no row covers resolves to nullptr -- it must NOT fall through to
// kFuncMap or the uppercase fallback, because the call is wrong-arity for a
// known function, not an unknown name.
struct ArityMapping {
  const char *xmile;
  int argCount;
  const char *vensim;
};

constexpr ArityMapping kArityMap[] = {
    // The writer always emits ZIDZ for safediv, but xmutil has both ZIDZ
    // (2-arg) and XIDZ (3-arg) registered, so the reader picks based on the
    // call site.
    {"safediv", 2, "ZIDZ"},
    {"safediv", 3, "XIDZ"},
    // MIN/MAX: XMILE spells both the scalar pairwise form and the array-
    // reduction form "min"/"max" (XMILE min/max accept 1 or 2 args), but
    // Vensim splits them into VMIN/VMAX (the 1-arg array reducer) and MIN/MAX
    // (the 2-arg scalar pair), so a one-argument min(arr) becomes VMIN rather
    // than a wrong-arity MIN.
    {"min", 1, "VMIN"},
    {"min", 2, "MIN"},
    {"max", 1, "VMAX"},
    {"max", 2, "MAX"},
    // The one-argument XMILE form INIT(x) is the initial-value function
    // (Vensim INITIAL, whose alternate name is "INIT", so the writer
    // round-trips it back to INIT(x)); the rarer two-argument form
    // INIT(active, initial) is Vensim ACTIVE INITIAL.
    {"init", 1, "INITIAL"},
    {"init", 2, "ACTIVE INITIAL"},
    // The first-order/third-order smooth and delay builtins each share one
    // XMILE spelling between the plain form and the "...I" (explicit-initial)
    // form -- Vensim's SMOOTH/SMOOTHI, SMOOTH3/SMOOTH3I, DELAY1/DELAY1I,
    // DELAY3/DELAY3I all un-rename to the same alternate name, so the writer
    // emits one spelling and the reader disambiguates by arity: the
    // two-argument call is the plain form, the three-argument call carries
    // the initial value.
    {"smth1", 2, "SMOOTH"},
    {"smth1", 3, "SMOOTHI"},
    {"smth3", 2, "SMOOTH3"},
    {"smth3", 3, "SMOOTH3I"},
    {"delay1", 2, "DELAY1"},
    {"delay1", 3, "DELAY1I"},
    {"delay3", 2, "DELAY3"},
    {"delay3", 3, "DELAY3I"},
};

// Mirrors third_party/simlin/.../mdl/writer.rs's xmile_to_mdl_function_name.
// Names that dispatch on arity (safediv, min, max, init, smth1, ...) live in
// kArityMap above instead.
constexpr FuncMapping kFuncMap[] = {
    {"delay", "DELAY FIXED"},
    {"delayn", "DELAY N"},
    {"smthn", "SMOOTH N"},
    {"int", "INTEGER"},
    // SIZE is the XMILE spelling (and xmutil's ComputableName) for the element-
    // count builtin registered under "ELMCOUNT"; without this entry the fallback
    // (underbar_to_space + uppercase -> "SIZE") misses the registry and the
    // single-argument call fell through to a phantom lookup on an undefined
    // "SIZE" variable, silently turning SIZE(dim) into a lookup miss.
    {"size", "ELMCOUNT"},
    {"lookupinv", "LOOKUP INVERT"},
    {"uniform", "RANDOM UNIFORM"},
    {"forcst", "FORECAST"},
    {"normalpink", "RANDOM PINK NOISE"},
    {"normal", "RANDOM NORMAL"},
    {"lookup", "LOOKUP"},
    {"integ", "INTEG"},
};

struct KeywordMapping {
  const char *xmile;
  const char *vensimVar;
};

// Mirrors third_party/simlin/.../mdl/writer.rs's mdl_bare_keyword.
constexpr KeywordMapping kKeywordMap[] = {
    {"time", "Time"},
    {"dt", "TIME STEP"},
    {"time_step", "TIME STEP"},
    {"starttime", "INITIAL TIME"},
    {"initial_time", "INITIAL TIME"},
    {"endtime", "FINAL TIME"},
    {"stoptime", "FINAL TIME"},
    {"final_time", "FINAL TIME"},
};

std::string ToLower(const std::string &in) {
  std::string out(in);
  std::transform(out.begin(), out.end(), out.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return out;
}

// Apply the fallback transform: replace underscores with spaces and uppercase.
// This is the writer.rs `_ => underbar_to_space(name).to_uppercase()` arm; any
// XMILE name not in kFuncMap goes through it before the namespace lookup.
std::string UnderbarToSpaceUpper(const std::string &in) {
  std::string out;
  out.reserve(in.size());
  for (char ch : in) {
    unsigned char uc = static_cast<unsigned char>(ch);
    if (uc == '_')
      out.push_back(' ');
    else
      out.push_back(static_cast<char>(std::toupper(uc)));
  }
  return out;
}

Function *FindFunction(SymbolNameSpace *sns, const std::string &vensimName) {
  if (!sns)
    return nullptr;
  Symbol *sym = sns->Find(vensimName);
  if (!sym)
    return nullptr;
  if (sym->isType() != Symtype_Function)
    return nullptr;
  return static_cast<Function *>(sym);
}

}  // namespace

Variable *LookupBareKeyword(SymbolNameSpace *sns, const std::string &xmileName) {
  if (!sns)
    return nullptr;
  const std::string lowered = ToLower(xmileName);
  for (const KeywordMapping &kw : kKeywordMap) {
    if (lowered != kw.xmile)
      continue;
    Symbol *existing = sns->Find(kw.vensimVar);
    if (existing) {
      if (existing->isType() == Symtype_Variable)
        return static_cast<Variable *>(existing);
      // A non-Variable name collision (e.g. somebody registered a Function
      // under "Time") makes the keyword unresolvable; surface as NULL rather
      // than silently producing a typed mismatch.
      return nullptr;
    }
    // The Variable constructor registers itself in the namespace via
    // Symbol::Symbol(sns, name), so the next Find here will hit.
    return new Variable(sns, kw.vensimVar);
  }
  return nullptr;
}

Function *LookupFunction(SymbolNameSpace *sns, const std::string &xmileName, int argCount) {
  if (!sns)
    return nullptr;
  const std::string lowered = ToLower(xmileName);
  bool nameMatched = false;
  for (const ArityMapping &am : kArityMap) {
    if (lowered != am.xmile)
      continue;
    if (argCount == am.argCount)
      return FindFunction(sns, am.vensim);
    nameMatched = true;
  }
  if (nameMatched)
    return nullptr;
  for (const FuncMapping &fm : kFuncMap) {
    if (lowered == fm.xmile)
      return FindFunction(sns, fm.vensim);
  }
  return FindFunction(sns, UnderbarToSpaceUpper(lowered));
}

void ReorderArgs(const std::string &vensimName, ExpressionList *args) {
  if (!args)
    return;
  if (vensimName == "DELAY N" || vensimName == "SMOOTH N") {
    // XMILE: (input, dt, n, init) -> Vensim: (input, dt, init, n).
    // The two grammars agree on positions 0 and 1, so we only need to swap
    // the last two slots when the call actually provides them.
    if (args->Length() >= 4) {
      Expression *a2 = args->GetExp(2);
      Expression *a3 = args->GetExp(3);
      args->SetExp(2, a3);
      args->SetExp(3, a2);
    }
    return;
  }
  if (vensimName == "RANDOM NORMAL") {
    // XMILE: (mean, sd, seed, min, max) -> Vensim: (min, max, mean, sd, seed).
    if (args->Length() >= 5) {
      Expression *mean = args->GetExp(0);
      Expression *sd = args->GetExp(1);
      Expression *seed = args->GetExp(2);
      Expression *minv = args->GetExp(3);
      Expression *maxv = args->GetExp(4);
      args->SetExp(0, minv);
      args->SetExp(1, maxv);
      args->SetExp(2, mean);
      args->SetExp(3, sd);
      args->SetExp(4, seed);
    }
    return;
  }
}

}  // namespace xmile
