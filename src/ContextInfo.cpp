#include "ContextInfo.h"

Symbol* ContextInfo::GetLHSSpecific(Symbol* dim)
{
	if (!pLHSElmsGeneric)
		return dim;
	int n = pLHSElmsGeneric->size();
	int i;
	for (i = 0; i < n; i++)
	{
		if ((*pLHSElmsGeneric)[i] == dim)
			return (*pLHSElmsSpecific)[i];
	}
	return dim;
}
