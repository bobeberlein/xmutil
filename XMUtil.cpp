// XMUtil.cpp : Defines the entry point for the console application.
//
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include "Vensim/VensimParse.h"
#include "unicode/utypes.h"
#include "unicode/ustring.h"
#include "unicode/ucasemap.h"
#include "Model.h"
#include "XMUtil.h"

UCaseMap *GlobalUCaseMap ;
bool OpenUCaseMap(void)
{

   UErrorCode ec = U_ZERO_ERROR  ;
   GlobalUCaseMap = ucasemap_open("en",0,&ec) ;
   if(!GlobalUCaseMap)
      return false ;
   return true ;
}
void CloseUCaseMap(void)
{
   ucasemap_close(GlobalUCaseMap) ;
}

std::string StringFromDouble(double val)
{
	char buf[128];
	sprintf(buf, "%g", val);
	return std::string(buf);
}

std::string SpaceToUnderBar(const std::string& s)
{
	std::string rval;
	for (const char *tv = s.c_str(); *tv; tv++)
	{
		if (*tv == ' ')
			rval.push_back('_');
		else
			rval.push_back(*tv);
	}
	return rval;
}


bool ParseVensimModel(int argc, char* argv[],Model *m)
{
   if(argc < 2)
      return false ;
   VensimParse vp(m->GetNameSpace()) ;
   if(!vp.ProcessFile(argv[1])) 
      return false ;
	return true ;
}

#ifdef _DEBUG
void CheckMemoryTrack(int clear) ;
#endif

int main(int argc, char* argv[])
{
   if(!OpenUCaseMap())
      return -1 ;

   Model *m = new Model() ;
   if(ParseVensimModel(argc,argv,m)) {
      /*if(m->AnalyzeEquations()) {
         m->Simulate() ;
         m->OutputComputable(true);
      }*/

	   // mark variable types and potentially convert INTEG equations involving expressions
	   // into flows (a single net flow on the first pass though this)
	   m->MarkVariableTypes();



	   boost::filesystem::path p(argv[1]);
	   p.replace_extension(".xmile");

	   std::vector<std::string> errs;
	   m->WriteToXMILE(p.string(), errs);

	   BOOST_FOREACH(const std::string& err, errs)
	   {
		   std::cout << err << std::endl;
	   }
   }
   delete m ;
   CloseUCaseMap() ;
   //CheckMemoryTrack(1) ;


   //printf("Size of symbol is %d\n",sizeof(Symbol)) ;
   //printf("Size of variable is %d\n",sizeof(Variable)) ;
  // _CrtDumpMemoryLeaks() ;

   // if want to look at terminal std::cin.get() ;
   return 0 ;
}


#if defined(_DEBUG) && defined(wantownmemorytesting)
#include <boost/unordered_map.hpp>
#include <assert.h>
#undef new // regular new used in this section
#undef delete // same for delete

typedef struct {
   size_t size ;
   int line_no ;
   char file[32] ;
} AllocInfo ;

typedef boost::unordered_map<void*, AllocInfo> MemTrackMap ;

MemTrackMap *AllocList = 0 ;

void AddTrack(void *addr,  size_t size,  const char *fname, int lnum)
{
   if(!AllocList)
      AllocList = new MemTrackMap() ;
   AllocInfo ai ;
   ai.size = size ;
   ai.line_no = lnum ;
   if(strlen(fname) > 31) 
      strcpy(ai.file,fname+strlen(fname)-31) ;
   else
      strcpy(ai.file,fname) ;
   (*AllocList)[addr] = ai ;
};

static int Uk =0 ;
void RemoveTrack(void *addr)
{
   if(AllocList) {
      MemTrackMap::iterator node = AllocList->find(addr) ;
      if(node != AllocList->end()) {
         AllocList->erase(node) ;
         return ;
      }
   }
   //printf("%x %d\n",addr,++Uk) ;
   // ignore things that may have been allocated elsewhere - boost is not controllable
}

void CheckMemoryTrack(int clear)
{
   if(!AllocList)
      return ;
   MemTrackMap::iterator node = AllocList->begin() ;
   for(;node != AllocList->end();node++) {
      fprintf(stderr,"Uncleared Memory at %u size %d from %s(%d)\n",node->first,node->second.size,node->second.file,node->second.line_no) ;
   }
   if(clear) {
      MemTrackMap *a = AllocList ;
      AllocList = '\0' ;
      delete a ;
   }
}

#endif