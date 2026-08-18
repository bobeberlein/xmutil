#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include "../../src/Mdl/MDLGenerator.h"
#include "../../src/Model.h"
#include "../../src/Symbol/Variable.h"
#include "../../src/Vensim/VensimView.h"
#include "../../src/XMUtil.h"
#include "../TestHarness.h"
#include "ModelComparator.h"
#include "RoundTrip.h"

#ifdef _WIN32
#include <io.h>
#define XMUTIL_TEST_DUP _dup
#define XMUTIL_TEST_DUP2 _dup2
#define XMUTIL_TEST_CLOSE _close
#define XMUTIL_TEST_FILENO _fileno
#else
#include <unistd.h>
#define XMUTIL_TEST_DUP dup
#define XMUTIL_TEST_DUP2 dup2
#define XMUTIL_TEST_CLOSE close
#define XMUTIL_TEST_FILENO fileno
#endif

namespace {

using roundtrip::ExpectCleanRoundTrip;

// The element-bearing VensimView of a model, or nullptr if the model has no
// non-empty view. Mirrors the comparator's NonEmptyViews filter: the writer
// always emits a minimal empty *View frame, so a model that genuinely has no
// geometry parses to a single zero-element view that this helper treats as
// "no view" -- distinguishing a real sketch from the writer's framing.
VensimView *FirstNonEmptyView(Model *m) {
  for (View *v : m->Views()) {
    VensimView *vv = static_cast<VensimView *>(v);
    if (!vv->Elements().empty())
      return vv;
  }
  return nullptr;
}

// Count the polarity characters present across a view's connector elements,
// returning how many carry '+' and how many carry '-'. Used to prove the
// polarity assertions are non-vacuous: a fixture that lacks polarized connectors
// would make a "polarity survives" assertion pass trivially, so each test first
// confirms the ORIGINAL model actually has both signs before asserting they
// survive the round trip.
struct PolarityCounts {
  int plus = 0;
  int minus = 0;
};

PolarityCounts CountPolarities(VensimView *view) {
  PolarityCounts counts;
  for (VensimViewElement *e : view->Elements()) {
    if (!e || e->Type() != VensimViewElement::ElementTypeCONNECTOR)
      continue;
    char p = static_cast<VensimConnectorElement *>(e)->Polarity();
    if (p == '+')
      counts.plus++;
    else if (p == '-')
      counts.minus++;
  }
  return counts;
}

// The uid-ordered "<uid>:<type>@<x>,<y>,<w>,<h>" signature of a view's non-NULL
// elements. The view-title tests compare this before and after the round trip to
// prove the SKETCH ITSELF survives a hostile title, not merely that the file
// re-parses: the header is positional, so a title that splits into two physical
// lines pushes the font/cosmetic line into the first record slot and every
// following record is misread -- which shows up here as changed geometry (or an
// empty element list), not as a parse failure.
std::vector<std::string> ElementSignature(VensimView *view) {
  std::vector<std::string> sig;
  size_t uid = 0;
  for (VensimViewElement *e : view->Elements()) {
    if (e) {
      std::ostringstream os;
      os << uid << ":" << static_cast<int>(e->Type()) << "@" << e->X() << "," << e->Y() << "," << e->Width() << ","
         << e->Height();
      sig.push_back(os.str());
    }
    uid++;
  }
  return sig;
}

// Split .mdl text into lines on LF, dropping a trailing CR so CRLF output can be
// inspected line by line.
std::vector<std::string> SplitLines(const std::string &text) {
  std::vector<std::string> lines;
  std::string cur;
  for (char c : text) {
    if (c == '\n') {
      if (!cur.empty() && cur.back() == '\r')
        cur.pop_back();
      lines.push_back(cur);
      cur.clear();
    } else {
      cur += c;
    }
  }
  if (!cur.empty()) {
    if (cur.back() == '\r')
      cur.pop_back();
    lines.push_back(cur);
  }
  return lines;
}

// The three header lines that follow the first sketch opener marker: the version
// line, the *Title line, and the font/cosmetic line -- in that fixed order, which
// is exactly what a multi-line title destroys. Returns fewer than three entries
// if the opener is missing or the frame is truncated.
std::vector<std::string> SketchHeaderLines(const std::string &mdl) {
  std::vector<std::string> lines = SplitLines(mdl);
  const std::string opener = "\\\\\\---///";
  for (size_t i = 0; i < lines.size(); i++) {
    if (lines[i].compare(0, opener.size(), opener) != 0)
      continue;
    std::vector<std::string> header;
    for (size_t j = i + 1; j < lines.size() && header.size() < 3; j++)
      header.push_back(lines[j]);
    return header;
  }
  return {};
}

// Parse `mdl` and force the first element-bearing view's title to `title`. A
// title hostile to the sketch header cannot be expressed in a .mdl SOURCE file --
// the reader's ReadLine ends the *Title line at the first break, so the bytes
// after it were never part of the title -- so it is injected on the parsed model.
// That is precisely the state XmileReader::ProcessViews leaves behind for an
// XMILE <view name="Main&#10;View|x">, which is where such titles actually come
// from. Returns a model the caller owns, or nullptr.
Model *ParseWithViewTitle(const std::string &mdl, const std::string &title) {
  Model *m = roundtrip::ParseVensim(mdl);
  if (!m)
    return nullptr;
  VensimView *v = FirstNonEmptyView(m);
  if (!v) {
    delete m;
    return nullptr;
  }
  v->SetTitle(title);
  return m;
}

// Count the ghost variable elements in a view. Ghost (alias) variable boxes are
// a distinct sketch feature -- the same variable can appear ghosted in another
// view region -- so a fixture exercising ghost round-trip must actually contain
// some, which this lets a test assert before relying on the comparator's
// ghost-state check.
int CountGhosts(VensimView *view) {
  int ghosts = 0;
  for (VensimViewElement *e : view->Elements()) {
    if (!e || e->Type() != VensimViewElement::ElementTypeVARIABLE)
      continue;
    if (static_cast<VensimVariableElement *>(e)->Ghost(nullptr, false))
      ghosts++;
  }
  return ghosts;
}

// Everything log() wrote to stderr while `body` ran.
//
// log() (src/Log.cpp) writes straight to the stderr FILE*, so a warning is only
// observable from a test by pointing that stream somewhere readable for the
// duration. The descriptor is saved with dup/dup2 rather than reopened by name
// because stderr may be a pipe, which has no path to reopen. On any failure to
// set the capture up, the body still runs and the capture comes back empty --
// which fails the assertion rather than skipping it.
std::string CaptureStderr(const std::function<void()> &body) {
  std::string captured;
  FILE *tmp = tmpfile();
  if (!tmp) {
    body();
    return captured;
  }
  fflush(stderr);
  int saved = XMUTIL_TEST_DUP(XMUTIL_TEST_FILENO(stderr));
  XMUTIL_TEST_DUP2(XMUTIL_TEST_FILENO(tmp), XMUTIL_TEST_FILENO(stderr));
  try {
    body();
  } catch (...) {
    // Restoring stderr matters more than this exception: leaving the capture in
    // place would swallow every later test's diagnostics. Re-thrown below.
    fflush(stderr);
    XMUTIL_TEST_DUP2(saved, XMUTIL_TEST_FILENO(stderr));
    XMUTIL_TEST_CLOSE(saved);
    fclose(tmp);
    throw;
  }
  fflush(stderr);
  XMUTIL_TEST_DUP2(saved, XMUTIL_TEST_FILENO(stderr));
  XMUTIL_TEST_CLOSE(saved);
  rewind(tmp);
  char buf[4096];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), tmp)) > 0)
    captured.append(buf, n);
  fclose(tmp);
  return captured;
}

// The number of non-overlapping occurrences of `needle` in `haystack`.
size_t CountOccurrences(const std::string &haystack, const std::string &needle) {
  size_t count = 0;
  for (size_t pos = haystack.find(needle); pos != std::string::npos; pos = haystack.find(needle, pos + needle.size()))
    count++;
  return count;
}

// The three properties an emitted sketch must have, checked against the model a
// re-read of that sketch produces. Each returns a description of the first
// violation, or "" when the property holds.
//
// They are checked on the RE-PARSED side deliberately: every one of them is a
// statement about what the emitted bytes mean to a reader, and the reader is the
// only thing that can answer that. A record naming a variable the equation
// section never defined is exactly the case where FindVariable comes back empty
// and VensimVariableElement::_variable is left NULL, which is also the "Can't
// find - <name>" diagnostic the Vensim reader prints.
std::string EveryVariableRecordResolves(VensimView *view) {
  size_t uid = 0;
  for (VensimViewElement *e : view->Elements()) {
    if (e && e->Type() == VensimViewElement::ElementTypeVARIABLE && !e->GetVariable()) {
      std::ostringstream os;
      os << "sketch record at uid " << uid << " names a variable the equation section does not define";
      return os.str();
    }
    uid++;
  }
  return "";
}

std::string EveryConnectorEndpointExists(VensimView *view) {
  VensimViewElements &elems = view->Elements();
  size_t uid = 0;
  for (VensimViewElement *e : elems) {
    if (e && e->Type() == VensimViewElement::ElementTypeCONNECTOR) {
      VensimConnectorElement *ce = static_cast<VensimConnectorElement *>(e);
      const int endpoints[2] = {ce->From(), ce->To()};
      for (int endpoint : endpoints) {
        if (endpoint < 0 || static_cast<size_t>(endpoint) >= elems.size() || !elems[endpoint]) {
          std::ostringstream os;
          os << "connector at uid " << uid << " names uid " << endpoint << ", which has no record";
          return os.str();
        }
      }
    }
    uid++;
  }
  return "";
}

std::string EveryAttachedValveIsFollowedByItsFlow(VensimView *view) {
  VensimViewElements &elems = view->Elements();
  for (size_t uid = 0; uid < elems.size(); uid++) {
    VensimViewElement *e = elems[uid];
    if (!e || e->Type() != VensimViewElement::ElementTypeVALVE)
      continue;
    if (!static_cast<VensimValveElement *>(e)->Attached())
      continue;
    const size_t flow = uid + 1;
    if (flow >= elems.size() || !elems[flow] || elems[flow]->Type() != VensimViewElement::ElementTypeVARIABLE) {
      std::ostringstream os;
      os << "attached valve at uid " << uid << " is not followed by a variable record";
      return os.str();
    }
  }
  return "";
}

// Parse `mdl`, emit it, re-parse the emission, and assert the three sketch
// invariants on the result. `label` names the case in any failure message.
// Returns the emitted text so the caller can make further assertions on it.
std::string ExpectSelfConsistentSketch(const std::string &mdl, const char *label) {
  Model *m0 = roundtrip::ParseVensim(mdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return "";
  std::vector<std::string> errs;
  std::string emitted = m0->PrintMDL(errs);
  delete m0;
  CHECK(errs.empty());
  CHECK(!emitted.empty());
  if (emitted.empty())
    return "";

  Model *m1 = roundtrip::ParseVensim(emitted);
  CHECK(m1 != nullptr);
  if (!m1)
    return emitted;
  // The writer always emits a frame, so there is always a view to check; an
  // element-free frame satisfies all three properties vacuously, which is why
  // the callers below also assert on what SURVIVED.
  for (View *v : m1->Views()) {
    VensimView *vv = static_cast<VensimView *>(v);
    const std::string bad[3] = {EveryVariableRecordResolves(vv), EveryConnectorEndpointExists(vv),
                                EveryAttachedValveIsFollowedByItsFlow(vv)};
    for (const std::string &b : bad) {
      if (!b.empty())
        printf("  [%s] %s\n", label, b.c_str());
      CHECK(b.empty());
    }
  }
  delete m1;
  return emitted;
}

// The "<type>,<uid>" prefix of every sketch record between the frame opener and
// the terminator, so a test can state exactly which records survived.
std::vector<std::string> SketchRecordIds(const std::string &mdl) {
  std::vector<std::string> ids;
  bool inSketch = false;
  for (const std::string &line : SplitLines(mdl)) {
    if (line.compare(0, 9, "\\\\\\---///") == 0) {
      inSketch = true;
      continue;
    }
    if (line.compare(0, 9, "///---\\\\\\") == 0)
      break;
    if (!inSketch || line.empty() || line[0] < '0' || line[0] > '9')
      continue;
    size_t first = line.find(',');
    if (first == std::string::npos)
      continue;
    size_t second = line.find(',', first + 1);
    ids.push_back(line.substr(0, second == std::string::npos ? line.size() : second));
  }
  return ids;
}

// The frame boilerplate shared by the hand-authored sketch fixtures below: the
// .Control group, the sketch opener, version, title and font lines, and the
// terminator plus a minimal settings section.
const char *const kFixtureControlAndSketchHeader =
    "\r\n"
    "********************************************************\r\n"
    "\t.Control\r\n"
    "********************************************************~\r\n"
    "\t\tSimulation Control Parameters\r\n"
    "\t|\r\n"
    "INITIAL TIME = 0\r\n\t~~|\r\n"
    "FINAL TIME = 10\r\n\t~~|\r\n"
    "TIME STEP = 1\r\n\t~~|\r\n"
    "SAVEPER = 1\r\n\t~~|\r\n"
    "\\\\\\---///\r\n"
    "V300  Do not put anything below this section - it will be ignored\r\n"
    "*View 1\r\n"
    "$192-192-192,0,Helvetica|10|B|0-0-0|0-0-0|-1--1--1|-1--1--1|96,96,100,0\r\n";

const char *const kFixtureSketchFooter =
    "///---\\\\\\\r\n"
    ":L\x7f<%^E!@\r\n"
    "15:0,0,0,0,0,0\r\n";

std::string BuildSketchFixture(const std::string &equations, const std::string &records) {
  return "{UTF-8}\r\n" + equations + kFixtureControlAndSketchHeader + records + kFixtureSketchFooter;
}

// teacup.mdl, inlined verbatim from the pysimlin test fixtures in the simlin
// project (src/pysimlin/tests/fixtures/teacup.mdl). Inlined (rather than read
// from disk) so the test does not depend on the process working
// directory, matching the EquationRoundTripTest corpus convention. A small but
// complete sketched model: one stock with a flow/valve/cloud, several auxiliaries,
// and connectors -- all connectors here are unpolarized (POL field 0), so this
// fixture exercises geometry/attached/cloud round-trip but NOT polarity (SIR
// below covers polarity).
const char *kTeacupMdl =
    R"MDL({UTF-8}
Characteristic Time=
	10
	~	Minutes [0,?]
	~	How long will it take the teacup to cool 1/e of the way to equilibrium?
	|

Heat Loss to Room=
	(Teacup Temperature - Room Temperature) / Characteristic Time
	~	Degrees Fahrenheit/Minute
	~	This is the rate at which heat flows from the cup into the room. We can \
		ignore it at this point.
	|

Room Temperature=
	70
	~	Degrees Fahrenheit [-459.67,?]
	~	Put in a check to ensure the room temperature is not driven below absolute \
		zero.
	|

Teacup Temperature= INTEG (
	-Heat Loss to Room,
		180)
	~	Degrees Fahrenheit [32,212]
	~	The model is only valid for the liquid phase of tea. While the tea could \
		theoretically freeze or boil off, we would want an error to be thrown in \
		these cases so that the modeler can identify the issue and decide whether \
		to expand the model.
		Of course, this refers to standard sea-level conditions...
	|

********************************************************
	.Control
********************************************************~
		Simulation Control Parameters
	|

FINAL TIME  = 30
	~	Minute
	~	The final time for the simulation.
	|

INITIAL TIME  = 0
	~	Minute
	~	The initial time for the simulation.
	|

SAVEPER  =
        TIME STEP
	~	Minute [0,?]
	~	The frequency with which output is stored.
	|

TIME STEP  = 0.125
	~	Minute [0,?]
	~	The time step for the simulation.
	|

\\\---/// Sketch information - do not modify anything except names
V300  Do not put anything below this section - it will be ignored
*View 1
$192-192-192,0,Times New Roman|12||0-0-0|0-0-0|0-0-255|-1--1--1|-1--1--1|72,72,100,0
10,1,Teacup Temperature,307,235,40,20,3,3,0,0,0,0,0,0
12,2,48,479,235,10,8,0,3,0,0,-1,0,0,0
1,3,5,2,4,0,0,22,0,0,0,-1--1--1,,1|(441,235)|
1,4,5,1,100,0,0,22,0,0,0,-1--1--1,,1|(374,235)|
11,5,48,408,235,6,8,34,3,0,0,1,0,0,0
10,6,Heat Loss to Room,408,251,49,8,40,3,0,0,-1,0,0,0
10,7,Room Temperature,469,304,49,8,8,3,0,0,0,0,0,0
10,8,Characteristic Time,408,174,49,8,8,3,0,0,0,0,0,0
1,9,8,5,0,0,0,0,0,64,0,-1--1--1,,1|(408,198)|
1,10,1,6,1,0,0,0,0,64,0,-1--1--1,,1|(340,296)|
1,11,7,6,1,0,0,0,0,64,0,-1--1--1,,1|(437,284)|
///---\\\
:L<%^E!@
1:Current.vdf
9:Current
22:$,Dollar,Dollars,$s
22:Hour,Hours
22:Month,Months
22:Person,People,Persons
22:Unit,Units
22:Week,Weeks
22:Year,Years
22:Day,Days
15:0,0,0,0,0,0
19:100,0
27:2,
34:0,
4:Time
5:Heat Loss to Room
35:Date
36:YYYY-MM-DD
37:2000
38:1
39:1
40:6
41:0
42:1
24:0
25:30
26:30
)MDL";

// SIR.mdl, inlined verbatim from the libsimlin test data in the simlin project
// (src/libsimlin/testdata/SIR.mdl). The classic
// Susceptible-Infectious-Recovered model: two stocks with flows, several
// auxiliaries, ghost (alias) variable boxes, comment/cloud labels, and -- the
// reason this fixture is used here -- connectors carrying BOTH '+' (POL 43) and
// '-' (POL 45) polarity. Inlined for working-directory independence.
const char *kSirMdl =
    R"MDL({UTF-8}
********************************************************
	.SIR-Model
********************************************************~

		The SIR Model of Infectious Disease
		John Sterman (1999) Business Dynamics.  Irwin/McGraw-Hill
		Copyright (c) 1999 John Sterman

		This is the classic SIR (Susceptible-Infectious-Recovered) model of infectious \
		disease.	Infectious individuals remain infectious for a constant average
		period, then recover.

		In this version, the contact rate can be set to increase linearly,
		and the population is challenged by the arrival of a single infectious
		individual every 50 days.  Illustrates herd immunity.  Chapter 9.
	|

Infectious Population I= INTEG (
	Infection Rate-Recovery Rate,
		1)
	~	People
	~	The infectious population accumulates the infection rate and the \
		inmigration of infectious rate less the recovery rate.
	|

Initial Contact Rate=
	2.5
	~	1/Day
	~	The initial contact rate; the actual contact rate rises at a slope \
		determined by the user.
	|

Contact Rate c=
	Initial Contact Rate
	~	1/Day
	~	People in the community interact at a certain rate (the Contact Rate, c, \
		measured in people contacted per person per time period, or 1/time \
		periods).  The contact rate rises at the Ramp Slope starting in day 1.
	|

Reproduction Rate=
	Contact Rate c*Infectivity i*Average Duration of Illness d*Susceptible Population S/\
		Total Population P
	~	Dimensionless
	~		|

Total Population P=
	10000
	~	People
	~	The total population is constant
	|

Infection Rate=
	Contact Rate c*Infectivity i*Susceptible Population S*Infectious Population I/Total Population P
	~	People/Day
	~	The infection rate is the total number of encounters Sc multiplied by the \
		probability that any of those encounters is with an infectious individual \
		I/N, and finally multiplied by the probability that an encounter with an \
		infectious person results in infection i.
	|

Average Duration of Illness d=
	2
	~	Day
	~	The average length of time that a person is infectious.
	|

Recovered Population R= INTEG (
	Recovery Rate,
		0)
	~	People
	~	The recovered population R accumulates the recovery rate
	|

Recovery Rate=
	Infectious Population I/Average Duration of Illness d
	~	People/Day
	~	The rate at which the infected population recover and become immune to the \
		infection.
	|

Infectivity i=
	0.25
	~	Dimensionless
	~	The infectivity (i) of the disease is the probability that a person will \
		become infected after exposure to someone with the disease.
	|

Susceptible Population S= INTEG (
	-Infection Rate,
		Total Population P - Infectious Population I -  Recovered Population R)
	~	People
	~	The susceptible population, as in the simple logistic epidemic model, is \
		reduced by the infection rate.  The initial susceptible population is the \
		total population less the initial number of infectives and any initially \
		recovered individuals.
	|

********************************************************
	.Control
********************************************************~
		Simulation Control Paramaters
	|

FINAL TIME  = 200
	~	Day
	~	The final time for the simulation.
	|

INITIAL TIME  = 0
	~	Day
	~	The initial time for the simulation.
	|

SAVEPER  = 2
	~	Day
	~	The frequency with which output is stored.
	|

TIME STEP  = 0.0625
	~	Day
	~	The time step for the simulation.
	|

\\\---/// Sketch information - do not modify anything except names
V300  Do not put anything below this section - it will be ignored
*View 1
$192-192-192,0,Helvetica|10|B|0-0-0|0-0-0|0-0-0|-1--1--1|-1--1--1|96,96,100,0
10,1,Susceptible Population S,162,192,40,20,3,3,0,0,0,0,0,0
10,2,Infectious Population I,428,190,40,20,3,3,0,0,0,0,0,0
1,3,5,2,4,0,0,22,0,0,0,-1--1--1,,1|(344,191)|
1,4,5,1,100,0,0,22,0,0,0,-1--1--1,,1|(245,191)|
11,5,444,295,191,6,8,34,3,0,0,1,0,0,0
10,6,Infection Rate,295,228,40,29,40,3,0,0,-1,0,0,0
1,7,1,6,1,0,43,0,0,64,0,-1--1--1,,1|(214,259)|
1,8,2,6,1,0,43,0,0,64,0,-1--1--1,,1|(389,256)|
10,9,Infectivity i,394,326,34,15,8,3,0,0,0,0,0,0
10,10,Contact Rate c,168,300,31,19,8,3,0,0,0,0,0,0
1,11,10,6,1,0,43,0,0,192,0,-1--1--1,,1|(269,270)|
1,12,9,6,1,0,43,0,0,64,0,-1--1--1,,1|(313,268)|
12,13,0,232,218,15,15,5,4,0,0,-1,0,0,0
B
12,14,0,365,216,15,15,4,4,0,0,-1,0,0,0
R
10,15,Recovered Population R,672,190,40,20,3,3,0,0,0,0,0,0
1,16,18,15,4,0,0,22,0,0,0,-1--1--1,,1|(594,190)|
1,17,18,2,100,0,0,22,0,0,0,-1--1--1,,1|(506,190)|
11,18,492,550,190,6,8,34,3,0,0,1,0,0,0
10,19,Recovery Rate,550,221,33,23,40,3,0,0,-1,0,0,0
1,20,2,19,1,0,43,0,0,192,0,-1--1--1,,1|(484,256)|
10,21,Average Duration of Illness d,571,316,40,24,8,3,0,0,0,0,0,0
1,22,21,19,1,0,45,0,0,192,0,-1--1--1,,1|(593,264)|
12,23,0,493,212,15,15,5,4,0,0,-1,0,0,0
B
10,24,Total Population P,257,325,40,20,8,3,0,0,0,0,0,0
1,25,24,6,1,0,45,0,0,192,0,-1--1--1,,1|(292,277)|
10,26,Reproduction Rate,383,472,51,21,8,3,0,0,0,0,0,0
10,27,Contact Rate c,249,523,40,20,8,2,0,3,-1,0,0,0,128-128-128,0-0-0,|12|B|128-128-128
10,28,Total Population P,374,574,40,20,8,2,0,3,-1,0,0,0,128-128-128,0-0-0,|12|B|128-128-128
10,29,Infectivity i,504,526,50,18,8,2,0,3,-1,0,0,0,128-128-128,0-0-0,|12|B|128-128-128
10,30,Average Duration of Illness d,236,444,40,20,8,2,0,3,-1,0,0,0,128-128-128,0-0-0,|12|B|128-128-128
10,31,Susceptible Population S,530,449,49,29,8,2,0,3,-1,0,0,0,128-128-128,0-0-0,|12|B|128-128-128
1,32,30,26,0,0,43,0,0,64,0,-1--1--1,,1|(297,455)|
1,33,27,26,0,0,43,0,0,64,0,-1--1--1,,1|(303,502)|
1,34,29,26,0,0,43,0,0,64,0,-1--1--1,,1|(453,503)|
1,35,31,26,0,0,43,0,0,64,0,-1--1--1,,1|(464,458)|
1,36,28,26,0,0,45,0,0,64,0,-1--1--1,,1|(377,530)|
12,37,0,232,238,29,9,8,4,0,8,-1,0,0,0,0-0-0,0-0-0,|8|B|0-0-0
Depletion
12,38,0,365,240,30,8,8,4,0,8,-1,0,0,0,0-0-0,0-0-0,|8|B|0-0-0
Contagion
12,39,0,496,235,29,9,8,4,0,8,-1,0,0,0,0-0-0,0-0-0,|8|B|0-0-0
Recovery
10,40,Initial Contact Rate,58,364,50,25,8,3,0,0,0,0,0,0
1,41,40,10,1,0,43,0,0,192,0,-1--1--1,,1|(112,348)|
1,42,2,1,0,0,0,0,0,64,1,-1--1--1,,1|(301,190)|
1,43,15,1,0,0,0,0,0,64,1,-1--1--1,,1|(423,190)|
1,44,24,1,0,0,0,0,0,64,1,-1--1--1,,1|(213,264)|
///---\\\
:L<%^E!@
1:sir.vdf
9:sir
15:0,0,0,0,0,0
19:100,0
27:2,
34:0,
4:Time
5:Reproduction Rate
35:Date
36:YYYY-MM-DD
37:2000
38:1
39:1
40:2
41:0
42:0
24:0
25:200
26:200
57:1
54:0
55:0
59:0
56:0
58:0
44:65001
46:0
45:0
49:0
50:0
51:
52:
53:
43:sir
47:sir
48:
)MDL";

}  // namespace

// AC4.1 / AC3.4: a small, complete sketched Vensim model (teacup) round-trips
// with its VensimView geometry intact -- element types, positions, the
// attached-to-valve flow, the comment cloud, and all connector endpoints are
// equivalent after parse -> write -> re-parse. All teacup connectors are
// unpolarized, so this is the geometry/structure case; SIR below covers polarity.
TEST(SketchRoundTrip_teacup_geometry) {
  ExpectCleanRoundTrip(kTeacupMdl);
}

// AC4.1 / AC3.4: the SIR model round-trips its full sketch -- two stocks with
// attached flows/valves, ghost (alias) variable boxes, comment/cloud labels, and
// polarized connectors. An empty diff list means every element's type, geometry,
// connector endpoints, polarity, variable reference, attached state, and ghost
// state matched.
TEST(SketchRoundTrip_sir_geometry) {
  ExpectCleanRoundTrip(kSirMdl);
}

// AC3.4 (focused, non-vacuous): connector polarity survives the round trip.
// Guards specifically against a writer that drops the POL field. The assertion
// is non-vacuous because it first confirms the ORIGINAL SIR view actually
// carries both '+' and '-' connectors (a fixture without polarity would make a
// "polarity survives" check pass trivially), then confirms the re-parsed view
// carries the same counts of each sign.
TEST(SketchRoundTrip_sir_polarity_survives) {
  Model *m0 = roundtrip::ParseVensim(kSirMdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  CHECK(errs.empty());

  Model *m1 = roundtrip::ParseVensim(regen);
  CHECK(m1 != nullptr);
  if (!m1) {
    delete m0;
    return;
  }

  VensimView *v0 = FirstNonEmptyView(m0);
  VensimView *v1 = FirstNonEmptyView(m1);
  CHECK(v0 != nullptr);
  CHECK(v1 != nullptr);
  if (v0 && v1) {
    PolarityCounts c0 = CountPolarities(v0);
    PolarityCounts c1 = CountPolarities(v1);
    // Non-vacuity: the source fixture must genuinely contain both signs, or the
    // survival assertions below would be testing nothing.
    CHECK(c0.plus > 0);
    CHECK(c0.minus > 0);
    // Survival: the same counts of '+' and '-' connectors are present after the
    // round trip.
    CHECK(c1.plus == c0.plus);
    CHECK(c1.minus == c0.minus);
  }

  delete m0;
  delete m1;
}

// AC3.4 (focused, non-vacuous): ghost (alias) variable boxes survive the round
// trip. The SIR sketch ghosts several auxiliaries (UIDs 27-31). The assertion
// first confirms the original view actually contains ghost elements, then that
// the re-parsed view contains the same number -- guarding against a writer that
// inverts or drops the ghost bit, which would silently turn alias boxes into
// duplicate primary definitions.
TEST(SketchRoundTrip_sir_ghosts_survive) {
  Model *m0 = roundtrip::ParseVensim(kSirMdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  CHECK(errs.empty());

  Model *m1 = roundtrip::ParseVensim(regen);
  CHECK(m1 != nullptr);
  if (!m1) {
    delete m0;
    return;
  }

  VensimView *v0 = FirstNonEmptyView(m0);
  VensimView *v1 = FirstNonEmptyView(m1);
  CHECK(v0 != nullptr);
  CHECK(v1 != nullptr);
  if (v0 && v1) {
    int g0 = CountGhosts(v0);
    int g1 = CountGhosts(v1);
    CHECK(g0 > 0);  // non-vacuity: the fixture must actually ghost some variables
    CHECK(g1 == g0);
  }

  delete m0;
  delete m1;
}

// AC4.3 (no fabrication): a model whose sketch references only some of its
// variables round-trips with the SAME view element count -- the writer invents
// no new variable elements for variables that were absent from the original
// sketch. teacup defines four variables; its sketch holds a specific set of
// element records (variables, a valve, a cloud, connectors). The count must be
// non-trivial (so "equal counts" is meaningful) and unchanged across the trip.
TEST(SketchRoundTrip_teacup_no_fabrication) {
  Model *m0 = roundtrip::ParseVensim(kTeacupMdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  CHECK(errs.empty());

  Model *m1 = roundtrip::ParseVensim(regen);
  CHECK(m1 != nullptr);
  if (!m1) {
    delete m0;
    return;
  }

  VensimView *v0 = FirstNonEmptyView(m0);
  VensimView *v1 = FirstNonEmptyView(m1);
  CHECK(v0 != nullptr);
  CHECK(v1 != nullptr);
  if (v0 && v1) {
    size_t n0 = v0->Elements().size();
    size_t n1 = v1->Elements().size();
    CHECK(n0 > 1);    // non-vacuity: a real sketch with several elements
    CHECK(n1 == n0);  // no element invented, none dropped
  }

  delete m0;
  delete m1;
}

// AC4.2 (no view): a model with equations but no sketch section round-trips
// cleanly. The writer emits a single empty *View frame so the trailing settings
// section still re-parses; the empty view carries no geometry, and the
// comparator's NonEmptyViews filter treats "no sketch" and "one empty view" as
// equivalent (both: no geometry). The fixture below has no \\\---/// block at
// all, so the reader produces no view, and the round trip must still be clean.
TEST(SketchRoundTrip_no_view) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "a = 2\r\n\t~~|\r\n"
      "b = 3\r\n\t~~|\r\n"
      "c = a * b\r\n\t~~|\r\n";
  ExpectCleanRoundTrip(mdl);
}

// A view title is modeler-authored free text that reaches the writer unfiltered
// (XmileReader::ProcessViews stores an XMILE <view name="..."> verbatim), and it
// is emitted onto the sketch header's *Title line. That header is POSITIONAL --
// opener, version, *Title, font/cosmetic line, then one record per line -- so a
// title carrying a line break splits into two physical lines, the font line
// lands in the first record slot, and every following record is misread. This
// asserts the structure survives: the font line is still directly below the
// title line, and the view's element geometry is unchanged.
TEST(SketchTitle_line_break_does_not_corrupt_sketch) {
  Model *m0 = ParseWithViewTitle(kTeacupMdl, "Main\nView|x\n\\\\\\---///\n*trailing");
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  VensimView *v0 = FirstNonEmptyView(m0);
  CHECK(v0 != nullptr);
  if (!v0) {
    delete m0;
    return;
  }
  std::vector<std::string> sig0 = ElementSignature(v0);
  CHECK(sig0.size() > 5);  // non-vacuity: teacup's sketch really has records

  std::vector<std::string> errs;
  std::string regen = m0->PrintMDL(errs);
  CHECK(errs.empty());

  // Structural: exactly one line between the version line and the font line.
  std::vector<std::string> header = SketchHeaderLines(regen);
  CHECK(header.size() == 3);
  if (header.size() == 3) {
    CHECK(header[0].compare(0, 5, "V300 ") == 0);
    CHECK(!header[1].empty() && header[1][0] == '*');
    CHECK(!header[2].empty() && header[2][0] == '$');
  }

  Model *m1 = roundtrip::ParseVensim(regen);
  CHECK(m1 != nullptr);
  if (!m1) {
    delete m0;
    return;
  }
  VensimView *v1 = FirstNonEmptyView(m1);
  CHECK(v1 != nullptr);
  if (v1) {
    // Geometry, not just parseability: every element keeps its uid, kind and box.
    CHECK(ElementSignature(v1) == sig0);
  }

  delete m0;
  delete m1;
}

// The characters a *Title line has to neutralize, and what each becomes. The
// reader recovers the title as everything after the leading '*' of one physical
// line (VensimParse.cpp:315-317), so the only hazard intrinsic to THIS line is a
// break; the '|' -> '/' and section-terminator substitutions come with
// mdl::SanitizeFreeText and are kept as defense in depth for consumers that scan
// the line more aggressively. A leading '*' is deliberately NOT substituted: the
// reader skips exactly one character, so "*" + "*starred" reads back as
// "*starred". An empty title falls back to "View 1" rather than emitting a bare
// "*", which would read back as the empty string and lose the frame's name.
TEST(SketchTitle_hostile_titles_round_trip_to_documented_forms) {
  struct Case {
    const char *raw;
    const char *expected;
  };
  const Case cases[] = {
      {"Main\nView|x", "Main View/x"},
      {"a\r\nb", "a b"},
      {"a\rb", "a b"},
      {"trailing break\n", "trailing break "},
      {"\\\\\\---///", " "},
      {"///---\\\\\\", " "},
      {"has | pipe", "has / pipe"},
      {"*starred", "*starred"},
      {"$fontish", "$fontish"},
      {"", "View 1"},
  };

  for (const Case &c : cases) {
    Model *m0 = ParseWithViewTitle(kTeacupMdl, c.raw);
    CHECK(m0 != nullptr);
    if (!m0)
      continue;

    std::vector<std::string> errs;
    std::string regen = m0->PrintMDL(errs);
    CHECK(errs.empty());

    Model *m1 = roundtrip::ParseVensim(regen);
    CHECK(m1 != nullptr);
    if (!m1) {
      delete m0;
      continue;
    }
    VensimView *v1 = FirstNonEmptyView(m1);
    CHECK(v1 != nullptr);
    if (v1)
      CHECK_EQ_STR(v1->Title(), c.expected);

    delete m0;
    delete m1;
  }
}

// Sanitizing must land on a FIXPOINT, not merely a safe first emission: the
// sanitized title is what the re-parse hands back to the writer, so a
// substitution that is not idempotent (e.g. one that appended or trimmed on
// every pass) would make the writer's output drift on each conversion. Emit,
// re-parse, emit -- the two emissions must be byte-identical.
//
// The title deliberately places a '|' directly against a "---" run as well as
// carrying breaks and a whole terminator: without that adjacency the case is
// silent about the substitutions INTERACTING, and the interaction is where the
// fixpoint was actually lost ('|' becomes '/', completing a run that the
// terminator scan had already looked past).
TEST(SketchTitle_sanitized_title_emit_is_fixpoint) {
  Model *m0 = ParseWithViewTitle(kTeacupMdl, "Main\nView|x\n\\\\\\---///\n*trailing\n\\\\\\---||||");
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  std::vector<std::string> errs;
  std::string first = m0->PrintMDL(errs);
  CHECK(errs.empty());
  CHECK(!first.empty());
  delete m0;

  Model *m1 = roundtrip::ParseVensim(first);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  std::string second = m1->PrintMDL(errs);
  delete m1;
  CHECK(errs.empty());
  CHECK_EQ_STR(first, second);
}

// A sketch describes the equation section, and the only thing a record carries
// about its subject is a NAME -- re-reading resolves it through
// VensimParse::FindVariable, which sees only what the equation text interned. So
// a record naming a variable the emitted equations never mention is a `.mdl`
// that is inconsistent with ITSELF: it re-reads with
// VensimVariableElement::_variable NULL, and every consumer that follows that
// pointer has a NULL dereference waiting for it.
//
// This is the shape that actually produced one: an XMILE <flow> with no <eqn>
// stays untyped, so the equation section emits nothing for it while the sketch
// still named it. The three invariants asserted here (via
// ExpectSelfConsistentSketch) are what makes the emission well-formed, and the
// explicit record list is what makes the test non-vacuous -- an emitter that
// simply dropped the whole sketch would satisfy the invariants too.
TEST(SketchRecords_unresolved_name_drops_its_valve_and_connectors) {
  const std::string mdl = BuildSketchFixture("keeper = 1\r\n\t~~|\r\n",
                                             "10,1,keeper,400,300,0,0,3,3,0,0,0,0,0,0\r\n"
                                             "11,2,0,200,150,0,0,34\r\n"
                                             "10,3,orphan flow,200,150,0,0,35,3,0,0,0,0,0,0\r\n"
                                             "12,4,0,120,150,0,0,8,0\r\n"
                                             "12,5,0,300,150,0,0,8,0\r\n"
                                             "1,6,2,4,1,0,0,0,0,64,0,-1--1--1,,1|(160,150)|\r\n"
                                             "1,7,2,5,1,0,0,0,0,64,0,-1--1--1,,1|(250,150)|\r\n");

  const std::string emitted = ExpectSelfConsistentSketch(mdl, "orphan flow");
  CHECK(!emitted.empty());

  // Exactly the unresolvable flow record, the attached valve that paired with
  // it, and the two pipe connectors that started at that valve are gone; the
  // unrelated records keep their UIDs.
  const std::vector<std::string> expected = {"10,1", "12,4", "12,5"};
  CHECK(SketchRecordIds(emitted) == expected);
}

// A variable can be perfectly legitimate and still have no definition of its
// own: `keeper = phantom` names `phantom` without defining it, so the equation
// section spells the name out and the re-read resolves it. Its sketch record --
// and the connector into it -- must therefore SURVIVE.
//
// This is the case that separates the filter this writer uses (the name was
// emitted) from the obvious wrong one (the variable has an equation, or a
// non-UNKNOWN type). `phantom` has neither an equation nor a type, so a filter
// keyed on either would delete a record that re-reads perfectly well.
TEST(SketchRecords_referenced_but_undefined_variable_keeps_its_record) {
  const std::string mdl = BuildSketchFixture("keeper = phantom\r\n\t~~|\r\n",
                                             "10,1,keeper,100,100,40,20,3,3,0,0,0,0,0,0\r\n"
                                             "10,2,phantom,300,100,40,20,3,3,0,0,0,0,0,0\r\n"
                                             "1,3,2,1,1,0,0,0,0,64,0,-1--1--1,,1|(200,100)|\r\n");

  const std::string emitted = ExpectSelfConsistentSketch(mdl, "referenced-only");
  CHECK(!emitted.empty());

  const std::vector<std::string> expected = {"10,1", "10,2", "1,3"};
  CHECK(SketchRecordIds(emitted) == expected);
}

// A connector whose endpoint is a record that had to go must go with it.
// Leaving it behind is what made the earlier behaviour STABLE rather than
// self-correcting: the connector was re-read, re-emitted, and re-dangled on
// every conversion, forever, naming two UIDs with no records.
TEST(SketchRecords_connector_into_a_removed_uid_is_dropped) {
  const std::string mdl = BuildSketchFixture("keeper = 1\r\n\t~~|\r\n",
                                             "10,1,keeper,100,100,40,20,3,3,0,0,0,0,0,0\r\n"
                                             "10,2,phantom,300,100,40,20,3,3,0,0,0,0,0,0\r\n"
                                             "1,3,1,2,1,0,0,0,0,64,0,-1--1--1,,1|(200,100)|\r\n");

  const std::string emitted = ExpectSelfConsistentSketch(mdl, "dangling connector");
  CHECK(!emitted.empty());

  const std::vector<std::string> expected = {"10,1"};
  CHECK(SketchRecordIds(emitted) == expected);
  // Stated directly as well as through the invariant, because this is the exact
  // byte pattern the defect emitted: a connector between two absent UIDs.
  CHECK(emitted.find("1,3,1,2,") == std::string::npos);
}

// A hole in the UID sequence is ordinary input, not a malformed file: the
// reader leaves a NULL in every slot no record claims, so a hand-authored
// sketch can skip UIDs freely -- and an attached valve can end up pointing at
// one. Converting such a file to XMILE used to walk straight into the NULL
// (generateView re-points a connector endpoint to valve_uid + 1 without
// re-testing it) and segfault, with no involvement from the writer at all.
TEST(SketchRecords_sparse_uid_sequence_converts_without_crashing) {
  const std::string mdl = BuildSketchFixture("keeper = 1\r\n\t~~|\r\n",
                                             "10,1,keeper,400,300,0,0,3,3,0,0,0,0,0,0\r\n"
                                             "11,2,0,200,150,0,0,34\r\n"
                                             "12,4,0,120,150,0,0,8,0\r\n"
                                             "12,5,0,300,150,0,0,8,0\r\n"
                                             "1,6,2,4,1,0,0,0,0,64,0,-1--1--1,,1|(160,150)|\r\n"
                                             "1,7,2,5,1,0,0,0,0,64,0,-1--1--1,,1|(250,150)|\r\n");

  // The XMILE direction is the one that crashed; reaching this line at all is
  // most of the assertion.
  char *xmile = convert_mdl_to_xmile(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "sparse.mdl",
                                     /*isCompact=*/false, /*isLongName=*/0, /*isAsSectors=*/false);
  CHECK(xmile != nullptr);
  if (xmile) {
    CHECK(strstr(xmile, "<xmile") != nullptr);
    free(xmile);
  }

  // The .mdl direction must also close the hole rather than re-emitting an
  // attached valve with nothing to attach to.
  const std::string emitted = ExpectSelfConsistentSketch(mdl, "sparse");
  CHECK(!emitted.empty());
  const std::vector<std::string> expected = {"10,1", "12,4", "12,5"};
  CHECK(SketchRecordIds(emitted) == expected);
}

// The end-to-end shape, driven through the same extern-C entry points the CLI
// uses. An XMILE <flow> with no <eqn> and no stock attachment is accepted by
// the reader with an advisory, so this document is well formed:
//   pass 1  XMILE -> .mdl
//   pass 2  .mdl  -> .mdl   (must reproduce pass 1 BYTE FOR BYTE)
//   pass 3  .mdl  -> XMILE  (used to be the segfault)
// Before the fix, pass 1 emitted a record for the untyped flow, pass 2 dropped
// it and left a hole its attached valve still pointed into, and pass 3 died on
// that hole.
TEST(SketchRecords_untyped_xmile_flow_survives_three_conversions) {
  const std::string xmileDoc =
      "<xmile xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\" version=\"1.0\">\n"
      " <header><vendor>t</vendor><product lang=\"en\">t</product></header>\n"
      " <sim_specs><start>0</start><stop>10</stop><dt>1</dt></sim_specs>\n"
      " <model>\n"
      "  <variables>\n"
      "   <aux name=\"keeper\"><eqn>1</eqn></aux>\n"
      "   <flow name=\"orphan flow\"></flow>\n"
      "  </variables>\n"
      "  <views>\n"
      "   <view>\n"
      "    <aux name=\"keeper\" x=\"400\" y=\"300\"/>\n"
      "    <flow name=\"orphan flow\" x=\"200\" y=\"150\">\n"
      "     <pts><pt x=\"120\" y=\"150\"/><pt x=\"300\" y=\"150\"/></pts>\n"
      "    </flow>\n"
      "   </view>\n"
      "  </views>\n"
      " </model>\n"
      "</xmile>\n";

  char *pass1 =
      convert_xmile_to_mdl(xmileDoc.c_str(), static_cast<uint32_t>(xmileDoc.size()), "orphan.xmile", /*isLongName=*/0);
  CHECK(pass1 != nullptr);
  if (!pass1)
    return;
  const std::string mdl1(pass1);
  free(pass1);

  char *pass2 = convert_to_mdl(mdl1.c_str(), static_cast<uint32_t>(mdl1.size()), "orphan.mdl", /*isLongName=*/0);
  CHECK(pass2 != nullptr);
  if (!pass2)
    return;
  const std::string mdl2(pass2);
  free(pass2);
  // The fixpoint IS the fix: the writer must not be able to emit something it
  // would itself edit on the next pass.
  CHECK_EQ_STR(mdl1, mdl2);

  char *pass3 = convert_mdl_to_xmile(mdl2.c_str(), static_cast<uint32_t>(mdl2.size()), "orphan.mdl",
                                     /*isCompact=*/false, /*isLongName=*/0, /*isAsSectors=*/false);
  CHECK(pass3 != nullptr);
  if (pass3) {
    CHECK(strstr(pass3, "<xmile") != nullptr);
    free(pass3);
  }

  // Non-vacuity: pass 1 really did carry a sketch, so the invariants below are
  // about a sketch that exists.
  Model *m1 = roundtrip::ParseVensim(mdl1);
  CHECK(m1 != nullptr);
  if (m1) {
    CHECK(FirstNonEmptyView(m1) != nullptr);
    delete m1;
  }
  ExpectSelfConsistentSketch(mdl1, "xmile orphan flow");
}

// The corpus case, kept as its own test because it is a real vendored model
// rather than a constructed one: multipoint-connection.stmx declares two
// <aux> elements with no <eqn> at all and draws a connector between them, so
// every sketch record it produced named a variable its own equation section
// never defined. Re-reading the emission printed "Can't find - <name>" once per
// record; the target is zero.
TEST(SketchRecords_multipoint_connection_fixture_reemits_consistently) {
  const std::string path =
      std::string(XMUTIL_SRC_ROOT) + "/test/fixtures/test-models/samples/display/multipoint-connection.stmx";
  std::ifstream in(path, std::ios::binary);
  CHECK(in.good());  // a moved or renamed fixture must fail, never skip
  if (!in.good())
    return;
  std::ostringstream ss;
  ss << in.rdbuf();
  const std::string stmx = ss.str();
  CHECK(!stmx.empty());

  char *converted =
      convert_xmile_to_mdl(stmx.c_str(), static_cast<uint32_t>(stmx.size()), "multipoint.stmx", /*isLongName=*/0);
  CHECK(converted != nullptr);
  if (!converted)
    return;
  const std::string mdl(converted);
  free(converted);

  // Re-read the emission and count the reader's own diagnostic for a record it
  // could not resolve. Reading it off stderr rather than off the model is the
  // point: this is the exact signal a user sees.
  std::string diagnostics = CaptureStderr([&mdl]() {
    Model *m = roundtrip::ParseVensim(mdl);
    delete m;
  });
  CHECK(CountOccurrences(diagnostics, "Can't find - ") == 0);
  if (CountOccurrences(diagnostics, "Can't find - ") != 0)
    printf("  unexpected reader diagnostics:\n%s", diagnostics.c_str());

  ExpectSelfConsistentSketch(mdl, "multipoint-connection");
}

// The suppression must be TARGETED. A model whose sketch is entirely legitimate
// keeps every record, valve pairing and connector it had -- otherwise the three
// invariants above could be satisfied by an emitter that simply dropped
// everything, and the tests that assert them would mean nothing.
TEST(SketchRecords_legitimate_sketches_are_untouched) {
  struct Case {
    const char *label;
    const char *mdl;
  };
  const Case cases[] = {{"teacup", kTeacupMdl}, {"sir", kSirMdl}};
  for (const Case &c : cases) {
    Model *m0 = roundtrip::ParseVensim(c.mdl);
    CHECK(m0 != nullptr);
    if (!m0)
      continue;
    VensimView *v0 = FirstNonEmptyView(m0);
    CHECK(v0 != nullptr);
    std::vector<std::string> sig0 = v0 ? ElementSignature(v0) : std::vector<std::string>();
    CHECK(sig0.size() > 5);  // non-vacuity: a real sketch with many records
    std::vector<std::string> errs;
    const std::string emitted = m0->PrintMDL(errs);
    delete m0;
    CHECK(errs.empty());

    Model *m1 = roundtrip::ParseVensim(emitted);
    CHECK(m1 != nullptr);
    if (!m1)
      continue;
    VensimView *v1 = FirstNonEmptyView(m1);
    CHECK(v1 != nullptr);
    if (v1) {
      // Every element keeps its uid, kind and box: nothing was suppressed.
      CHECK(ElementSignature(v1) == sig0);
      // And the model has attached valves, so the valve-pairing invariant is
      // being exercised rather than passing over an empty set.
      bool sawAttachedValve = false;
      for (VensimViewElement *e : v1->Elements()) {
        if (e && e->Type() == VensimViewElement::ElementTypeVALVE && static_cast<VensimValveElement *>(e)->Attached())
          sawAttachedValve = true;
      }
      CHECK(sawAttachedValve);
      CHECK_EQ_STR(EveryVariableRecordResolves(v1), "");
      CHECK_EQ_STR(EveryConnectorEndpointExists(v1), "");
      CHECK_EQ_STR(EveryAttachedValveIsFollowedByItsFlow(v1), "");
    }
    delete m1;
  }
}

// XMILEGenerator::generateView walks the view by index, and two things it does
// depend on that index being the element's real UID: a flow takes its position
// from the valve at local_uid - 1, and its pipe is the connectors whose From()
// is that valve. The index used to be a counter advanced at the bottom of the
// loop, and one `continue` in the body skipped it -- the branch for an element
// whose variable has no XMILE tag at all (an untyped variable, an array). From
// that element onward the whole view was off by one, so the flow paired with
// whatever sat one slot early and its pipe connectors matched nothing, and the
// writer quietly substituted a fabricated straight pipe for the real geometry.
//
// The two models below carry the SAME sketch bytes and differ only in whether
// `phantom` has an equation, which is the only thing that decides whether it
// gets a tag. Their emitted flow geometry must be identical: an element the
// writer cannot tag must not move anything else.
TEST(SketchView_untagged_element_does_not_shift_the_flow_pairing) {
  const char *const kSketch =
      "\r\n"
      "Stock = INTEG( inflow, 0)\r\n\t~~|\r\n"
      "inflow = 5\r\n\t~~|\r\n"
      "\r\n"
      "********************************************************\r\n"
      "\t.Control\r\n"
      "********************************************************~\r\n"
      "\t\tSimulation Control Parameters\r\n"
      "\t|\r\n"
      "INITIAL TIME = 0\r\n\t~~|\r\n"
      "FINAL TIME = 10\r\n\t~~|\r\n"
      "TIME STEP = 1\r\n\t~~|\r\n"
      "SAVEPER = 1\r\n\t~~|\r\n"
      "\\\\\\---///\r\n"
      "V300  Do not put anything below this section - it will be ignored\r\n"
      "*View 1\r\n"
      "$192-192-192,0,Helvetica|10|B|0-0-0|0-0-0|-1--1--1|-1--1--1|96,96,100,0\r\n"
      // phantom sits FIRST, so a skipped index shifts everything after it.
      "10,1,phantom,100,100,40,20,3,3,0,0,0,0,0,0\r\n"
      "10,2,keeper,100,200,40,20,3,3,0,0,0,0,0,0\r\n"
      "10,3,Stock,500,300,40,20,3,3,0,0,0,0,0,0\r\n"
      "11,4,0,300,300,6,8,34\r\n"
      "10,5,inflow,300,320,40,20,35,3,0,0,0,0,0,0\r\n"
      "12,6,0,200,300,10,8,8,0\r\n"
      "1,7,4,6,4,0,0,22,0,0,0,-1--1--1,,1|(250,300)|\r\n"
      "1,8,4,3,100,0,0,22,0,0,0,-1--1--1,,1|(400,300)|\r\n"
      "///---\\\\\\\r\n"
      ":L\x7f<%^E!@\r\n"
      "15:0,0,0,0,0,0\r\n";

  // `phantom` is named but never defined, so it stays untyped and gets no tag.
  const std::string untyped = std::string("{UTF-8}\r\nkeeper = phantom\r\n\t~~|\r\n") + kSketch;
  // The control: one extra equation gives `phantom` a type, and nothing else
  // changes -- same records, same coordinates, same view origin.
  const std::string typed = std::string("{UTF-8}\r\nkeeper = phantom\r\n\t~~|\r\nphantom = 1\r\n\t~~|\r\n") + kSketch;

  auto flowGeometry = [](const std::string &mdl) {
    char *out = convert_mdl_to_xmile(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "desync.mdl",
                                     /*isCompact=*/true, /*isLongName=*/0, /*isAsSectors=*/false);
    CHECK(out != nullptr);
    if (!out)
      return std::string();
    const std::string doc(out);
    free(out);
    // The <flow name="inflow" ...> placement element and its <pts> block, which
    // is where both the pairing and the pipe reconstruction show up.
    const size_t start = doc.find("<flow name=\"inflow\" ");
    if (start == std::string::npos)
      return std::string();
    const size_t end = doc.find("</flow>", start);
    return doc.substr(start, end == std::string::npos ? std::string::npos : end - start);
  };

  const std::string typedGeometry = flowGeometry(typed);
  const std::string untypedGeometry = flowGeometry(untyped);
  // Non-vacuity: the control really did reconstruct a pipe from the sketch, and
  // it anchors on the cloud and the stock rather than on the fabricated
  // fallback the writer uses when it matches no pipe connector.
  CHECK(typedGeometry.find("<pts>") != std::string::npos);
  CHECK(typedGeometry.find("x=\"200\"") != std::string::npos);
  CHECK(typedGeometry.find("x=\"500\"") != std::string::npos);
  CHECK_EQ_STR(untypedGeometry, typedGeometry);
}

// The same index, seen through its other consumer. A connector out of a ghost
// emits `<from><alias uid="UIDOffset + cele->From()"/></from>` -- computed from
// the array index -- while the `<alias>` element itself is emitted with the
// walk's own uid. The two are the same number only while the walk's uid tracks
// the array index, so the drift above did not merely misplace geometry: it
// emitted an alias reference pointing at a uid no element in the document
// carried.
TEST(SketchView_alias_reference_matches_the_alias_element_uid) {
  const std::string mdl = BuildSketchFixture(
      // `phantom` is named but never defined, so it stays untagged and is the
      // element the walk used to skip without advancing.
      "keeper = phantom\r\n\t~~|\r\nsrc = 1\r\n\t~~|\r\ndst = src\r\n\t~~|\r\n",
      "10,1,phantom,100,100,40,20,3,3,0,0,0,0,0,0\r\n"
      "10,2,src,100,200,40,20,3,3,0,0,0,0,0,0\r\n"
      "10,3,dst,400,200,40,20,3,3,0,0,0,0,0,0\r\n"
      // bits bit0 clear marks this second `src` box a ghost.
      "10,4,src,300,400,40,20,3,2,0,0,0,0,0,0\r\n"
      "1,5,4,3,1,0,0,0,0,64,0,-1--1--1,,1|(350,300)|\r\n");

  char *out = convert_mdl_to_xmile(mdl.c_str(), static_cast<uint32_t>(mdl.size()), "alias.mdl",
                                   /*isCompact=*/true, /*isLongName=*/0, /*isAsSectors=*/false);
  CHECK(out != nullptr);
  if (!out)
    return;
  const std::string doc(out);
  free(out);

  // The <alias ...> placement element carries uid as its LAST attribute; the
  // reference inside <from> is a bare <alias uid="N"/>.
  const size_t placement = doc.find("<alias x=");
  CHECK(placement != std::string::npos);
  const size_t reference = doc.find("<alias uid=");
  CHECK(reference != std::string::npos);
  if (placement == std::string::npos || reference == std::string::npos)
    return;

  auto uidAfter = [&doc](size_t from) {
    const size_t key = doc.find("uid=\"", from);
    if (key == std::string::npos)
      return std::string();
    const size_t start = key + 5;
    const size_t end = doc.find('"', start);
    return end == std::string::npos ? std::string() : doc.substr(start, end - start);
  };

  const std::string placementUid = uidAfter(placement);
  const std::string referenceUid = uidAfter(reference);
  CHECK(!placementUid.empty());
  CHECK_EQ_STR(referenceUid, placementUid);
}

// mdl::SanitizeFreeText has to reach a FIXPOINT, and the section-terminator
// scan is where it did not: the per-char pass rewrites '|' as '/', so a title
// of backslash-backslash-backslash-dash-dash-dash-pipe-pipe-pipe-pipe carries
// no terminator on the way in and a live one on the way out. The next
// conversion's scan -- now looking at slashes -- ate it and most of the field
// with it, so emit(1) and emit(2) disagreed and text vanished on the second
// write. Asserted through the writer rather than the helper because the drift
// only shows up across a full emit / re-read / emit cycle.
TEST(SketchTitle_pipes_completing_a_terminator_are_a_fixpoint) {
  Model *m0 = ParseWithViewTitle(kTeacupMdl, "\\\\\\---||||");
  CHECK(m0 != nullptr);
  if (!m0)
    return;

  std::vector<std::string> errs;
  const std::string first = m0->PrintMDL(errs);
  delete m0;
  CHECK(errs.empty());
  CHECK(!first.empty());

  Model *m1 = roundtrip::ParseVensim(first);
  CHECK(m1 != nullptr);
  if (!m1)
    return;
  const std::string second = m1->PrintMDL(errs);
  CHECK(errs.empty());
  CHECK_EQ_STR(first, second);

  // Non-vacuity, and the documented outcome: the title the reader hands back
  // carries no terminator run at all, on the FIRST emission rather than after a
  // second pass silently rewrote it.
  VensimView *v1 = FirstNonEmptyView(m1);
  CHECK(v1 != nullptr);
  if (v1) {
    CHECK(v1->Title().find("---///") == std::string::npos);
    CHECK(v1->Title().find("///---") == std::string::npos);
  }
  delete m1;

  // A third pass changes nothing either.
  Model *m2 = roundtrip::ParseVensim(second);
  CHECK(m2 != nullptr);
  if (m2) {
    const std::string third = m2->PrintMDL(errs);
    delete m2;
    CHECK_EQ_STR(second, third);
  }
}

// The Vensim sketch and settings readers pull a line into a fixed 4096-byte
// buffer and hand the remainder back as the NEXT line, which shifts every
// following line by one slot: an over-long *Title costs the re-read the whole
// sketch AND the trailing :L settings block, at exit status zero with no other
// diagnostic. Truncating the text here would violate the free-text contract
// this writer holds everywhere else (a documented substitution, never a silent
// drop), so the text is emitted intact and the hazard is reported instead.
//
// The boundary is asserted in both directions so the threshold itself is
// pinned: a title one character shorter is silent, and that shorter title's
// sketch really does survive a re-read.
TEST(SketchTitle_overlong_line_warns_instead_of_vanishing_silently) {
  // The emitted line is '*' + title, and the longest line the reader returns
  // intact is 4094 characters.
  const std::string safeTitle(4093, 'T');
  const std::string overlongTitle(4094, 'T');

  std::string safeWarnings;
  std::string safeMdl;
  Model *mSafe = ParseWithViewTitle(kTeacupMdl, safeTitle);
  CHECK(mSafe != nullptr);
  if (mSafe) {
    safeWarnings = CaptureStderr([&]() {
      std::vector<std::string> errs;
      safeMdl = mSafe->PrintMDL(errs);
    });
    delete mSafe;
  }
  CHECK(safeWarnings.find("sketch frame title") == std::string::npos);
  // Non-vacuity for the threshold: the safe title really does survive a
  // re-read, so 4094 is the boundary and not an arbitrary number.
  CHECK(!safeMdl.empty());
  Model *mSafeBack = safeMdl.empty() ? nullptr : roundtrip::ParseVensim(safeMdl);
  CHECK(mSafeBack != nullptr);
  if (mSafeBack) {
    CHECK(FirstNonEmptyView(mSafeBack) != nullptr);
    delete mSafeBack;
  }

  std::string overlongWarnings;
  Model *mLong = ParseWithViewTitle(kTeacupMdl, overlongTitle);
  CHECK(mLong != nullptr);
  if (mLong) {
    overlongWarnings = CaptureStderr([&]() {
      std::vector<std::string> errs;
      mLong->PrintMDL(errs);
    });
    delete mLong;
  }
  CHECK(overlongWarnings.find("warning:") != std::string::npos);
  CHECK(overlongWarnings.find("sketch frame title") != std::string::npos);
  if (overlongWarnings.find("sketch frame title") == std::string::npos)
    printf("  captured instead: [%s]\n", overlongWarnings.c_str());

  // The same limit applies to a variable record, whose name is equally
  // modeler-authored. The source has to be XMILE rather than .mdl: a name this
  // long cannot reach the writer through a .mdl sketch, because the reader
  // splits the over-long record on the way IN, which is the whole problem. An
  // XMILE name attribute has no such limit, which is where such names come
  // from.
  const std::string longName(4200, 'v');
  const std::string xmileDoc =
      "<xmile xmlns=\"http://docs.oasis-open.org/xmile/ns/XMILE/v1.0\" version=\"1.0\">\n"
      " <header><vendor>t</vendor><product lang=\"en\">t</product></header>\n"
      " <sim_specs><start>0</start><stop>10</stop><dt>1</dt></sim_specs>\n"
      " <model><variables><aux name=\"" +
      longName +
      "\"><eqn>1</eqn></aux></variables>\n"
      "  <views><view name=\"V\"><aux name=\"" +
      longName +
      "\" x=\"100\" y=\"100\"/></view></views>\n"
      " </model></xmile>\n";

  char *converted = nullptr;
  std::string recWarnings = CaptureStderr([&]() {
    converted = convert_xmile_to_mdl(xmileDoc.c_str(), static_cast<uint32_t>(xmileDoc.size()), "long.xmile",
                                     /*isLongName=*/0);
  });
  CHECK(converted != nullptr);  // the text is emitted intact, never truncated
  if (converted) {
    CHECK(strstr(converted, longName.c_str()) != nullptr);
    free(converted);
  }
  CHECK(recWarnings.find("sketch variable record") != std::string::npos);
  if (recWarnings.find("sketch variable record") == std::string::npos)
    printf("  captured instead: [%s]\n", recWarnings.c_str());
}

// AC4.2 (no view, regression guard): confirm the no-sketch fixture above truly
// produces NO non-empty view on the source side. If the reader ever started
// synthesizing geometry from a sketch-less model, the AC4.2 round-trip test
// would still pass (both sides would gain the same synthetic view) while
// silently violating the "never fabricate geometry" contract. This check makes
// that contract explicit at the source.
TEST(SketchRoundTrip_no_view_has_no_geometry) {
  const std::string mdl =
      "{UTF-8}\r\n"
      "a = 2\r\n\t~~|\r\n"
      "b = 3\r\n\t~~|\r\n"
      "c = a * b\r\n\t~~|\r\n";
  Model *m0 = roundtrip::ParseVensim(mdl);
  CHECK(m0 != nullptr);
  if (!m0)
    return;
  CHECK(FirstNonEmptyView(m0) == nullptr);
  delete m0;
}
