#include "XMILEGenerator.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <map>

#include "../Model.h"
#include "../Symbol/ExpressionList.h"
#include "../Vensim/VensimView.h"
#include "../XMUtil.h"

namespace {

// A group name carries no content when it is empty or all whitespace, and it
// reaches the document as the name attribute of a <module>, a <model> or a
// <group> -- the handle every cross level and every re-import resolves through.
// Both readers can hand the writer one: XmileReader::NormalizeName trims
// `<group name="   ">` down to the empty string, and Model::AdjustGroupNames
// only uniquifies names, it does not require them to be non-empty.
bool IsBlankName(const std::string &name) {
  for (char c : name) {
    if (!std::isspace(static_cast<unsigned char>(c)))
      return false;
  }
  return true;
}

bool GroupNameIsTaken(Model *model, const std::string &name) {
  // A group name becomes a module name in the emitted document, which shares
  // one XMILE name space with the variables -- the same two-sided test
  // Model::AdjustGroupNames applies when it disambiguates colliding groups.
  if (model->GetNameSpace()->Find(name))
    return true;
  for (ModelGroup *group : model->Groups()) {
    if (group->sName == name)
      return true;
  }
  return false;
}

// Give every blank-named group a real name before anything serializes one.
// Substituting beats the two alternatives: emitting name="" ships a document
// whose modules cannot be referenced (and that our own reader reads back as a
// nameless group), and dropping the group would take its variables with it --
// generateModelAsGroups emits equations only through the groups, so a skipped
// group is a set of equations missing from the output.
void SanitizeGroupNames(Model *model) {
  for (ModelGroup *group : model->Groups()) {
    if (!IsBlankName(group->sName))
      continue;
    std::string name = "Unnamed Group";
    for (int suffix = 2; GroupNameIsTaken(model, name); suffix++)
      name = "Unnamed Group " + std::to_string(suffix);
    log("warning: group with a blank name emitted as '%s'\n", name.c_str());
    group->sName = name;
  }
}

// True when generateModelAsGroups will actually write a <model> for this group.
// "Holds variables" is not the same question: the emission loop builds its
// member set by filtering out Unwanted(), which the four control variables all
// are by the time it runs (generateSimSpecs marks them). Answering with
// vVariables.empty() let a `.Control` banner count as populated, so it earned a
// <module> reference and a <model> carrying an empty <variables/> -- a level of
// structure the source never had, which the re-import then reads back as real.
bool GroupIsEmitted(const ModelGroup *group) {
  for (Variable *var : group->vVariables) {
    if (var && !var->Unwanted())
      return true;
  }
  return false;
}

}  // namespace

XMILEGenerator::XMILEGenerator(Model *model, double xratio, double yratio, bool from_dynamo) {
  _model = model;
  _xratio = xratio;
  _yratio = yratio;
  _from_dynamo = from_dynamo;
}

std::string XMILEGenerator::Print(bool is_compact, std::vector<std::string> &errs, bool as_sectors) {
  tinyxml2::XMLDocument doc;

  tinyxml2::XMLElement *root = doc.NewElement("xmile");
  root->SetName("xmile");
  root->SetAttribute("xmlns", "http://docs.oasis-open.org/xmile/ns/XMILE/v1.0");
  root->SetAttribute("xmlns:isee", "http://iseesystems.com/XMILE");
  root->SetAttribute("version", "1.0");
  doc.InsertFirstChild(root);

  tinyxml2::XMLElement *prefs = doc.NewElement("isee:prefs");
  root->InsertEndChild(prefs);
  prefs->SetAttribute("show_module_prefix", "true");
  prefs->SetAttribute("layer", "model");

  tinyxml2::XMLElement *header = doc.NewElement("header");
  this->generateHeader(header, errs);
  root->InsertEndChild(header);

  tinyxml2::XMLElement *specs = doc.NewElement("sim_specs");
  this->generateSimSpecs(specs, errs);
  root->InsertEndChild(specs);

  tinyxml2::XMLElement *model_units = doc.NewElement("model_units");
  this->generateModelUnits(model_units, errs);
  root->InsertEndChild(model_units);

  tinyxml2::XMLElement *dimensions = doc.NewElement("dimensions");
  this->generateDimensions(dimensions, errs);
  root->InsertEndChild(dimensions);
  _model->MakeViewNamesUnique();
  // Both emission paths below name groups: the sectors path writes <group
  // name="...">, the module path writes <module>/<model name="...">. Fix the
  // blanks once here rather than in each, the same way view titles are.
  SanitizeGroupNames(_model);
  if (as_sectors) {
    tinyxml2::XMLElement *model = doc.NewElement("model");
    this->generateModelAsSectors(model, errs, NULL, true);
    root->InsertEndChild(model);
  } else {
    // One <model> per view: a stock's flows have to be defined in the stock's
    // own <model>, so a flow the modeler drew in another view is stood in for
    // by a local proxy. Only this emission form needs that, so it is done here
    // rather than in the post-parse pipeline.
    _model->LocalizeCrossViewFlows();
    this->generateModelAsModules(root, errs, NULL);
  }

  // macros are presented as separate models
  for (MacroFunction *mf : _model->MacroFunctions()) {
    tinyxml2::XMLElement *macro = doc.NewElement("macro");
    macro->SetAttribute("name", mf->GetName().c_str());

    // in vensim the equation is always just the name fo the macro
    tinyxml2::XMLElement *xeqn = doc.NewElement("eqn");
    macro->InsertEndChild(xeqn);
    xeqn->SetText(mf->GetName().c_str());
    // the parms are all of the entries in the macro description
    ExpressionList *args = mf->Args();
    int n = args->Length();
    for (int i = 0; i < n; i++) {
      tinyxml2::XMLElement *xparm = doc.NewElement("parm");
      macro->InsertEndChild(xparm);
      Expression *pexp = args->GetExp(i);
      ContextInfo info(NULL);
      pexp->OutputComputable(&info);
      xparm->SetText(info.str().c_str());
    }

    this->generateModelAsSectors(macro, errs, mf->NameSpace(), false);  // not really a secotr - only a root module here
    root->InsertEndChild(macro);
  }

  tinyxml2::XMLPrinter printer{nullptr, is_compact};
  if (!doc.Accept(&printer)) {
    if (doc.ErrorStr()) {
      errs.push_back("TinyXML2 Error: " + std::string(doc.ErrorStr()));
    }
    return "";
  }

  std::string xmile = printer.CStr();

  return xmile;
}

void XMILEGenerator::generateHeader(tinyxml2::XMLElement *element, std::vector<std::string> &errs) {
  tinyxml2::XMLDocument *doc = element->GetDocument();

  tinyxml2::XMLElement *options = doc->NewElement("options");
  options->SetAttribute("namespace", "std");
  element->InsertEndChild(options);

  if (!_from_dynamo) {
    tinyxml2::XMLElement *vendor = doc->NewElement("vendor");
    vendor->SetText("Ventana Systems, xmutil");
    element->InsertEndChild(vendor);
  }

  tinyxml2::XMLElement *product = doc->NewElement("product");
  product->SetAttribute("lang", "en");
  if (_from_dynamo)
    product->SetText("Dynamo, xmutil");
  else
    product->SetText("Vensim, xmutil");
  element->InsertEndChild(product);
}

void XMILEGenerator::generateSimSpecs(tinyxml2::XMLElement *element, std::vector<std::string> &errs) {
  /*
  <sim_specs method = "Euler" time_units = "Months">
  <start>0< / start>
  <stop>100< / stop>
  <dt>0.125< / dt>
  < / sim_specs>
  */

  tinyxml2::XMLDocument *doc = element->GetDocument();

  if (_model->IntegrationType() == Integration_Type_RK4)
    element->SetAttribute("method", "RK4");
  else if (_model->IntegrationType() == Integration_Type_RK2)
    element->SetAttribute("method", "RK2");
  else
    element->SetAttribute("method", "Euler");

  UnitExpression *uexpr = _model->GetUnits("TIME STEP");
  if (!uexpr)
    uexpr = _model->GetUnits("FINAL TIME");
  if (!uexpr)
    uexpr = _model->GetUnits("INITIAL TIME");
  if (uexpr)
    element->SetAttribute("time_units", uexpr->GetEquationString().c_str());
  else
    element->SetAttribute("time_units", "Months");

  double start = _model->GetConstanValue(
      "INITIAL TIME", _model->initial_time());  // default to 0 if INITIAL TIME is missing or an equation
  double stop = _model->GetConstanValue("FINAL TIME", _model->final_time());
  double dt = _model->GetConstanValue("TIME STEP", _model->dt());
  double saveper = _model->GetConstanValue("SAVEPER", dt);
  double speed = _model->GetConstanValue("SIMULATION PAUSE", 0);

  if (start == -1) {
    if (stop > 200)  // this happens to work for national model - but hey
      start = stop - 200;
    else
      start = 0;
  }
  if (stop <= start)
    stop = start + 10 * dt;

  // isee:sim_duration is Stella's animation-length hint, derived from how many
  // steps get saved. SAVEPER is the modeler's value and is never clamped on the
  // way in, and a zero or non-finite dt reaches here as SAVEPER through the
  // reader's default, so the quotient -- not its divisor -- is where this has to
  // be made safe: "%g" would otherwise print a literal "inf"/"nan" into the
  // attribute. Falling back to the no-animation "0" matches the speed <= 0 arm.
  double duration = (speed > 0) ? (stop - start) / saveper * speed : 0.0;
  if (!std::isfinite(duration))
    duration = 0.0;
  char dur[32];
  snprintf(dur, sizeof(dur), "%g", duration);
  element->SetAttribute("isee:sim_duration", dur);

  tinyxml2::XMLElement *startEle = doc->NewElement("start");
  startEle->SetText(StringFromDouble(start).c_str());
  element->InsertEndChild(startEle);

  tinyxml2::XMLElement *stopEle = doc->NewElement("stop");
  stopEle->SetText(StringFromDouble(stop).c_str());
  element->InsertEndChild(stopEle);

  tinyxml2::XMLElement *dtEle = doc->NewElement("dt");
  dtEle->SetText(StringFromDouble(dt).c_str());
  element->InsertEndChild(dtEle);

  // A reader that finds no save interval defaults SAVEPER to dt, so equality is
  // the only case that needs no attribute: a sub-dt SAVEPER is just as much a
  // modeler-chosen value as a super-dt one, and the old `> dt` test dropped it.
  // isfinite because a NaN SAVEPER compares unequal to everything, dt included,
  // so the difference test alone would emit save_interval="nan" -- which our own
  // reader then rejects on re-import. ShortestDouble instead of std::to_string
  // because the latter's fixed six decimals truncate a small interval to
  // "0.000000" (same reasoning as the table samples below).
  if (std::isfinite(saveper) && saveper != dt)
    element->SetAttribute("isee:save_interval", ShortestDouble(saveper).c_str());

  _model->SetUnwanted("TIME", "TIME");
  _model->SetUnwanted("INITIAL TIME", "STARTTIME");
  _model->SetUnwanted("FINAL TIME", "STOPTIME");
  _model->SetUnwanted("TIME STEP", "DT");
  _model->SetUnwanted("SAVEPER", "SAVEPER");
}

// Emit <model_units>, one <unit> per Model::UnitEquivs() entry:
//
//   <model_units>
//     <unit name="Dollar">
//       <eqn>$</eqn>
//       <alias>Dollars</alias>
//       <alias>$s</alias>
//     </unit>
//   </model_units>
//
// The declaration is already in this shape in memory (see UnitEquiv), so the
// equation stays an <eqn> instead of being guessed back out of a flattened
// list; an empty equation simply emits no <eqn> child.
void XMILEGenerator::generateModelUnits(tinyxml2::XMLElement *element, std::vector<std::string> &errs) {
  tinyxml2::XMLDocument *doc = element->GetDocument();

  for (const UnitEquiv &equiv : _model->UnitEquivs()) {
    tinyxml2::XMLElement *xunit = doc->NewElement("unit");
    xunit->SetAttribute("name", equiv.name.c_str());
    if (!equiv.eqn.empty()) {
      tinyxml2::XMLElement *xeqn = doc->NewElement("eqn");
      xeqn->SetText(equiv.eqn.c_str());
      xunit->InsertEndChild(xeqn);
    }
    for (const std::string &alias : equiv.aliases) {
      // An empty alias is skipped for the same reason an empty eqn is: the
      // reader resolves an alias through GetText(), which tinyxml2 reports as
      // null for an empty element, so <alias></alias> would not survive its own
      // re-read -- and emitting one breaks the XMILE->XMILE fixpoint guarantee.
      // A degenerate `22:a,,b` field therefore collapses on the .mdl path, which
      // the flat Vensim list cannot represent losslessly either way.
      if (alias.empty())
        continue;
      tinyxml2::XMLElement *xalias = doc->NewElement("alias");
      xalias->SetText(alias.c_str());
      xunit->InsertEndChild(xalias);
    }
    element->InsertEndChild(xunit);
  }
}

void XMILEGenerator::generateDimensions(tinyxml2::XMLElement *element, std::vector<std::string> &errs) {
  tinyxml2::XMLDocument *doc = element->GetDocument();
  std::vector<Variable *> vars = _model->GetVariables();  // all symbols that are variables
  for (Variable *var : vars) {
    if (var->VariableType() == XMILE_Type_ARRAY) {
      // simple minded - defining equation -
      Equation *eq = var->GetEquation(0);
      if (eq) {
        Expression *exp = eq->GetExpression();
        if (exp && exp->GetType() == EXPTYPE_Symlist) {
          SymbolList *symlist = static_cast<ExpressionSymbolList *>(exp)->SymList();
          std::vector<Symbol *> expanded;
          int n = symlist->Length();
          for (int i = 0; i < n; i++) {
            const SymbolList::SymbolListEntry &elm = (*symlist)[i];
            if (elm.eType == SymbolList::EntryType_SYMBOL) {
              Equation::GetSubscriptElements(expanded, elm.u.pSymbol);
            }
          }
          // we define subranges as if they were arrays themselves - because of the unique namespace in XMILE this
          // is proper - and it will make any model with partial definitions more or less okay -
          if (!expanded.empty() /* && expanded[0]->Owner() == var*/) {
            tinyxml2::XMLElement *xsub = doc->NewElement("dim");
            xsub->SetAttribute("name", var->GetName().c_str());
            for (Symbol *s : expanded) {
              tinyxml2::XMLElement *xelm = doc->NewElement("elem");
              xelm->SetAttribute("name", s->GetName().c_str());
              xsub->InsertEndChild(xelm);
            }
            element->InsertEndChild(xsub);
          }
        }
      }
    }
  }
}

// first pass if flat - we probably want to do this differently when we break up into modules
void XMILEGenerator::generateModelAsSectors(tinyxml2::XMLElement *element, std::vector<std::string> &errs,
                                            SymbolNameSpace *ns, bool want_diagram) {
  tinyxml2::XMLDocument *doc = element->GetDocument();
  tinyxml2::XMLElement *variables = doc->NewElement("variables");
  element->InsertEndChild(variables);

  std::vector<Variable *> vars = _model->GetVariables(ns);  // all symbols that are variables
  for (Variable *var : vars) {
    if (var->Unwanted())
      continue;
    generateEquation(var, doc, variables);
  }
  if (want_diagram) {
    tinyxml2::XMLElement *views = doc->NewElement("views");
    this->generateSectorViews(views, variables, errs, ns == NULL);
    element->InsertEndChild(views);
  }
}

void XMILEGenerator::generateEquations(std::set<Variable *, SymbolNameLess> &included, tinyxml2::XMLDocument *doc,
                                       tinyxml2::XMLElement *variables) {
  for (Variable *var : included) {
    generateEquation(var, doc, variables);
  }
}

void XMILEGenerator::generateEquation(Variable *var, tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *variables) {
  XMILE_Type type = var->VariableType();
  std::string tag;
  switch (type) {
  case XMILE_Type_DELAYAUX:
  case XMILE_Type_AUX:
    tag = "aux";
    break;
  case XMILE_Type_STOCK:
    tag = "stock";
    break;
  case XMILE_Type_FLOW:
    tag = "flow";
    break;
  case XMILE_Type_ARRAY:
    return;
  case XMILE_Type_ARRAY_ELM:
    return;
  default:
    return;
    break;
  }
  tinyxml2::XMLElement *xvar = doc->NewElement(tag.c_str());

  variables->InsertEndChild(xvar);
  xvar->SetAttribute("name", var->GetAlternateName().c_str());

  if (type == XMILE_Type_DELAYAUX) {
    tinyxml2::XMLElement *xcomment = doc->NewElement("isee:delay_aux");
    xvar->InsertEndChild(xcomment);
  }

  std::vector<Equation *> eqns;
  // for vensim models init equations will always be empty
  std::vector<Equation *> init_eqns = var->GetAllInitEquations();
  bool wrap_init = false;
  if (init_eqns.empty())
    eqns = var->GetAllEquations();
  else if (type == XMILE_Type_STOCK)
    eqns.swap(init_eqns);  // init_eqns is used for init values of aux otherwise
  else {
    eqns = var->GetAllEquations();
    if (eqns.empty()) {
      eqns.swap(init_eqns);  // dynamo convention can have N equations which should translate to INIT
      wrap_init = true;
    }
  }
  size_t eq_count = eqns.size();

  // dimensions
  std::vector<Variable *> elmlist;
  int dim_count = var->SubscriptCountVars(elmlist);

  std::string comment = var->Comment();
  if (!comment.empty()) {
    tinyxml2::XMLElement *xcomment = doc->NewElement("doc");
    xvar->InsertEndChild(xcomment);
    xcomment->SetText(comment.c_str());
  }
  if (type == XMILE_Type_STOCK) {
    for (Variable *in : var->Inflows()) {
      tinyxml2::XMLElement *inflow = doc->NewElement("inflow");
      xvar->InsertEndChild(inflow);
      inflow->SetText(SpaceToUnderBar(in->GetAlternateName()).c_str());
    }
    for (Variable *out : var->Outflows()) {
      tinyxml2::XMLElement *outflow = doc->NewElement("outflow");
      xvar->InsertEndChild(outflow);
      outflow->SetText(SpaceToUnderBar(out->GetAlternateName()).c_str());
    }
  }

  tinyxml2::XMLElement *xelement = xvar;  // usually these are the same - but for non a2a we have element entries
  size_t eq_ind = 0;
  size_t eq_pos = 0;
  std::vector<Symbol *> subs;               // [ship,location]
  std::vector<std::vector<Symbol *>> elms;  // [s1,l1]
  std::vector<std::set<Symbol *, SymbolNameLess>> entries;
  std::vector<Symbol *> dims;
  while (eq_ind < eq_count) {
    Equation *eqn = eqns[eq_ind];
    if (eq_count > 1) {
      if (entries.empty())
        entries.resize(dim_count);
      // we will blow up everything to single elements
      if (elms.empty()) {
        eq_pos = 0;
        elms.clear();
        eqn->SubscriptExpand(elms, subs);
        if (!elms.empty()) {
          for (std::vector<Symbol *> elm : elms) {
            for (int i = 0; i < dim_count; i++) {
              entries[i].insert(elm[i]);
            }
          }
        }
      }
      if (!elms.empty()) {
        dims = elms[eq_pos];
        std::string s;
        int dim_count = dims.size();
        for (int j = 0; j < dim_count; j++) {
          if (j)
            s += ", ";
          s += dims[j]->GetName();
        }
        xelement = doc->NewElement("element");
        xelement->SetAttribute("subscript", s.c_str());
        xvar->InsertEndChild(xelement);
      }
    }
    // skip it altogether if it is an A FUNCTION OF equation
    std::string rhs = eqn->RHSFormattedXMILE(var, subs, dims, false);
    // Standalone graphical-function variable: the equation's RHS *is* the
    // ExpressionTable (token '(' from VensimParse::AddTable, mirrored by the
    // XMILE reader for an <aux> carrying only <gf>). Emitting the placeholder
    // "0+0" <eqn> body would make the re-parse misread the variable as a
    // WITH LOOKUP form. Suppress <eqn> here; the <gf> emission below carries
    // the full meaning.

    // the XMILE Spec suggests a <gf> with no equation is okay, but in practice Stella would take
    // it and turn the equation to Time - so we keep the 0+0 inserted upstream as the equation
    // and the treat that as no equation on reading in xmile
    // bool standalone_gf = eqn->GetExpression() && eqn->GetExpression()->GetType() == EXPTYPE_Table;
    if (eq_count <= 1 || rhs.size() < 42 || rhs.substr(28, 13) != "A FUNCTION OF") {
      /* if (!standalone_gf) */ {
        tinyxml2::XMLElement *xeqn = doc->NewElement("eqn");
        xelement->InsertEndChild(xeqn);
        if (wrap_init)
          rhs = "INIT(" + rhs + ")";
        xeqn->SetText(rhs.c_str());
      }

      // it it is active init we need to store that separately
      if (eqn->IsActiveInit() || !init_eqns.empty()) {
        tinyxml2::XMLElement *xieqn = doc->NewElement("init_eqn");
        xelement->InsertEndChild(xieqn);
        if (eqn->IsActiveInit())
          xieqn->SetText(eqn->RHSFormattedXMILE(var, subs, dims, true).c_str());
        else {
          // this will not work correctly for complicated situations
          if (eq_ind < init_eqns.size())
            xieqn->SetText(init_eqns[eq_ind]->RHSFormattedXMILE(var, subs, dims, true).c_str());
          else
            xieqn->SetText(init_eqns[0]->RHSFormattedXMILE(var, subs, dims, true).c_str());
        }
      }

      // if it has a lookup we need to store that separately
      ExpressionTable *et = eqn->GetTable();
      if (et) {
        assert(type == XMILE_Type_AUX || type == XMILE_Type_FLOW);
        std::vector<double> *xvals = et->GetXVals();
        std::vector<double> *yvals = et->GetYVals();
        tinyxml2::XMLElement *gf = doc->NewElement("gf");
        if (et->Extrapolate())
          gf->SetAttribute("type", "extrapolate");
        xelement->InsertEndChild(gf);
        tinyxml2::XMLElement *yscale = doc->NewElement("yscale");
        gf->InsertEndChild(yscale);
        tinyxml2::XMLElement *xpts = doc->NewElement("xpts");
        gf->InsertEndChild(xpts);
        tinyxml2::XMLElement *ypts = doc->NewElement("ypts");
        gf->InsertEndChild(ypts);

        if (xvals->empty()) {
          // most likely a translation error - fill in with 0,1 based on ysize
          log("WARNING The graphical %s does not have a known x axis assuming 0,1", var->GetAlternateName().c_str());
          int n = yvals->size();
          if (n) {
            double increment = 1.0 / (double)n;
            double val = 0;
            for (int i = 0; i < n; i++, val += increment) {
              xvals->push_back(val);
            }
          }
        }
        // Table samples use ShortestDouble, not StringFromDouble's lossy "%g":
        // a re-parse must recover the identical doubles or the round trip
        // drifts. Sketch coordinates elsewhere can stay on "%g".
        std::string xstr;
        for (size_t i = 0; i < xvals->size(); i++) {
          if (i)
            xstr += ",";
          xstr += ShortestDouble((*xvals)[i]);
        }
        xpts->SetText(xstr.c_str());

        std::string ystr;
        double ymin = 0;
        double ymax = 0;
        for (size_t i = 0; i < yvals->size(); i++) {
          if (i) {
            ystr += ",";
            if ((*yvals)[i] < ymin)
              ymin = (*yvals)[i];
            else if ((*yvals)[i] > ymax)
              ymax = (*yvals)[i];
          } else
            ymin = ymax = (*yvals)[i];
          ystr += ShortestDouble((*yvals)[i]);
        }
        ypts->SetText(ystr.c_str());

        if (ymin == ymax)
          ymax = ymin + 1;
        yscale->SetAttribute("min", StringFromDouble(ymin).c_str());
        yscale->SetAttribute("max", StringFromDouble(ymax).c_str());
      }
    }
    if (eq_count > 1) {
      eq_pos++;
      if (eq_pos >= elms.size()) {
        elms.clear();
        eq_ind++;
      }
    } else
      eq_ind++;
  }

  // use entries to try to figure out the appropriate dimensions
  if (dim_count) {
    // Vensim allowed partial definition sets - XMILE uses subranges as separate dimensions so we
    // try to find the most compact set of dimensions possible that inlcude all the equations include
    std::vector<Variable *> dimensions;

    tinyxml2::XMLElement *xdims = doc->NewElement("dimensions");
    for (int i = 0; i < dim_count; i++) {
      tinyxml2::XMLElement *xdim = doc->NewElement("dim");
      if (entries.empty()) {
        // we might get a subrange in elmlist so need to get parent - but only if there is more than 1 equation
        if (eq_count > 1 || elmlist[i]->GetAllEquations().empty())
          xdim->SetAttribute("name", elmlist[i]->Owner()->GetName().c_str());
        else
          xdim->SetAttribute("name", elmlist[i]->GetName().c_str());
      } else {
        std::set<Symbol *, SymbolNameLess> &entry = entries[i];
        Symbol *parent = (*entry.begin())->Owner();
        Symbol *best = parent;
        if (parent->Subranges() != NULL && static_cast<Variable *>(parent)->Nelm() > entry.size()) {
          // Subranges() is a pointer-keyed set; ties between equally sized
          // complete subranges are broken by iteration order, which reaches
          // the emitted dim name. Sort by name so the choice is deterministic.
          std::vector<Symbol *> subranges(parent->Subranges()->begin(), parent->Subranges()->end());
          std::sort(subranges.begin(), subranges.end(), SymbolNameLess());
          for (Symbol *subrange : subranges) {
            if (static_cast<Variable *>(subrange)->Nelm() >= entry.size() &&
                static_cast<Variable *>(subrange)->Nelm() < static_cast<Variable *>(best)->Nelm()) {
              // does it have them all
              bool complete = true;
              std::vector<Symbol *> telms;
              Equation::GetSubscriptElements(telms, subrange);
              for (Symbol *elm : entries[i]) {
                if (std::find(telms.begin(), telms.end(), elm) == telms.end()) {
                  complete = false;
                  break;
                }
              }
              if (complete)
                best = subrange;
            }
          }
        }
        xdim->SetAttribute("name", best->GetName().c_str());
      }
      xdims->InsertEndChild(xdim);
    }
    xvar->InsertEndChild(xdims);
  }

  UnitExpression *un = var->Units();
  if (un) {
    tinyxml2::XMLElement *units = doc->NewElement("units");
    xvar->InsertEndChild(units);
    units->SetText(un->GetEquationString().c_str());
  } else if (!var->GetUnitsString().empty()) {
    tinyxml2::XMLElement *units = doc->NewElement("units");
    xvar->InsertEndChild(units);
    units->SetText(var->GetUnitsString().c_str());
  }
}

void XMILEGenerator::generateModelAsModules(tinyxml2::XMLElement *element, std::vector<std::string> &errs,
                                            SymbolNameSpace *ns) {
  // we will leave the base model completely empty and only generate the modules - letting the opening software
  // lay out the modules and mark the connections between them
  // we will also leave in anything that does not exist in any view at a reasonable level of
  // attachment
  std::vector<View *> &views = _model->Views();
  tinyxml2::XMLDocument *doc = element->GetDocument();
  if (views.size() < 2) {
    if (generateModelAsGroups(element, errs, ns))
      return;
    tinyxml2::XMLElement *model = doc->NewElement("model");
    generateModelAsSectors(model, errs, ns, true);
    element->InsertEndChild(model);
    return;
  }
  std::vector<Variable *> vars = _model->GetVariables(ns);  // all symbols that are variables
  // for every variable in a view, put its causes into the same view if they are not in any view
  // otherwise we let the go cross level
  for (Variable *var : vars) {
    var->SetViewOfCauses();
  }
  // and now for any variables still not assigned a view we do essentially the opposite
  for (Variable *var : vars) {
    var->SetViewToCause(5);
  }

  tinyxml2::XMLElement *mainmodel = doc->NewElement("model");
  element->InsertEndChild(mainmodel);
  tinyxml2::XMLElement *modules = doc->NewElement("variables");
  mainmodel->InsertEndChild(modules);
  int uid_off = 0;

  for (View *gview : views) {
    if (gview->empty())
      continue;
    VensimView *view = static_cast<VensimView *>(gview);
    tinyxml2::XMLElement *submodel = doc->NewElement("model");
    // The title is non-empty because Print ran Model::MakeViewNamesUnique,
    // which substitutes a placeholder for a blank one and disambiguates the
    // rest -- the view-side counterpart of SanitizeGroupNames. This used to
    // assert it instead, which turns a document a reader accepted into an
    // abort in a debug build and into name="" in a release build.
    submodel->SetAttribute("name", view->Title().c_str());
    element->InsertEndChild(submodel);

    tinyxml2::XMLElement *variables = doc->NewElement("variables");
    submodel->InsertEndChild(variables);
    // first we get a list of variables included in the view and all of their inputs - if an
    // input is not in the view we need to make a ghost to hold its place
    std::set<Variable *, SymbolNameLess> included;
    for (Variable *var : vars) {
      if (var->Unwanted())
        continue;
      if (var->GetView() == gview)  // wanted normal
        included.insert(var);
    }
    std::set<Variable *, SymbolNameLess> needed;
    for (Variable *var : included) {
      std::vector<Variable *> inputs = var->GetInputVars();
      for (Variable *input : inputs) {
        if (included.find(input) == included.end() && input->VariableType() != XMILE_Type_ARRAY &&
            input->VariableType() != XMILE_Type_ARRAY_ELM && input->VariableType() != XMILE_Type_UNKNOWN &&
            !StringMatch(input->GetName(), "Time") && !input->Unwanted()) {
          needed.insert(input);
        }
      }
    }

    // needed will be cross level
    // add this as a module to the main model
    tinyxml2::XMLElement *module = doc->NewElement("module");
    module->SetAttribute("name", view->Title().c_str());
    modules->InsertEndChild(module);
    // and mark the cross levels
    for (Variable *var : needed) {
      tinyxml2::XMLElement *connect = doc->NewElement("connect");
      std::string to = view->Title() + "." + var->GetAlternateName();
      connect->SetAttribute("to", SpaceToUnderBar(to).c_str());
      std::string from;
      if (var->GetView())
        from = static_cast<VensimView *>(var->GetView())->Title();
      from += "." + var->GetAlternateName();
      connect->SetAttribute("from", SpaceToUnderBar(from).c_str());
      module->InsertEndChild(connect);
    }
    generateEquations(included, doc, variables);
    // for the incomming cross levels they get no equations but need to be included as variables
    for (Variable *var : needed) {
      tinyxml2::XMLElement *xvar;
      if (var->VariableType() == XMILE_Type_STOCK)
        xvar = doc->NewElement("stock");
      else
        xvar = doc->NewElement("aux");

      variables->InsertEndChild(xvar);
      xvar->SetAttribute("name", var->GetAlternateName().c_str());
      xvar->SetAttribute("access", "input");
    }

    tinyxml2::XMLElement *xviews = doc->NewElement("views");
    submodel->InsertEndChild(xviews);
    tinyxml2::XMLElement *xview = doc->NewElement("view");
    if (_model->LetterPolarity())
      xview->SetAttribute("isee:use_lettered_polarity", "true");
    xviews->InsertEndChild(xview);
    uid_off = view->SetViewStart(100, 100, _xratio, _yratio, uid_off);
    this->generateView(view, xview, errs, &needed);
  }
  std::set<Variable *, SymbolNameLess> remnant;
  for (Variable *var : vars) {
    if (var->GetView() == NULL && !var->Unwanted())
      remnant.insert(var);
  }
  generateEquations(remnant, doc, modules);
  // tinyxml2::XMLElement* views = doc->NewElement("views");
  // this->generateSectorViews(views, variables, errs, ns == NULL);
  // element->InsertEndChild(views);
}

bool XMILEGenerator::generateModelAsGroups(tinyxml2::XMLElement *element, std::vector<std::string> &errs,
                                           SymbolNameSpace *ns) {
  tinyxml2::XMLDocument *doc = element->GetDocument();
  std::vector<ModelGroup *> &groups = _model->Groups();
  int used_groups = 0;
  for (ModelGroup *group : groups) {
    if (GroupIsEmitted(group))
      used_groups++;
  }
  if (used_groups < 2)
    return false;

  groups.push_back(new ModelGroup("Other not Grouped", NULL));
  // we use addess of group for simplicity
  std::vector<Variable *> vars = _model->GetVariables(ns);  // all symbols that are variables
  // for every variable in a view, put its causes into the same view if they are not in any view
  // otherwise we let the go cross level
  for (Variable *var : vars) {
    if (!var->GetGroup() && var->VariableType() != XMILE_Type_ARRAY && var->VariableType() != XMILE_Type_ARRAY_ELM &&
        var->VariableType() != XMILE_Type_UNKNOWN) {
      var->SetGroup(groups.back());
      groups.back()->vVariables.push_back(var);
    }
  }
  // all vars are now grouped

  // A group that holds no variables is not emitted at all below (no <model>, so
  // no <variables> list to hang anything from), which makes it unusable as an
  // owner: the two loops that follow would give its children no <module>, and
  // the cross-level <connect> loop then dereferences a NULL pModule. Both
  // readers produce that shape from ordinary input -- a Vensim group banner
  // with no equations under it chains the NEXT banner's group to it
  // (VensimParse::ProcessFile), and an XMILE <group> can lose its last member
  // to a competing claim. So re-point every owner at the nearest ancestor that
  // really is emitted. The link is only ever SHORTENED, never dropped, so every
  // variable still names a module that exists and no <connect> is lost; what a
  // contentless level of nesting expressed is not expressible here anyway.
  //
  // The walk is bounded rather than a plain `while (!GroupIsEmitted(owner))`:
  // nothing re-validates a Model on the way into the writer, and a pOwner cycle
  // -- which the XMILE reader refuses to build and the Vensim reader cannot,
  // but a future producer might -- would otherwise spin forever instead of
  // saying so. A simple chain visits each group at most once, so passing that
  // count means the chain closed on itself.
  std::map<ModelGroup *, ModelGroup *> effective_owner;
  for (ModelGroup *group : groups) {
    ModelGroup *owner = group->pOwner;
    bool cycles = false;
    for (size_t hops = 0; owner && (owner == group || !GroupIsEmitted(owner)); hops++) {
      if (hops >= groups.size()) {
        cycles = true;
        break;
      }
      owner = owner->pOwner;
    }
    if (cycles) {
      log("warning: the owner chain above group '%s' never terminates (a cycle); emitting it at the top level\n",
          group->sName.c_str());
      owner = NULL;
    }
    effective_owner[group] = owner;
  }

  tinyxml2::XMLElement *mainmodel = doc->NewElement("model");
  element->InsertEndChild(mainmodel);
  tinyxml2::XMLElement *mainvariables = doc->NewElement("variables");
  mainmodel->InsertEndChild(mainvariables);
  for (ModelGroup *group : groups) {
    if (GroupIsEmitted(group) && effective_owner[group] == NULL) {
      group->pModule = doc->NewElement("module");
      group->pModule->SetAttribute("name", group->sName.c_str());
      mainvariables->InsertEndChild(group->pModule);
    }
  }

  // set up the module constucts - these need to be filled in before any cross levels are created
  for (ModelGroup *group : groups) {
    if (!GroupIsEmitted(group))
      continue;
    group->pModel = doc->NewElement("model");
    group->pModel->SetAttribute("name", group->sName.c_str());
    element->InsertEndChild(group->pModel);
    group->pVariables = doc->NewElement("variables");
    group->pModel->InsertEndChild(group->pVariables);
    //  find any modules that we own
    for (ModelGroup *sgroup : groups) {
      // Emptiness is tested here and not only in the loop above because a
      // <module> whose <model name="..."> is never written is a reference to
      // nothing -- the same reason an empty group gets no top-level module.
      if (GroupIsEmitted(sgroup) && effective_owner[sgroup] == group) {
        sgroup->pModule = doc->NewElement("module");
        sgroup->pModule->SetAttribute("name", sgroup->sName.c_str());
        group->pVariables->InsertEndChild(sgroup->pModule);
      }
    }
  }
  // now output cross levels and actual variables
  for (ModelGroup *group : groups) {
    if (!GroupIsEmitted(group))
      continue;
    // first we get a list of variables included in the view and all of their inputs - if an
    // input is not in the view we need to make a ghost to hold its place
    std::set<Variable *, SymbolNameLess> included;
    for (Variable *var : group->vVariables) {
      if (var->Unwanted())
        continue;
      included.insert(var);
    }
    std::set<Variable *, SymbolNameLess> needed;
    for (Variable *var : included) {
      std::vector<Variable *> inputs = var->GetInputVars();
      for (Variable *input : inputs) {
        if (included.find(input) == included.end() && input->VariableType() != XMILE_Type_ARRAY &&
            input->VariableType() != XMILE_Type_ARRAY_ELM && input->VariableType() != XMILE_Type_UNKNOWN &&
            !StringMatch(input->GetName(), "Time") && !input->Unwanted()) {
          needed.insert(input);
        }
      }
      inputs = var->GetInitInputVars();
      for (Variable *input : inputs) {
        if (included.find(input) == included.end() && input->VariableType() != XMILE_Type_ARRAY &&
            input->VariableType() != XMILE_Type_ARRAY_ELM && input->VariableType() != XMILE_Type_UNKNOWN &&
            !StringMatch(input->GetName(), "Time") && !input->Unwanted()) {
          needed.insert(input);
        }
      }
    }

    // needed will be cross level

    // and mark the cross levels
    for (Variable *var : needed) {
      // Every variable that reaches here was put in a group above, and every
      // group that holds a variable was given a pModule above -- a cross level
      // names the module it comes FROM, so neither can be missing without the
      // reference being unwritable. Say so and drop the one connect rather than
      // dereferencing NULL, in case a later change breaks either invariant.
      ModelGroup *source = var->GetGroup();
      if (!source || !source->pModule) {
        log("warning: '%s' is used by group '%s' but belongs to no emitted module; dropping the cross level\n",
            var->GetName().c_str(), group->sName.c_str());
        continue;
      }
      tinyxml2::XMLElement *connect = doc->NewElement("connect");
      std::string to = group->sName + "." + var->GetAlternateName();
      connect->SetAttribute("to", SpaceToUnderBar(to).c_str());
      std::string from = source->sName;
      from += "." + var->GetAlternateName();
      connect->SetAttribute("from", SpaceToUnderBar(from).c_str());
      source->pModule->InsertEndChild(connect);  // receiving module
    }
    generateEquations(included, doc, group->pVariables);
    // for the incomming cross levels they get no equations but need to be included as variables
    for (Variable *var : needed) {
      tinyxml2::XMLElement *xvar;
      if (var->VariableType() == XMILE_Type_STOCK)
        xvar = doc->NewElement("stock");
      else
        xvar = doc->NewElement("aux");

      group->pVariables->InsertEndChild(xvar);
      xvar->SetAttribute("name", var->GetAlternateName().c_str());
      xvar->SetAttribute("access", "input");  // let the receiving softwwre take care out output access
    }
  }
  return true;
}

void XMILEGenerator::generateSectorViews(tinyxml2::XMLElement *element, tinyxml2::XMLElement *xvars,
                                         std::vector<std::string> &errs, bool mainmodel) {
  tinyxml2::XMLDocument *doc = element->GetDocument();

  std::vector<View *> &views = _model->Views();
  if (views.empty() && mainmodel) {
    std::vector<ModelGroup *> &groups = _model->Groups();
    if (!groups.empty()) {
      for (ModelGroup *group : groups) {
        tinyxml2::XMLElement *xgroup = doc->NewElement("group");
        xgroup->SetAttribute("name", group->sName.c_str());
        if (group->pOwner)
          xgroup->SetAttribute("owner", group->pOwner->sName.c_str());
        element->InsertEndChild(xgroup);
        for (Variable *var : group->vVariables) {
          tinyxml2::XMLElement *xvar = doc->NewElement("var");
          xvar->SetText(SpaceToUnderBar(var->GetAlternateName()).c_str());
          xgroup->InsertEndChild(xvar);
        }
      }
    }
    return;
  }
  int x, y;
  // start at a reasonable distance from 0 - the x,y values are generally around hte center
  // of the var
  x = 100;
  y = 100;
  // all the views against a single xmile view - or break up into modules - need vector of models as input to do that
  tinyxml2::XMLElement *xview = doc->NewElement("view");
  if (_model->LetterPolarity())
    xview->SetAttribute("isee:use_lettered_polarity", "true");
  element->InsertEndChild(xview);
  int uid_off = 0;
  for (View *gview : views) {
    VensimView *view = static_cast<VensimView *>(gview);
    // first update geometry - we put views one after another along the y axix - could lay out in pages or something
    uid_off = view->SetViewStart(x, y + 20, _xratio, _yratio, uid_off);
    int width = view->GetViewMaxX(100);
    int height = view->GetViewMaxY(y + 80) - y;
    // add a surrounding sector to contain this view - call it the view name
    // 				<group locked="false" x="184" y="154" width="300" height="184" name="Sector 1"/>

    if (views.size() > 1) {
      std::string name = view->Title();
      tinyxml2::XMLElement *xsectorvar = doc->NewElement("group");
      xvars->InsertEndChild(xsectorvar);
      xsectorvar->SetAttribute("name", name.c_str());
      tinyxml2::XMLElement *xsector = doc->NewElement("group");
      xview->InsertEndChild(xsector);
      xsector->SetAttribute("name", name.c_str());
      xsector->SetAttribute("x", StringFromDouble(x - 40).c_str());
      xsector->SetAttribute("y", StringFromDouble(y).c_str());
      xsector->SetAttribute("width", StringFromDouble(width + 60).c_str());
      xsector->SetAttribute("height", StringFromDouble(height + 40).c_str());
    }

    y += height + 80;

    this->generateView(view, xview, errs, NULL);
  }
}

void XMILEGenerator::generateView(VensimView *view, tinyxml2::XMLElement *element, std::vector<std::string> &errs,
                                  std::set<Variable *, SymbolNameLess> *adds) {
  tinyxml2::XMLDocument *doc = element->GetDocument();
  VensimViewElements &elements = view->Elements();
  // Every lookup into `elements` by a UID goes through here. The vector is
  // indexed by on-wire UID and is SPARSE by construction -- VensimView::ReadView
  // leaves a NULL in every slot no record claims -- while the indices themselves
  // (a connector's From()/To(), and the valve+1 flow pairing) come straight off
  // the wire, so a hand-authored or truncated sketch can name an empty slot or
  // one past the end. Both used to be dereferenced unchecked, which is a plain
  // segfault on ordinary user input.
  auto at = [&elements](int index) -> VensimViewElement * {
    if (index < 0 || static_cast<size_t>(index) >= elements.size())
      return NULL;
    return elements[index];
  };
  // local_uid and uid are DERIVED from the loop index rather than counted with
  // it. Counting them meant every `continue` in the body had to remember to
  // advance both, and one did not: the `tag.empty()` skip for an element whose
  // variable has no XMILE tag (an untyped variable, an array) left both one
  // behind for the whole rest of the view. Two things broke as a result.
  // The flow pairing at local_uid - 1 and the `From() == local_uid - 1` pipe
  // predicate looked at the wrong element, so a flow silently got a fabricated
  // straight pipe instead of its real geometry. And `uid` has to equal
  // UIDOffset + array index, because that is what a connector's
  // `<from><alias uid="UIDOffset + cele->From()"/>` computes -- so once the two
  // disagreed, an emitted alias reference pointed at a uid no `<alias>` element
  // carried. A derived index cannot drift.
  for (size_t index = 0; index < elements.size(); index++) {
    const int local_uid = static_cast<int>(index);
    const int uid = view->UIDOffset() + local_uid;
    VensimViewElement *ele = elements[index];
    if (ele) {
      if (ele->Type() == VensimViewElement::ElementTypeVARIABLE) {
        assert(ele->X() > 0 && ele->Y() > 0);
        VensimVariableElement *vele = static_cast<VensimVariableElement *>(ele);
        Variable *var = vele->GetVariable();
        // skip time altogether - this never shows up under xmil
        if (!var || StringMatch(vele->GetVariable()->GetName(), "Time") || var->Unwanted())
          ;  // do nothing
        else if (vele->Ghost(adds, true)) {
          assert(vele->GetVariable()->VariableType() != XMILE_Type_ARRAY);
          tinyxml2::XMLElement *xghost = doc->NewElement("alias");
          element->InsertEndChild(xghost);
          xghost->SetAttribute("x", vele->X());
          xghost->SetAttribute("y", vele->Y());
          if (vele->GetVariable() && vele->GetVariable()->VariableType() == XMILE_Type_STOCK) {
            xghost->SetAttribute("x", vele->X() - 22);
            xghost->SetAttribute("y", vele->Y() - 17);
            xghost->SetAttribute("width", 45);
            xghost->SetAttribute("height", 35);
          } else {
            xghost->SetAttribute("x", vele->X());
            xghost->SetAttribute("y", vele->Y());
          }
          xghost->SetAttribute("uid", uid);
          tinyxml2::XMLElement *xof = doc->NewElement("of");
          xghost->InsertEndChild(xof);
          xof->SetText(SpaceToUnderBar(vele->GetVariable()->GetAlternateName()).c_str());
        } else {
          XMILE_Type type = vele->GetVariable()->VariableType();
          std::string tag;
          switch (type) {
          case XMILE_Type_DELAYAUX:
          case XMILE_Type_AUX:
            tag = "aux";
            break;
          case XMILE_Type_STOCK:
            tag = "stock";
            break;
          case XMILE_Type_FLOW:
            tag = "flow";
            break;
          default:
            log("unknown view element type %d\n", type);
          }
          if (tag.empty())
            continue;
          tinyxml2::XMLElement *xvar = doc->NewElement(tag.c_str());

          element->InsertEndChild(xvar);

          std::string name = vele->GetVariable()->GetAlternateName();
          xvar->SetAttribute("name", SpaceToUnderBar(vele->GetVariable()->GetAlternateName()).c_str());
          // The valve a flow is attached to sits one slot earlier. That slot can
          // be absent (an unpaired flow record) and, at local_uid 0, is not even
          // a legal index.
          VensimViewElement *valve = at(local_uid - 1);
          if (type == XMILE_Type_FLOW && vele->Attached() && valve &&
              valve->Type() == VensimViewElement::ElementTypeVALVE) {
            xvar->SetAttribute("x", valve->X());
            xvar->SetAttribute("y", valve->Y());
          } else {
            // pretty big things - Vensim's default size is 80x40 - width and height are half vals so a fair bit bigger
            // 90x50 then bring size across
            if (type == XMILE_Type_STOCK && !vele->CrossLevel() && !vele->Ghost(NULL, false) &&
                (vele->Width() > 45 || vele->Height() > 25)) {
              int x = vele->X();
              int y = vele->Y();
              int width = 2 * vele->Width();
              int height = 2 * vele->Height();
              if (width < 60)
                width = 60;
              if (height < 40)
                height = 40;
              x -= width / 2;
              y -= height / 2;
              xvar->SetAttribute("x", x);
              xvar->SetAttribute("y", y);
              xvar->SetAttribute("width", width);
              xvar->SetAttribute("height", height);
            } else {
              xvar->SetAttribute("x", vele->X());
              xvar->SetAttribute("y", vele->Y());
            }
          }
          if (type == XMILE_Type_FLOW) {
            // need points - these are the location of the from and to - no matter what they are
            // but we need to search through the list of eleemnts to find the from and to - flow
            // arrows are always out of the attached value which is just before us in the list
            // flow direction we need to take from the model proper - arbitrary if flow is not connected
            size_t n = elements.size();
            int count = 0;
            int toind = -1;
            int xpt[2];
            int ypt[2];
            int xanchor[2];
            int yanchor[2];
            for (size_t i = 0; i < n; i++) {
              // Type() is checked on the base pointer BEFORE the downcast: the
              // slot may hold any element kind (or nothing), and casting first
              // and asking afterwards reads the wrong object's vtable.
              VensimViewElement *pipe = elements[i];
              if (!pipe || pipe->Type() != VensimViewElement::ElementTypeCONNECTOR)
                continue;
              VensimConnectorElement *cele = static_cast<VensimConnectorElement *>(pipe);
              if (cele->From() != local_uid - 1)
                continue;
              // check to see what to is
              VensimViewElement *endpoint = at(cele->To());
              bool isgood = false;
              if (endpoint) {
                if ((endpoint->Type() == VensimViewElement::ElementTypeVARIABLE)) {
                  Variable *var = endpoint->GetVariable();
                  if (var && var->VariableType() == XMILE_Type_STOCK)
                    isgood = true;
                } else if (endpoint->Type() == VensimViewElement::ElementTypeCOMMENT) {
                  isgood = true;
                }
              }
              if (!isgood)
                continue;
              xpt[count] = cele->X();
              xanchor[count] = endpoint->X();
              ypt[count] = cele->Y();
              yanchor[count] = endpoint->Y();
              if (endpoint->Type() == VensimViewElement::ElementTypeVARIABLE) {
                Variable *var = endpoint->GetVariable();
                if (toind == -1 && var && var->VariableType() == XMILE_Type_STOCK) {
                  // are we an inflow or an outflow
                  for (Variable *inflow : var->Inflows()) {
                    if (inflow == vele->GetVariable()) {
                      toind = count;
                      break;
                    }
                  }
                  if (toind == -1) {
                    for (Variable *outflow : var->Outflows()) {
                      if (outflow == vele->GetVariable()) {
                        toind = count ? 0 : 1;
                        break;
                      }
                    }
                  }
                }
              }
              count++;
              if (count == 2)
                break;
            }
            if (count < 2) {
              // Neither pipe endpoint resolved to a stock or cloud connector
              // record in the model -- synthesize a straight horizontal pipe
              // centered on the flow so the emitted <pts> is well-formed.
              xpt[0] = vele->X() - 150;
              xpt[1] = vele->X() + 25;
              ypt[0] = ypt[1] = vele->Y();
              toind = 1;
            } else {
              // When both endpoints are clouds (toind stays -1 because no
              // stock was matched), the flow has no canonical direction in the
              // engine's stock-flow graph. Default to toind=1 so the emitted
              // <pts> records the dst position last; both endpoints carry
              // their actual cloud coordinates from xanchor.
              if (toind < 0)
                toind = 1;
              // Each pipe endpoint sits at the center of the element it connects
              // to (the stock or the cloud), on BOTH axes. The connector's own
              // point (cele->X/Y) is a routing waypoint -- for a straight pipe it
              // is the midpoint between the valve and the endpoint, not the
              // endpoint itself -- so using it for either axis pulls the pipe end
              // halfway toward the valve and the diagram creeps on every round
              // trip. Anchoring both coordinates at the element center makes the
              // emitted <pts> depend only on the (stable) element positions, so
              // the geometry is a fixpoint. (The earlier code kept the connector
              // coordinate on one axis and only worked for near-horizontal pipes,
              // where the routing y happens to fall close to the endpoint y; a
              // vertical pipe was misdetected via the exact xpt[0]==xpt[1] test
              // and drifted.)
              xpt[0] = xanchor[0];
              xpt[1] = xanchor[1];
              ypt[0] = yanchor[0];
              ypt[1] = yanchor[1];
            }
            tinyxml2::XMLElement *xpts = doc->NewElement("pts");
            xvar->InsertEndChild(xpts);
            tinyxml2::XMLElement *xxpt = doc->NewElement("pt");
            xpts->InsertEndChild(xxpt);
            xxpt->SetAttribute("x", xpt[1 - toind]);
            xxpt->SetAttribute("y", ypt[1 - toind]);
            xxpt = doc->NewElement("pt");
            xpts->InsertEndChild(xxpt);
            xxpt->SetAttribute("x", xpt[toind]);
            xxpt->SetAttribute("y", ypt[toind]);
          }
        }
      } else if (ele->Type() == VensimViewElement::ElementTypeCONNECTOR) {
        VensimConnectorElement *cele = static_cast<VensimConnectorElement *>(ele);
        if (cele->From() > 0 && cele->To() > 0) {
          VensimViewElement *fromEle = at(cele->From());
          VensimViewElement *toEle = at(cele->To());
          if (fromEle && toEle) {
            // if from is a valve we switch it to the next element in the list which should be a var.
            // "should be" is the whole hazard: the slot one past an attached
            // valve is the flow-pipe convention (see src/Xmile/CLAUDE.md), not
            // something the file guarantees, so the re-pointed endpoint is
            // re-tested below rather than assumed. Re-pointing without
            // re-testing is what turned a missing flow record into a NULL
            // dereference here.
            if (fromEle->Type() == VensimViewElement::ElementTypeVALVE &&
                static_cast<VensimValveElement *>(fromEle)->Attached()) {
              fromEle = at(cele->From() + 1);
            }
            if (toEle->Type() == VensimViewElement::ElementTypeVALVE &&
                static_cast<VensimValveElement *>(toEle)->Attached())
              toEle = at(cele->To() + 1);
            if (fromEle && fromEle->Type() == VensimViewElement::ElementTypeVARIABLE && toEle &&
                toEle->Type() == VensimViewElement::ElementTypeVARIABLE) {
              VensimVariableElement *from = static_cast<VensimVariableElement *>(fromEle);
              VensimVariableElement *to = static_cast<VensimVariableElement *>(toEle);
              // Time (and the other control variables) never appear in the
              // XMILE view, so a connector touching one has no endpoint to
              // reference; ghosts and cross-level stubs likewise cannot be a
              // connector target.
              if (from->GetVariable() && to->GetVariable() && to->GetVariable()->VariableType() != XMILE_Type_STOCK &&
                  !from->GetVariable()->Unwanted() && !to->GetVariable()->Unwanted() && !to->Ghost(NULL, false) &&
                  !to->CrossLevel()) {
                // valid xmile connector
                tinyxml2::XMLElement *xconnector = doc->NewElement("connector");
                element->InsertEndChild(xconnector);
                xconnector->SetAttribute("uid", uid);
                // try to figure out the angle based on the 3 points -
#ifndef NDEBUG
                double thetax = 999;
                if (to->GetVariable()->GetName() == "US crude death rate")
                  thetax = AngleFromPoints(from->X(), from->Y(), cele->X(), cele->Y(), to->X(), to->Y());
#endif
                xconnector->SetAttribute("angle",
                                         AngleFromPoints(from->X(), from->Y(), cele->X(), cele->Y(), to->X(), to->Y()));
                if (cele->Polarity()) {
                  char cbuf[2];
                  cbuf[0] = cele->Polarity();
                  cbuf[1] = 0;
                  xconnector->SetAttribute("polarity", cbuf);
                }
                tinyxml2::XMLElement *xfrom = doc->NewElement("from");
                xconnector->InsertEndChild(xfrom);
                if (from->Ghost(adds, false)) {
                  tinyxml2::XMLElement *xalias = doc->NewElement("alias");
                  xfrom->InsertEndChild(xalias);
                  xalias->SetAttribute("uid", view->UIDOffset() + cele->From());
                } else if (from->GetVariable()) {
                  xfrom->SetText(QuotedSpaceToUnderBar(from->GetVariable()->GetAlternateName()).c_str());
                }
                tinyxml2::XMLElement *xto = doc->NewElement("to");
                xconnector->InsertEndChild(xto);
                xto->SetText(QuotedSpaceToUnderBar(to->GetVariable()->GetAlternateName()).c_str());
              }
            }
          }
        }
      }
    }
  }
}
