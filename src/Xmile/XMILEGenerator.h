#pragma once
#ifndef __XMILE_H
#define __XMILE_H

class Model;

#include <tinyxml2.h>
#include <string>
#include <vector>
class VensimView;

class XMILEGenerator
{
public:
	XMILEGenerator(Model* model);

	bool Generate(const std::string& path, std::vector<std::string>& errs);

protected:
	void generateHeader(tinyxml2::XMLElement* element, std::vector<std::string>& errs);
	void generateSimSpecs(tinyxml2::XMLElement* element, std::vector<std::string>& errs);
	void generateDimensions(tinyxml2::XMLElement* element, std::vector<std::string>& errs);
	void generateModel(tinyxml2::XMLElement* element, std::vector<std::string>& errs);
	void generateViews(tinyxml2::XMLElement* views, tinyxml2::XMLElement* vars, std::vector<std::string>& errs);
	void generateView(VensimView* view, tinyxml2::XMLElement* element, std::vector<std::string>& errs);

private:
	Model* _model;
};

#endif
