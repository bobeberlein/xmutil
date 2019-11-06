#ifndef _XMUTIL_SYMBOL_NAMESPACE_H
#define _XMUTIL_SYMBOL_NAMESPACE_H
#include <string>
#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>
#include <set>
class Symbol ;
class SymbolTableBase ; // forward declaration 



/* Namespace gives hashed lookup for names 

   to make it case and _ insensitive we convert the incoming name before
   passing it to the lookup functions - altering the hash and equality
   function would (likely) be a bit faster */

#define SNSitToSymbol(it) (it.second)
class SymbolNameSpace
{
public:
   SymbolNameSpace(void);
   ~SymbolNameSpace(void);
   Symbol *Find(const std::string &name) ;
   void Insert(Symbol *sym) ;
   bool Remove(Symbol *sym) ;
   bool Rename(Symbol* sym, const std::string& newname);
   void DeleteAllUnconfirmedAllocations(void) ;
   void ConfirmAllAllocations(void) ;
   void RemoveUnconfirmedAllocation(SymbolTableBase *s) { sUnconfirmedAllocations.erase(s) ; }
   inline void AddUnconfirmedAllocation(SymbolTableBase *s) { sUnconfirmedAllocations.insert(s) ; }
   typedef boost::unordered_map<std::string, Symbol*> HashTable ;
   typedef HashTable::value_type iterator ; // allows iterator type to be used directly with BOOST_FOREACH
   inline HashTable *GetHashTable(void) { return &mHashTable ; } 

private :
   std::string *ToLowerSpace(const std::string &name) ;
   std::set<SymbolTableBase*>sUnconfirmedAllocations ;
   HashTable mHashTable ;
};

#endif