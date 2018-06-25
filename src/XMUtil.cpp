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

#ifdef WITH_UI
#include "UI/Main_Window.h"
#include <QApplication>
#endif

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

bool StringMatch(const std::string& f, const std::string& s)
{
	if (f.size() != s.size())
		return false;
	const char *tv1 = f.c_str();
	const char *tv2 = s.c_str();
	char c1, c2;
	for (; c1 = *tv1; tv1++, tv2++)
	{
		c2 = *tv2;
		if (c1 != c2)
		{
			if (c1 >= 'A' && c1 <= 'Z')
				c1 += ('a' - 'A');
			if (c2 >= 'A' && c2 <= 'Z')
				c2 += ('a' - 'A');
			if (c1 != c2)
				return false;
		}
	}
	return true;
}


bool ParseVensimModel(int argc, char* argv[],Model *m)
{
   if(argc < 2)
      return false ;
   VensimParse vp(m) ;
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
      return -1;
    
   int ret = 0;
   Model *m = new Model() ;
#ifndef WITH_UI
    if(ParseVensimModel(argc,argv,m)) {
        
        /*if(m->AnalyzeEquations()) {
         m->Simulate() ;
         m->OutputComputable(true);
      }*/

	   // mark variable types and potentially convert INTEG equations involving expressions
	   // into flows (a single net flow on the first pass though this)
	   m->MarkVariableTypes();

	   // if there is a view then try to make sure everything is defined in the views
	   // put unknowns in a heap in the first view at 20,20 but for things that have
	   // connections try to put them in the right place
	   m->AttachStragglers();

	   boost::filesystem::path p(argv[1]);
	   p.replace_extension(".xmile");

	   std::vector<std::string> errs;
	   m->WriteToXMILE(p.string(), errs);

        BOOST_FOREACH(const std::string& err, errs)
        {
            std::cout << err << std::endl;
        }
    } else {
        ret = 0;
    }
#else
    QApplication app(argc, argv);
    //QApplication::setWindowIcon(QIcon(":icons/icon.svg"));
    QApplication::setOrganizationName("XMUtil");
    QApplication::setOrganizationDomain("github.com/xmutil");
    QApplication::setApplicationName("MDL to XMILE");
    
    Main_Window window;
    window.show();
    
    ret = app.exec();
#endif
    delete m ;
    CloseUCaseMap() ;
   //CheckMemoryTrack(1) ;


   //printf("Size of symbol is %d\n",sizeof(Symbol)) ;
   //printf("Size of variable is %d\n",sizeof(Variable)) ;
  // _CrtDumpMemoryLeaks() ;

   // if want to look at terminal 

   return ret;
}

double AngleFromPoints(double startx, double starty, double pointx, double pointy, double endx, double endy)
{
	double thetax;
	if (endx > startx)
		thetax = -atan((endy - starty) / (endx - startx)) * 180 / 3.14159265358979;
	else if (endx < startx)
		thetax = 180 - atan((starty - endy) / (startx - endx)) * 180 / 3.14159265358979;
	else if (endy > starty)
		thetax = 270;
	else
		thetax = 90;
	// straight line connector- use this is geometry fails

	// first take the start and end point - the center of the circle is on a line perpindicular
	// to the line between them and intersects it at its midpoint
	double line1x = (startx + endx) / 2;
	double line1y = (starty + endy) / 2;
	double slope1x, slope1y;
	if (startx == endx)
	{
		slope1x = 1;
		slope1y = 0;
	}
	else if (starty == endy)
	{
		slope1x = 0;
		slope1y = 1;
	}
	else
	{
		slope1x = endy - starty; // perpindicular - flip xy
		slope1y = startx - endx; // flip the sign
	}
	// next do point and end - most likely to have good numerics
	double line2x = (pointx + endx) / 2;
	double line2y = (pointy + endy) / 2;
	double slope2x, slope2y;
	if (pointx == endx)
	{
		slope2x = 1;
		slope2y = 0;
	}
	else if (pointy == endy)
	{
		slope2x = 0;
		slope2y = 1;
	}
	else
	{
		slope2x = endy - pointy; // perpindicular - flip xy
		slope2y = pointx - endx; // flip the sign
	}
	/* now we solve for delta1 and delta2 such that
	   line1y + delta1 * slope1y = line2y + delta2 * slope2y
	   line1x + delta1 * slope1x = line2x + delta2 * slope2x
	   */
	double delta1, delta2;
	if (slope1y == 0)
	{
		if (slope2y == 0 || slope1x == 0)
			return thetax;
		delta2 = (line1y - line2y) / slope2y;
		delta1 = (line2x + delta2*slope2x - line1x) / slope1x;
	}
	else if (slope1x == 0)
	{
		if (slope2x == 0)
			return thetax;
		delta2 = (line1x - line2x) / slope2x;
		delta1 = (line2y + delta2*slope2y - line1y) / slope1y;
	}
	else if (slope2y == 0)
	{
		if (slope2x == 0)
			return thetax;
		delta1 = (line2y - line1y) / slope1y;
		delta2 = (line1x + delta1*slope1x - line2x) / slope2x;
	}
	else
	{
		/* now we solve for delta1 and delta2 such that
		line1y + delta1 * slope1y = line2y + delta2 * slope2y
		   -> delta1 = (line2y + delta2 * slope2y - line1y)/slope1y
		line1x + delta1 * slope1x = line2x + delta2 * slope2x
		   -> line1x + (line2y + delta2 * slope2y - line1y)/slope1y * slope1x = line2x + delta2 * slope2x
		   -> line1x + (line2y - line1y)/slope1y * slope1x - line2x =  delta2 * (slope2x - slope1x*slope2y/slope1y)
		   ->
		*/
		if (abs(slope2x - slope1x*slope2y / slope1y) < 1e-8)
			return thetax;
		delta2 = (line1x + (line2y - line1y) / slope1y * slope1x - line2x) / (slope2x - slope1x*slope2y / slope1y);
		delta1 = (line2y + delta2*slope2y - line1y) / slope1y;
	}
	double centerx = line1x + delta1 * slope1x;
	double centery = line1y + delta1 * slope1y;
	assert(line2x + delta2 * slope2x - centerx < 1e-8);
	assert(line2y + delta2 * slope2y - centery < 1e-8);
	// arc tan of slope perpeindicular to center start line
	if (abs(centery - starty) < 1e-6)
	{
		if (pointy > starty)
			return 99;
		return 270;
	}
	if (abs(centerx - startx) < 1e-6)
	{
		if (pointx > startx)
			return 0;
		return 180;
	}
	thetax = atan2(-(starty - centery), (startx - centerx)) * 180 / 3.14159265358979;
	// this needs to go through the point - so add or subtract 90 to get o 
	// find the angle closest to the angle from start to point
	double direct = atan2 ( -(pointy - starty), (pointx - startx)) * 180 / 3.14159265358979;
	double diff1 = direct - (thetax - 90);
	while (diff1 < 0)
		diff1 += 360;
	while (diff1 > 180)
		diff1 -= 360;
	double diff2 = direct - (thetax + 90);
	while (diff2 < 0)
		diff2 += 360;
	while (diff2 > 180)
		diff2 -= 360;
	if (abs(diff1) < abs(diff2))
		thetax -= 90;
	else
		thetax += 90;
	return thetax;


	if (abs(pointx - startx) > abs(pointy - starty))
	{
		if (pointx < startx)
		{
			// need to end up in quadrant 2 or 3
			if (thetax >= 0) // in 1 or 2
				thetax += 90;
			else // in 3 or 4
				thetax -= 90;
		}
		else  // need to end up in quadrant 1 or 4
		{
			if (thetax >= 0) // in 1 or 2
				thetax -= 90;
			else // in 3 or 4
				thetax += 90;
		}
	}
	else
	{
		if (pointy < starty)
		{
			// need to end up in quadrant 3 or 4
			if (thetax >= 0) // in 1 or 2
			{
				if (thetax < 90) //1
					thetax -= 90;
				else
					thetax += 90;
			}
			else // in 3 or 4
			{
				if (thetax < -90)
					thetax -= 90;
				else
					thetax += 90;
			}
		}
		else  // need to end up in quadrant 1 or 2
		{
			if (thetax >= 0) // in 1 or 2
			{
				if (thetax < 90) //1
					thetax += 90;
				else
					thetax -= 90;
			}
			else // in 3 or 4
			{
				if (thetax < -90)
					thetax += 90;
				else
					thetax -= 90;
			}
		}
	}
	return thetax;



	// below is wrong - we need to triangulate to get the center then pull out the tangent at the start point

	// case point between start and end
	double a2 = (pointx - startx)*(pointx - startx) + (pointy - starty)*(pointy - starty);
	double b2 = (pointx - endx)* (pointx - endx) + (pointy - endy)* (pointy - endy);
	double c2 = (startx - endx)* (startx - endx) + (starty - endy)* (starty - endy);
	double x = (c2 + (a2 - b2))/(2*sqrt(c2));
	double y2 = a2 - x*x;
	double theta = atan(sqrt(y2) / x);
	if (!std::isnan(theta))
		return theta*180/3.141592676;
	theta = atan((endy - starty) / (endx - startx));
	if (!std::isnan(theta))
		return theta * 180 / 3.141592676;
	if (endy < starty)
		return 90;
	return 270;
	return 33;
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
      AllocList = NULL ;
      delete a ;
   }
}

#endif
