#include "XMILEGenerator.h"

#include "../Model.h"
#include "../XMUtil.h"
#include "../Vensim/VensimView.h"
#include "../Symbol/ExpressionList.h"

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

	tinyxml2::XMLElement* prefs = doc.NewElement("isee:prefs");
	root->InsertEndChild(prefs);
	prefs->SetAttribute("show_module_prefix", "true");
	prefs->SetAttribute("layer", "model");


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
	this->generateModel(model, errs, NULL);
	root->InsertEndChild(model);

	// macros are presented as separate models
	BOOST_FOREACH(MacroFunction* mf, _model->MacroFunctions())
	{
		tinyxml2::XMLElement* macro = doc.NewElement("macro");
		macro->SetAttribute("name", mf->GetName().c_str());

		// in vensim the equation is always just the name fo the macro
		tinyxml2::XMLElement* xeqn = doc.NewElement("eqn");
		macro->InsertEndChild(xeqn);
		xeqn->SetText(mf->GetName().c_str());
		// the parms are all of the entries in the macro description
		ExpressionList * args = mf->Args();
		int n = args->Length();
		for(int i = 0;i<n;i++)
		{ 
			tinyxml2::XMLElement* xparm = doc.NewElement("parm");
			macro->InsertEndChild(xparm);
			Expression* pexp = args->GetExp(i);
			ContextInfo info;
			pexp->OutputComputable(&info);
			xparm->SetText(info.str().c_str());
		}

		this->generateModel(macro, errs, mf->NameSpace());
		root->InsertEndChild(macro);
	}

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

	element->SetAttribute("isee:simulation_delay", "0");
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

	_model->SetUnwanted("INITIAL TIME", "STARTTIME");
	_model->SetUnwanted("FINAL TIME", "STOPTIME");
	_model->SetUnwanted("TIME STEP", "DT");
	_model->SetUnwanted("SAVEPER", "DT");
}

void XMILEGenerator::generateDimensions(tinyxml2::XMLElement* element, std::vector<std::string>& errs)
{
	tinyxml2::XMLDocument* doc = element->GetDocument();
	std::vector<Variable*> vars = _model->GetVariables(); // all symbols that are variables
	BOOST_FOREACH(Variable* var, vars)
	{
		if (var->VariableType() == XMILE_Type_ARRAY)
		{
			// simple minded - defining equation - 
			Equation* eq = var->GetEquation(0);
			if (eq)
			{
				Expression* exp = eq->GetExpression();
				if (exp && exp->GetType() == EXPTYPE_Symlist)
				{
					SymbolList* symlist = static_cast<ExpressionSymbolList*>(exp)->SymList();
					std::vector<Symbol*> expanded;
					int n = symlist->Length();
					for (int i = 0; i < n; i++)
					{
						const SymbolList::SymbolListEntry& elm = (*symlist)[i];
						if (elm.eType == SymbolList::EntryType_SYMBOL)
						{
							Equation::GetSubscriptElements(expanded, elm.u.pSymbol);
						}
					}
					// we define subranges as if they were arrays themselves - because of the unique namespace in XMILE this
					// is proper - and it will make any model with partial definitions more or less okay -
					if (!expanded.empty()/* && expanded[0]->Owner() == var*/)
					{
						tinyxml2::XMLElement* xsub = doc->NewElement("dim");
						xsub->SetAttribute("name", var->GetName().c_str());
						BOOST_FOREACH(Symbol* s, expanded)
						{
							tinyxml2::XMLElement* xelm = doc->NewElement("elem");
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
void XMILEGenerator::generateModel(tinyxml2::XMLElement* element, std::vector<std::string>& errs, SymbolNameSpace* ns)
{
	tinyxml2::XMLDocument* doc = element->GetDocument();
	tinyxml2::XMLElement* variables = doc->NewElement("variables");
	element->InsertEndChild(variables);

	std::vector<Variable*> vars = _model->GetVariables(ns); // all symbols that are variables
	BOOST_FOREACH(Variable* var, vars)
	{
		if (var->Unwanted())
			continue;
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
		int eq_count = eqns.size();


		// dimensions
		std::vector<Variable*> elmlist;
		int dim_count = var->SubscriptCount(elmlist);

		std::string comment = var->Comment();
		if (!comment.empty())
		{
			tinyxml2::XMLElement* xcomment = doc->NewElement("doc");
			xvar->InsertEndChild(xcomment);
			xcomment->SetText(comment.c_str());
		}
		if (type == XMILE_Type_STOCK)
		{
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


		tinyxml2::XMLElement* xelement = xvar; // usually these are the same - but for non a2a we have element entries
		int eq_ind = 0;
		int eq_pos = 0;
		std::vector<Symbol*> subs; // [ship,location]
		std::vector<std::vector<Symbol*> > elms; // [s1,l1]
		std::vector<std::set<Symbol *> > entries;
		std::vector<Symbol*> dims;
		while (eq_ind < eq_count)
		{
			Equation* eqn = eqns[eq_ind];
			if (eq_count > 1)
			{
				if (entries.empty())
					entries.resize(dim_count);
				// we will blow up everything to single elements
				if (elms.empty())
				{
					eq_pos = 0;
					elms.clear();
					eqn->SubscriptExpand(elms, subs);
					assert(!elms.empty());
					BOOST_FOREACH(std::vector<Symbol*> elm, elms)
					{
						for (int i = 0; i < dim_count; i++)
						{
							entries[i].insert(elm[i]);
						}
					}
				}
				dims = elms[eq_pos];
				std::string s;
				int dim_count = dims.size();
				for (int j = 0; j < dim_count; j++)
				{
					if (j)
						s += ", ";
					s += dims[j]->GetName();
				}
				xelement = doc->NewElement("element");
				xelement->SetAttribute("subscript", s.c_str());
				xvar->InsertEndChild(xelement);
			}
			tinyxml2::XMLElement* xeqn = doc->NewElement("eqn");
			xelement->InsertEndChild(xeqn);
			xeqn->SetText(eqn->RHSFormattedXMILE(subs,dims, false).c_str());


			// it it is active init we need to store that separately
			if (eqn->IsActiveInit())
			{
				tinyxml2::XMLElement* xieqn = doc->NewElement("init_eqn");
				xelement->InsertEndChild(xieqn);
				xieqn->SetText(eqn->RHSFormattedXMILE(subs, dims, true).c_str());
			}

			// if it has a lookup we need to store that separately
			ExpressionTable* et = eqn->GetTable();
			if (et)
			{
				assert(type == XMILE_Type_AUX || type == XMILE_Type_FLOW);
				std::vector<double>* xvals = et->GetXVals();
				std::vector<double>* yvals = et->GetYVals();
				tinyxml2::XMLElement* gf = doc->NewElement("gf");
				if (et->Extrapolate())
					gf->SetAttribute("type", "extrapolate");
				xelement->InsertEndChild(gf);
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
			if (eq_count > 1)
			{
				eq_pos++;
				if (eq_pos >= elms.size())
				{
					elms.clear();
					eq_ind++;
				}
			}
			else
				eq_ind++;
		}


		// use entries to try to figure out the appropriate dimensions
		if (dim_count)
		{
			// Vensim allowed partial definition sets - XMILE uses subranges as separate dimensions so we 
			// try to find the most compact set of dimensions possible that inlcude all the equations include
			std::vector<Variable*> dimensions;

			tinyxml2::XMLElement* xdims = doc->NewElement("dimensions");
			for (int i = 0; i < dim_count; i++)
			{
				tinyxml2::XMLElement* xdim = doc->NewElement("dim");
				if (entries.empty())
				{
					// we might get a subrange in elmlist so need to get parent - but only if there is more than 1 equation
					if (eq_count > 1 || elmlist[i]->GetAllEquations().empty())
						xdim->SetAttribute("name", elmlist[i]->Owner()->GetName().c_str());
					else
						xdim->SetAttribute("name", elmlist[i]->GetName().c_str());
				}
				else
				{
					std::set<Symbol*>& entry = entries[i];
					Symbol* parent = (*entry.begin())->Owner();
					Symbol* best = parent;
					if (parent->Subranges() != NULL && static_cast<Variable*>(parent)->Nelm() > entry.size())
					{
						BOOST_FOREACH(Symbol* subrange, *parent->Subranges())
						{
							if (static_cast<Variable*>(subrange)->Nelm() >= entry.size() &&
								static_cast<Variable*>(subrange)->Nelm() < static_cast<Variable*>(best)->Nelm())
							{
								// does it have them all
								bool complete = true;
								std::vector<Symbol*> telms;
								Equation::GetSubscriptElements(telms, subrange);
								BOOST_FOREACH(Symbol* elm, entries[i])
								{
									if (std::find(telms.begin(), telms.end(), elm) == telms.end())
									{
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

		UnitExpression* un = var->Units();
		if (un)
		{
			tinyxml2::XMLElement* units = doc->NewElement("units");
			xvar->InsertEndChild(units);
			units->SetText(un->GetEquationString().c_str());
		}
	}
	tinyxml2::XMLElement* views = doc->NewElement("views");
	this->generateViews(views, variables, errs);
	element->InsertEndChild(views);

}

void XMILEGenerator::generateViews(tinyxml2::XMLElement* element, tinyxml2::XMLElement* xvars, std::vector<std::string>& errs)
{
	tinyxml2::XMLDocument* doc = element->GetDocument();

	std::vector<View*>& views = _model->Views();
	int x, y;
	// start at a reasonable distance from 0 - the x,y values are generally around hte center
	// of the var
	x = 100;
	y = 100;
	// all the views against a single xmile view - or break up into modules - need vector of models as input to do that
	tinyxml2::XMLElement* xview = doc->NewElement("view");
	element->InsertEndChild(xview);
	int uid_off = 0;
	BOOST_FOREACH(View* gview, views)
	{
		VensimView* view = static_cast<VensimView*>(gview);
		// first update geometry - we put views one after another along the y axix - could lay out in pages or something
		uid_off = view->SetViewStart(x, y+20, uid_off);
		int width = view->GetViewMaxX(100);
		int height = view->GetViewMaxY(y + 80) - y;
		// add a surrounding sector to contain this view - call it the view name
		// 				<group locked="false" x="184" y="154" width="300" height="184" name="Sector 1"/>

		if (views.size() > 1)
		{
			std::string name = view->Title();
			while (_model->GetNameSpace()->Find(name))
			{
				name += "1"; // not very original
			}
			tinyxml2::XMLElement* xsectorvar = doc->NewElement("group");
			xvars->InsertEndChild(xsectorvar);
			xsectorvar->SetAttribute("name", name.c_str());
			tinyxml2::XMLElement* xsector = doc->NewElement("group");
			xview->InsertEndChild(xsector);
			xsector->SetAttribute("name", name.c_str());
			xsector->SetAttribute("x", StringFromDouble(x - 40).c_str());
			xsector->SetAttribute("y", StringFromDouble(y).c_str());
			xsector->SetAttribute("width", StringFromDouble(width + 60).c_str());
			xsector->SetAttribute("height", StringFromDouble(height + 40).c_str());
		}

		y += height + 80;

		this->generateView(view, xview, errs);

	}
}

void XMILEGenerator::generateView(VensimView* view, tinyxml2::XMLElement* element, std::vector<std::string>& errs)
{
	tinyxml2::XMLDocument* doc = element->GetDocument();
	int uid = view->UIDOffset();
	int local_uid = 0;
	VensimViewElements& elements = view->Elements();
	BOOST_FOREACH(VensimViewElement* ele, elements)
	{
		if (ele)
		{
			if (ele->Type() == VensimViewElement::ElementTypeVARIABLE)
			{
				VensimVariableElement* vele = static_cast<VensimVariableElement*>(ele);
				Variable* var = vele->GetVariable();
				// skip time altogether - this never shows up under xmil
				if (!var || StringMatch(vele->GetVariable()->GetName(),"Time") || var->Unwanted())
					; // do nothing
				else if (vele->Ghost())
				{
					assert(vele->GetVariable()->VariableType() != XMILE_Type_ARRAY);
					tinyxml2::XMLElement* xghost = doc->NewElement("alias");
					element->InsertEndChild(xghost);
					xghost->SetAttribute("x", vele->X());
					xghost->SetAttribute("y", vele->Y());
					xghost->SetAttribute("uid", uid);
					tinyxml2::XMLElement* xof = doc->NewElement("of");
					xghost->InsertEndChild(xof);
					xof->SetText(SpaceToUnderBar(vele->GetVariable()->GetAlternateName()).c_str());
				}
				else
				{
					XMILE_Type type = vele->GetVariable()->VariableType();
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
					}
					if (tag.empty())
						continue;
					tinyxml2::XMLElement* xvar = doc->NewElement(tag.c_str());

					element->InsertEndChild(xvar);

					std::string name = vele->GetVariable()->GetAlternateName();
					xvar->SetAttribute("name", SpaceToUnderBar(vele->GetVariable()->GetAlternateName()).c_str());
					if (type == XMILE_Type_FLOW && vele->Attached()  && elements[local_uid - 1] &&
						elements[local_uid-1]->Type() == VensimViewElement::ElementTypeVALVE)
					{
						xvar->SetAttribute("x", elements[local_uid - 1]->X());
						xvar->SetAttribute("y", elements[local_uid-1]->Y());

					}
					else
					{
						xvar->SetAttribute("x", vele->X());
						xvar->SetAttribute("y", vele->Y());
					}
					if (type == XMILE_Type_FLOW)
					{
						// need points - these are the location of the from and to - no matter what they are
						// but we need to search through the list of eleemnts to find the from and to - flow
						// arrows are always out of the attached value which is just before us in the list
						// flow direction we need to take from the model proper - arbitrary if flow is not connected
						size_t n = elements.size();
						int count = 0;
						int toind = -1;
						int xpt[2];
						int ypt[2];
						for (size_t i = 0; i < n; i++)
						{
							VensimConnectorElement* cele = static_cast<VensimConnectorElement*>(elements[i]);
							if (cele && cele->Type() == VensimViewElement::ElementTypeCONNECTOR)
							{
								if (cele->From() == uid - 1)
								{
									// check to see what to is
									VensimVariableElement* stock = static_cast<VensimVariableElement*>(elements[cele->To()]);
									if (stock)
									{
										xpt[count] = stock->X();
										ypt[count] = stock->Y();
										if (stock->Type() == VensimViewElement::ElementTypeVARIABLE)
										{
											Variable* var = stock->GetVariable();
											if (toind == -1 && var && var->VariableType() == XMILE_Type_STOCK)
											{
												// are we an inflow or an outflow
												BOOST_FOREACH(Variable* inflow, var->Inflows())
												{
													if (inflow == vele->GetVariable())
													{
														toind = count;
														break;
													}
												}
												if (toind == -1)
												{
													BOOST_FOREACH(Variable* outflow, var->Outflows())
													{
														if (outflow == vele->GetVariable())
														{
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
								}
							}
						}
						if (count < 2 || toind < 0)
						{
							xpt[0] = vele->X() - 25;
							xpt[1] = vele->X() + 25;
							ypt[0] = ypt[1] = vele->Y();
							toind = 1;
						}
						tinyxml2::XMLElement* xpts = doc->NewElement("pts");
						xvar->InsertEndChild(xpts);
						tinyxml2::XMLElement* xxpt = doc->NewElement("pt");
						xpts->InsertEndChild(xxpt);
						xxpt->SetAttribute("x", xpt[1 - toind]);
						xxpt->SetAttribute("y", ypt[1 - toind]);
						xxpt = doc->NewElement("pt");
						xpts->InsertEndChild(xxpt);
						xxpt->SetAttribute("x", xpt[toind]);
						xxpt->SetAttribute("y", ypt[toind]);
					}
				}
			}
			else if (ele->Type() == VensimViewElement::ElementTypeCONNECTOR)
			{
				VensimConnectorElement* cele = static_cast<VensimConnectorElement*>(ele);
				if (cele->From() > 0 && cele->To() > 0)
				{
					VensimVariableElement* from = static_cast<VensimVariableElement*>(elements[cele->From()]);
					// if from is a valve we switch it to the next element in the list which should be a var
					if (from->Type() == VensimViewElement::ElementTypeVALVE && static_cast<VensimValveElement*>(elements[cele->From()])->Attached())
						from = static_cast<VensimVariableElement*>(elements[cele->From() + 1]);
					VensimVariableElement* to = static_cast<VensimVariableElement*>(elements[cele->To()]);
					if (to->Type() == VensimViewElement::ElementTypeVALVE && static_cast<VensimValveElement*>(elements[cele->To()])->Attached())
						to = static_cast<VensimVariableElement*>(elements[cele->To() + 1]);
					if (from && to && from->Type() == VensimViewElement::ElementTypeVARIABLE && to && to->Type() == VensimViewElement::ElementTypeVARIABLE &&
						to->GetVariable() && 
						to->GetVariable()->VariableType() != XMILE_Type_STOCK)
					{
						tinyxml2::XMLElement* xconnector = doc->NewElement("connector");
						element->InsertEndChild(xconnector);
						xconnector->SetAttribute("uid", uid);
						// try to figure out the angle based on the 3 points - 
						xconnector->SetAttribute("angle", AngleFromPoints(from->X(), from->Y(), cele->X(), cele->Y(), to->X(), to->Y()));
						tinyxml2::XMLElement* xfrom = doc->NewElement("from");
						xconnector->InsertEndChild(xfrom);
						if (from->Ghost())
						{
							tinyxml2::XMLElement* xalias = doc->NewElement("alias");
							xfrom->InsertEndChild(xalias);
							xalias->SetAttribute("uid", view->UIDOffset() + cele->From());
						}
						else
						{
							xfrom->SetText(SpaceToUnderBar(from->GetVariable()->GetAlternateName()).c_str());
						}
						tinyxml2::XMLElement* xto = doc->NewElement("to");
						xconnector->InsertEndChild(xto);
						xto->SetText(SpaceToUnderBar(to->GetVariable()->GetAlternateName()).c_str());
					}
				}
			}
		}
		uid++;
		local_uid++;
	}
}
