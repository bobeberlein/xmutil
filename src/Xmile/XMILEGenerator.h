#pragma once
#ifndef __XMILE_H
#define __XMILE_H

class Model;

#include <tinyxml2.h>
#include <string>
#include <vector>
#include <set>
class VensimView;
class SymbolNameSpace;
class Variable;

class XMILEGenerator
{
public:
	XMILEGenerator(Model* model, double xtrario, double yratio, bool from_dynamo);

	std::string Print(bool is_compact, std::vector<std::string>& errs, bool as_sectors);

protected:
	void generateHeader(tinyxml2::XMLElement* element, std::vector<std::string>& errs);
	void generateSimSpecs(tinyxml2::XMLElement* element, std::vector<std::string>& errs);
	void generateModelUnits(tinyxml2::XMLElement* element, std::vector<std::string>& errs);
	void generateDimensions(tinyxml2::XMLElement* element, std::vector<std::string>& errs);
	void generateModelAsSectors(tinyxml2::XMLElement* element, std::vector<std::string>& errs, SymbolNameSpace* ns, bool wantDiagram);
	void generateEquation(Variable* var, tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* variables);
	void generateEquations(std::set<Variable*>& included, tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* variables);
	void generateModelAsModules(tinyxml2::XMLElement* element, std::vector<std::string>& errs, SymbolNameSpace* ns);
	bool generateModelAsGroups(tinyxml2::XMLElement* elemeGnt, std::vector<std::string>& errs, SymbolNameSpace* ns);
	void generateSectorViews(tinyxml2::XMLElement* views, tinyxml2::XMLElement* vars, std::vector<std::string>& errs, bool mainmodel);
	void generateView(VensimView* view, tinyxml2::XMLElement* element, std::vector<std::string>& errs, std::set<Variable*>* needed);

private:
	Model* _model;
	double _xratio;
	double _yratio;
	bool _from_dynamo;
};

#endif
