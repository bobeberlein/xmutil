#pragma once
#ifndef __XMILE_H
#define __XMILE_H

class Model;

#include <tinyxml2.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "../Symbol/Symbol.h"
class VensimView;
class SymbolNameSpace;
class Variable;

class XMILEGenerator {
public:
  XMILEGenerator(Model *model, double xtrario, double yratio, bool from_dynamo);

  std::string Print(bool is_compact, std::vector<std::string> &errs, bool as_sectors);

protected:
  void generateHeader(tinyxml2::XMLElement *element, std::vector<std::string> &errs);
  void generateSimSpecs(tinyxml2::XMLElement *element, std::vector<std::string> &errs);
  void generateModelUnits(tinyxml2::XMLElement *element, std::vector<std::string> &errs);
  void generateDimensions(tinyxml2::XMLElement *element, std::vector<std::string> &errs);
  void generateModelAsSectors(tinyxml2::XMLElement *element, std::vector<std::string> &errs, SymbolNameSpace *ns,
                              bool wantDiagram);
  void generateEquation(Variable *var, tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *variables);
  // Ordered by name (SymbolNameLess), not pointer value: iteration order of
  // these sets determines the order variables are emitted in the document.
  void generateEquations(std::set<Variable *, SymbolNameLess> &included, tinyxml2::XMLDocument *doc,
                         tinyxml2::XMLElement *variables);
  void generateModelAsModules(tinyxml2::XMLElement *element, std::vector<std::string> &errs, SymbolNameSpace *ns);
  bool generateModelAsGroups(tinyxml2::XMLElement *elemeGnt, std::vector<std::string> &errs, SymbolNameSpace *ns);
  void generateSectorViews(tinyxml2::XMLElement *views, tinyxml2::XMLElement *vars, std::vector<std::string> &errs,
                           bool mainmodel);
  void generateView(VensimView *view, tinyxml2::XMLElement *element, std::vector<std::string> &errs,
                    std::set<Variable *, SymbolNameLess> *needed);

private:
  Model *_model;
  // Per variable, as its equations are written: the inputs a translation read
  // as literals, and the variables the written equations refer to. An input in
  // the first and not the second is one the variable no longer depends on in
  // XMILE, so its arrow is not drawn (InputDropped, generateView).
  std::map<Variable *, std::set<Variable *>> _droppedInputs;
  std::map<Variable *, std::set<Variable *>> _referencedInputs;
  bool InputDropped(Variable *to, Variable *from) const {
    auto d = _droppedInputs.find(to);
    if (d == _droppedInputs.end() || !d->second.count(from))
      return false;
    auto r = _referencedInputs.find(to);
    return r == _referencedInputs.end() || !r->second.count(from);
  }
  double _xratio;
  double _yratio;
  bool _from_dynamo;
};

#endif
