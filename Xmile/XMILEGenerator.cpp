#include "XMILEGenerator.h"

#include "tinyxml2.cpp" // put elsewhere if we use tinyxml in more than one place or better still switch to RapidXML


#include "../Model.h"
#include "../XMUtil.h"

XMILEGenerator::XMILEGenerator(Model* model)
{
	_model = model;
}

bool XMILEGenerator::Generate(const std::string& path, std::vector<std::string>& errs)
{
	tinyxml2::XMLDocument doc;
	
	tinyxml2::XMLElement* root = doc.NewElement("xmile");
	root->SetName("xmile");
	root->SetAttribute("xmlns", "http://docs.oasis-open.org/xmile/ns/XMILE/v1.0");
	root->SetAttribute("version", "1.0");
	doc.InsertFirstChild(root);

	tinyxml2::XMLElement* header = doc.NewElement("header");
	this->generateHeader(header, errs);
	root->InsertEndChild(header);

	tinyxml2::XMLElement* specs = doc.NewElement("sim_specs");
	this->generateSimSpecs(specs, errs);
	root->InsertEndChild(specs);

	tinyxml2::XMLElement* dimensions = doc.NewElement("dimensions");
	this->generateDimensions(dimensions, errs);
	root->InsertEndChild(dimensions);

	tinyxml2::XMLElement* model = doc.NewElement("model");
	this->generateModel(model, errs);
	root->InsertEndChild(model);

	tinyxml2::XMLError err = doc.SaveFile(path.c_str());
	if (err != tinyxml2::XML_SUCCESS)
	{
		if (doc.GetErrorStr1())
			errs.push_back("TinyXML2 Error #1 " + std::string(doc.GetErrorStr1()));
		else
			errs.push_back("File error opening document");
		if (doc.GetErrorStr2())
			errs.push_back("TinyXML2 Error #2 " + std::string(doc.GetErrorStr2()));

		return false;
	}
	
	return true;
}

void XMILEGenerator::generateHeader(tinyxml2::XMLElement* element, std::vector<std::string>& errs)
{
	tinyxml2::XMLDocument* doc = element->GetDocument();
	
	tinyxml2::XMLElement* options = doc->NewElement("options");
	options->SetAttribute("namespace", "std");
	element->InsertEndChild(options);

	tinyxml2::XMLElement* vendor = doc->NewElement("vendor");
	vendor->SetText("Ventana Systems, xmutil");
	element->InsertEndChild(vendor);

	tinyxml2::XMLElement* product = doc->NewElement("product");
	product->SetAttribute("lang", "en");
	product->SetText("Vensim, xmutil");
	element->InsertEndChild(product);
}

void XMILEGenerator::generateSimSpecs(tinyxml2::XMLElement* element, std::vector<std::string>& errs)
{
	/*
	<sim_specs method = "Euler" time_units = "Months">
	<start>0< / start>
	<stop>100< / stop>
	<dt>0.125< / dt>
	< / sim_specs>
	*/

	tinyxml2::XMLDocument* doc = element->GetDocument();

	SymbolNameSpace* nameSpace = _model->GetNameSpace();
	Variable* time = static_cast<Variable *>(nameSpace->Find("Time"));

	element->SetAttribute("method", "Euler"); //TODO -- FIXME!!!!
	element->SetAttribute("time_units", "Months"); //TODO -- FIXME!!! how do I access the units of the Time Variable??

	tinyxml2::XMLElement* startEle = doc->NewElement("start");
	double val = _model->GetConstanValue("INITIAL TIME", 0); // default to 0 if INITIAL TIME is missing or an equation
	startEle->SetText(StringFromDouble(val).c_str());
	element->InsertEndChild(startEle);


	tinyxml2::XMLElement* stopEle = doc->NewElement("stop");
	val = _model->GetConstanValue("FINAL TIME", 100); 
	stopEle->SetText(StringFromDouble(val).c_str());
	element->InsertEndChild(stopEle);

	tinyxml2::XMLElement* dtEle = doc->NewElement("dt");
	val = _model->GetConstanValue("TIME STEP", 1);
	dtEle->SetText(StringFromDouble(val).c_str());
	element->InsertEndChild(dtEle);
}

void XMILEGenerator::generateDimensions(tinyxml2::XMLElement* element, std::vector<std::string>& errs)
{
}

// first pass if flat - we probably want to do this differently when we break up into modules
void XMILEGenerator::generateModel(tinyxml2::XMLElement* element, std::vector<std::string>& errs)
{
	tinyxml2::XMLDocument* doc = element->GetDocument();
	tinyxml2::XMLElement* variables = doc->NewElement("variables");
	element->InsertEndChild(variables);

	std::vector<Variable*> vars = _model->GetVariables(); // all symbols that are variables
	BOOST_FOREACH(Variable* var, vars)
	{
		XMILE_Type type = var->VariableType();
		std::string tag;
		switch (type)
		{
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
			continue;
		case XMILE_Type_ARRAY_ELM:
			continue;
		default:
			continue;
			break;
		}
		tinyxml2::XMLElement* xvar = doc->NewElement(tag.c_str());

		variables->InsertEndChild(xvar);
		xvar->SetAttribute("name", var->GetAlternateName().c_str());
		std::vector<Equation*> eqns = var->GetAllEquations();
		if (eqns.size() == 1)
		{
			tinyxml2::XMLElement* eqn = doc->NewElement("eqn");
			xvar->InsertEndChild(eqn);
			eqn->SetText(eqns[0]->RHSFormattedXMILE().c_str());
			if (type == XMILE_Type_STOCK)
			{
				assert(!eqns[0]->IsTable());
				BOOST_FOREACH(Variable* in, var->Inflows())
				{
					tinyxml2::XMLElement* inflow = doc->NewElement("inflow");
					xvar->InsertEndChild(inflow);
					inflow->SetText(SpaceToUnderBar(in->GetAlternateName()).c_str());
				}
				BOOST_FOREACH(Variable* out, var->Outflows())
				{
					tinyxml2::XMLElement* outflow = doc->NewElement("outflow");
					xvar->InsertEndChild(outflow);
					outflow->SetText(SpaceToUnderBar(out->GetAlternateName()).c_str());
				}


			}
			// if it has a lookup we need to store that separately
			if (eqns[0]->IsTable())
			{
				assert(type == XMILE_Type_AUX);
				ExpressionTable* et = static_cast<ExpressionTable*>(eqns[0]->GetExpression());
				std::vector<double>* xvals = et->GetXVals();
				std::vector<double>* yvals = et->GetYVals();
				tinyxml2::XMLElement* gf = doc->NewElement("gf");
				xvar->InsertEndChild(gf);
				tinyxml2::XMLElement* yscale = doc->NewElement("yscale");
				gf->InsertEndChild(yscale);
				tinyxml2::XMLElement* xpts = doc->NewElement("xpts");
				gf->InsertEndChild(xpts);
				tinyxml2::XMLElement* ypts = doc->NewElement("ypts");
				gf->InsertEndChild(ypts);

				std::string xstr;
				for (size_t i = 0; i < xvals->size(); i++)
				{
					if (i)
						xstr += ",";
					xstr += StringFromDouble((*xvals)[i]);
				}
				xpts->SetText(xstr.c_str());

				std::string ystr;
				double ymin, ymax;
				for (size_t i = 0; i < yvals->size(); i++)
				{
					if (i)
					{
						ystr += ",";
						if ((*yvals)[i] < ymin)
							ymin = (*yvals)[i];
						else if ((*yvals)[i] > ymax)
							ymax = (*yvals)[i];
					}
					else
						ymin = ymax = (*yvals)[i];
					ystr += StringFromDouble((*yvals)[i]);
				}
				ypts->SetText(ystr.c_str());

				if (ymin == ymax)
					ymax = ymin + 1;
				yscale->SetAttribute("min", StringFromDouble(ymin).c_str());
				yscale->SetAttribute("max", StringFromDouble(ymax).c_str());
			}
		}
	}
}

