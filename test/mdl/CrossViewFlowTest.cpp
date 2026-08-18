#include <string>
#include <vector>

#include "../../src/Model.h"
#include "../../src/Symbol/Variable.h"
#include "../TestHarness.h"
#include "RoundTrip.h"

// A stock whose flows are drawn in another view than the stock itself.
//
// XMILE's module decomposition (one <model> per view) needs a stock's flows in
// the stock's own <model>, so the writer stands a local "<stock> flow" proxy in
// for each such flow (Variable::LocalizeCrossViewFlows, run from
// XMILEGenerator::Print on the module path only). Nothing else may see those
// proxies: they are not part of the post-parse pipeline, the sector form emits
// the modeler's flows as they are, and the .mdl writer emits none of them --
// which is what keeps .mdl -> .mdl a fixpoint and XMILE -> .mdl comparable to
// its source.
//
// The sketch: view 1 holds stock S with its inflow `in` drawn as a proper
// valve. View 2 holds `out` and `shared` as plain auxes (shape 8) and a second
// stock T whose only inflow is `shared`. So on the module path:
//   - `out` is displaced from S and, listed by no stock any more, goes back to
//     the aux the modeler drew;
//   - `shared` is displaced from S but is still T's same-view inflow, so it
//     stays a flow;
//   - `in` is untouched.
namespace {

const char *kCrossViewMdl =
    R"MDL({UTF-8}
S= INTEG (
	in - out - shared,
		100)
	~
	~		|

T= INTEG (
	shared,
		0)
	~
	~		|

in=
	5
	~
	~		|

out=
	S / 10
	~
	~		|

shared=
	S / 20
	~
	~		|

********************************************************
	.Control
********************************************************~
		Simulation Control Parameters
	|

FINAL TIME  = 10
	~	Month
	~		|

INITIAL TIME  = 0
	~	Month
	~		|

SAVEPER  = TIME STEP
	~	Month
	~		|

TIME STEP  = 1
	~	Month
	~		|

\\\---/// Sketch information - do not modify anything except names
V300  Do not put anything below this section - it will be ignored
*View 1
$192-192-192,0,Times New Roman|12||0-0-0|0-0-0|0-0-255|-1--1--1|-1--1--1|72,72,100,0
10,1,S,307,235,40,20,3,3,0,0,0,0,0,0
12,2,48,150,235,10,8,0,3,0,0,-1,0,0,0
1,3,5,1,4,0,0,22,0,0,0,-1--1--1,,1|(250,235)|
1,4,5,2,100,0,0,22,0,0,0,-1--1--1,,1|(180,235)|
11,5,48,215,235,6,8,34,3,0,0,1,0,0,0
10,6,in,215,251,20,8,40,3,0,0,-1,0,0,0
\\\---/// Sketch information - do not modify anything except names
V300  Do not put anything below this section - it will be ignored
*View 2
$192-192-192,0,Times New Roman|12||0-0-0|0-0-0|0-0-255|-1--1--1|-1--1--1|72,72,100,0
10,1,out,200,150,20,8,8,3,0,0,0,0,0,0
10,2,shared,200,250,30,8,8,3,0,0,0,0,0,0
10,3,T,350,250,40,20,3,3,0,0,0,0,0,0
///---\\\
:L<%^E!@
9:Current
15:0,0,0,0,0,0
19:100,0
27:2,
34:0,
4:Time
5:S
24:0
25:10
26:10
)MDL";

Variable *FindVar(Model *m, const char *name) {
  Symbol *s = m->GetNameSpace()->Find(name);
  if (!s || s->isType() != Symtype_Variable)
    return nullptr;
  return static_cast<Variable *>(s);
}

bool Lists(const std::vector<Variable *> &flows, Variable *v) {
  for (Variable *f : flows) {
    if (f == v)
      return true;
  }
  return false;
}

// Count the module-path proxies present in the main namespace.
int CountProxies(Model *m) {
  int n = 0;
  for (Variable *v : m->GetVariables(nullptr)) {
    if (v->SynthesizedFlowProxy())
      n++;
  }
  return n;
}

// The fixture must actually put the flows in the other view, and the pipeline
// must leave the model exactly as the modeler wrote it, or every assertion
// below passes for the wrong reason.
bool CheckPristine(Model *m) {
  Variable *s = FindVar(m, "S");
  Variable *t = FindVar(m, "T");
  Variable *in = FindVar(m, "in");
  Variable *out = FindVar(m, "out");
  Variable *shared = FindVar(m, "shared");
  CHECK(s && t && in && out && shared);
  if (!s || !t || !in || !out || !shared)
    return false;
  CHECK(s->GetView() != nullptr);
  CHECK(in->GetView() == s->GetView());
  CHECK(out->GetView() != nullptr && out->GetView() != s->GetView());
  CHECK(shared->GetView() == out->GetView());
  CHECK(t->GetView() == out->GetView());
  CHECK(CountProxies(m) == 0);
  CHECK(in->VariableType() == XMILE_Type_FLOW);
  CHECK(out->VariableType() == XMILE_Type_FLOW);
  CHECK(shared->VariableType() == XMILE_Type_FLOW);
  CHECK(Lists(s->Inflows(), in));
  CHECK(Lists(s->Outflows(), out));
  CHECK(Lists(s->Outflows(), shared));
  CHECK(Lists(t->Inflows(), shared));
  return true;
}

// Everything before the sketch marker.
std::string EquationSection(const std::string &mdl) {
  return mdl.substr(0, mdl.find("\\\\\\---///"));
}

}  // namespace

TEST(CrossViewFlow_pipeline_leaves_the_model_as_written) {
  Model *m = roundtrip::ParseVensim(kCrossViewMdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  CheckPristine(m);
  delete m;
}

TEST(CrossViewFlow_module_output_localizes_and_restores_types) {
  Model *m = roundtrip::ParseVensim(kCrossViewMdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  if (!CheckPristine(m)) {
    delete m;
    return;
  }
  m->SetAsSectors(false);
  std::vector<std::string> errs;
  std::string xmile = m->PrintXMILE(/*isCompact=*/false, errs, 1.0, 1.0);
  CHECK(errs.empty());
  CHECK(!xmile.empty());

  Variable *s = FindVar(m, "S");
  Variable *t = FindVar(m, "T");
  Variable *in = FindVar(m, "in");
  Variable *out = FindVar(m, "out");
  Variable *shared = FindVar(m, "shared");

  // Both of S's other-view flows were replaced by proxies in S's own view; the
  // same-view inflow was not.
  CHECK(CountProxies(m) == 2);
  CHECK(Lists(s->Inflows(), in));
  CHECK(!Lists(s->Outflows(), out));
  CHECK(!Lists(s->Outflows(), shared));
  CHECK(s->Outflows().size() == 2);
  for (Variable *p : s->Outflows()) {
    CHECK(p->SynthesizedFlowProxy());
    CHECK(p->GetView() == s->GetView());
    CHECK(p->VariableType() == XMILE_Type_FLOW);
  }
  // T's flow list is untouched: `shared` is drawn in T's view.
  CHECK(t->Inflows().size() == 1 && Lists(t->Inflows(), shared));

  // Typing: `out` is a plain aux again (no stock lists it), `shared` is still
  // T's flow, `in` never moved.
  CHECK(out->VariableType() == XMILE_Type_AUX);
  CHECK(shared->VariableType() == XMILE_Type_FLOW);
  CHECK(in->VariableType() == XMILE_Type_FLOW);

  // The document says the same thing: S's module carries the proxies, and
  // `out` is emitted as an aux, not as an unattached flow.
  CHECK(xmile.find("<flow name=\"S flow\">") != std::string::npos);
  CHECK(xmile.find("<outflow>S_flow</outflow>") != std::string::npos);
  CHECK(xmile.find("<aux name=\"out\">") != std::string::npos);
  CHECK(xmile.find("<flow name=\"out\">") == std::string::npos);
  CHECK(xmile.find("<flow name=\"shared\">") != std::string::npos);

  // Printing again finds nothing new to localize.
  errs.clear();
  std::string again = m->PrintXMILE(/*isCompact=*/false, errs, 1.0, 1.0);
  CHECK(errs.empty());
  CHECK(again == xmile);
  CHECK(CountProxies(m) == 2);
  delete m;
}

TEST(CrossViewFlow_sector_output_keeps_the_modelers_flows) {
  Model *m = roundtrip::ParseVensim(kCrossViewMdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  if (!CheckPristine(m)) {
    delete m;
    return;
  }
  m->SetAsSectors(true);
  std::vector<std::string> errs;
  std::string xmile = m->PrintXMILE(/*isCompact=*/false, errs, 1.0, 1.0);
  CHECK(errs.empty());
  CHECK(!xmile.empty());
  CHECK(CountProxies(m) == 0);
  CHECK(xmile.find(" flow\"") == std::string::npos);
  CHECK(xmile.find("<outflow>out</outflow>") != std::string::npos);
  CHECK(xmile.find("<outflow>shared</outflow>") != std::string::npos);
  CHECK(xmile.find("<flow name=\"out\">") != std::string::npos);
  delete m;
}

TEST(CrossViewFlow_mdl_output_has_no_proxies_and_round_trips) {
  roundtrip::ExpectCleanRoundTrip(kCrossViewMdl);

  Model *m = roundtrip::ParseVensim(kCrossViewMdl);
  CHECK(m != nullptr);
  if (!m)
    return;
  std::vector<std::string> errs;
  std::string mdl = m->PrintMDL(errs);
  CHECK(errs.empty());
  CHECK(mdl.find("S flow") == std::string::npos);
  CHECK(mdl.find("in - out - shared") != std::string::npos);

  // A model that has already been emitted as XMILE modules carries the proxies
  // in memory; the .mdl writer still leaves them out.
  m->SetAsSectors(false);
  errs.clear();
  m->PrintXMILE(/*isCompact=*/false, errs, 1.0, 1.0);
  CHECK(errs.empty());
  CHECK(CountProxies(m) == 2);
  errs.clear();
  std::string mdl2 = m->PrintMDL(errs);
  CHECK(errs.empty());
  CHECK(mdl2.find("S flow") == std::string::npos);
  // Equation section only: PrintXMILE also normalizes the sketch origin in
  // place (see CorpusRoundTripTest), so the sketch records legitimately move.
  CHECK(EquationSection(mdl2) == EquationSection(mdl));
  delete m;
}
