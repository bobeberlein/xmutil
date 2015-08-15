#include "XMILEGenerator.h"

#include "../Model.h"

#include "tinyxml2.h"

using namespace tinyxml2;

XMILEGenerator::XMILEGenerator(Model* model)
{
	_model = model;
}

bool XMILEGenerator::Generate(const std::string& path, std::vector<std::string>& errs)
{
	XMLDocument doc;
	
	XMLElement* root = doc.NewElement("xmile");
	root->SetName("xmile");
	root->SetAttribute("xmlns", "http://docs.oasis-open.org/xmile/ns/XMILE/v1.0");
	root->SetAttribute("version", "1.0");
	doc.InsertFirstChild(root);

	XMLElement* header = doc.NewElement("header");
	this->generateHeader(header, errs);
	root->InsertEndChild(header);

	XMLElement* specs = doc.NewElement("sim_specs");
	this->generateSimSpecs(specs, errs);
	root->InsertEndChild(specs);

	XMLError err = doc.SaveFile(path.c_str());
	if (err != XML_SUCCESS)
	{
		errs.push_back("TinyXML2 Error #1 " + std::string(doc.GetErrorStr1()));
		errs.push_back("TinyXML2 Error #2 " + std::string(doc.GetErrorStr2()));
		return false;
	}
	
	return true;
}

void XMILEGenerator::generateHeader(XMLElement* element, std::vector<std::string>& errs)
{
	XMLDocument* doc = element->GetDocument();
	
	XMLElement* options = doc->NewElement("options");
	options->SetAttribute("namespace", "std");
	element->InsertEndChild(options);

	XMLElement* vendor = doc->NewElement("vendor");
	vendor->SetText("Ventana Systems, xmutil");
	element->InsertEndChild(vendor);

	XMLElement* product = doc->NewElement("product");
	product->SetAttribute("lang", "en");
	product->SetText("Vensim, xmutil");
	element->InsertEndChild(product);
}

void XMILEGenerator::generateSimSpecs(XMLElement* element, std::vector<std::string>& errs)
{
	/*
	<sim_specs method = "Euler" time_units = "Months">
	<start>0< / start>
	<stop>100< / stop>
	<dt>0.125< / dt>
	< / sim_specs>
	*/

	XMLDocument* doc = element->GetDocument();

	SymbolNameSpace* nameSpace = _model->GetNameSpace();
	Variable* time = static_cast<Variable *>(nameSpace->Find("Time"));
	Variable* start = static_cast<Variable *>(nameSpace->Find("INITIAL TIME"));
	Variable* end = static_cast<Variable *>(nameSpace->Find("FINAL TIME"));
	Variable* step = static_cast<Variable *>(nameSpace->Find("TIME STEP"));

	element->SetAttribute("method", "Euler"); //TODO -- FIXME!!!!
	element->SetAttribute("time_units", "Months"); //TODO -- FIXME!!! how do I access the units of the Time Variable??

	XMLElement* startEle = doc->NewElement("start");
	startEle->SetText("2000"); //TODO -- FIXME -- how do I get the equation for the INITIAL TIME variable?
	element->InsertEndChild(startEle);

	XMLElement* stopEle = doc->NewElement("stop");
	stopEle->SetText("2010"); //TODO -- FIXME -- how do I get the equation for the FINAL TIME variable?
	element->InsertEndChild(stopEle);

	XMLElement* dtEle = doc->NewElement("dt");
	dtEle->SetText("0.25"); //TODO -- FIXME -- how do I get the equation for the TIME STEP variable?
	element->InsertEndChild(dtEle);
}