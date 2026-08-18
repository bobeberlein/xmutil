#ifndef _XMUTIL_XMILE_XMILEFUNCTIONS_H
#define _XMUTIL_XMILE_XMILEFUNCTIONS_H

#include <string>

class ExpressionList;
class Function;
class SymbolNameSpace;
class Variable;

// Free functions that translate XMILE-canonical names into the Vensim-canonical
// names xmutil's engine speaks. Both tables mirror the un-rename rules in the
// simlin writer (third_party/simlin/src/simlin-engine/src/mdl/writer.rs around
// the mdl_bare_keyword / xmile_to_mdl_function_name / reorder_args helpers),
// which are the authoritative list. These helpers are pure lookups: they do not
// own state and do not touch global parser objects.
namespace xmile {

// Resolve an XMILE function name (lowercased, possibly with underscores) to the
// Function* registered in the namespace under its Vensim-canonical name.
// argCount disambiguates the safediv case (2 -> ZIDZ, 3 -> XIDZ); for every
// other name argCount is informational and ignored. Returns NULL if neither the
// explicit table nor the underbar_to_space + uppercase fallback resolves to a
// registered Function.
Function *LookupFunction(SymbolNameSpace *sns, const std::string &xmileName, int argCount);

// Resolve an XMILE bare keyword (time, dt, initial_time, ...) to a Variable*
// in the namespace, creating it if absent. Returns NULL if the name is not one
// of the recognized bare keywords.
Variable *LookupBareKeyword(SymbolNameSpace *sns, const std::string &xmileName);

// Reorder an argument list from XMILE order to Vensim order for the small set
// of functions whose XMILE and Vensim signatures disagree (DELAY N, SMOOTH N,
// RANDOM NORMAL). vensimName is the post-un-rename canonical name. The list is
// mutated in place; the operation is a no-op for any other name.
void ReorderArgs(const std::string &vensimName, ExpressionList *args);

}  // namespace xmile

#endif
