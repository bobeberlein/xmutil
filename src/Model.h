#ifndef _XMUTIL_MODEL_H
#define _XMUTIL_MODEL_H
#include <string>
#include <vector>

#include "Mdl/MDLFormat.h"
#include "Symbol/Expression.h"
#include "Symbol/Variable.h"

// One model-level unit declaration, held in the shape XMILE uses: a canonical
// name, an optional derived-unit equation, and any number of alternate
// spellings. The equation is a FORMULA for the unit ("kg*m/s^2" for a Newton,
// "1" for a dimensionless quantity), a different claim from "this is another
// name for the same unit", so it cannot be folded into the alias list without
// losing meaning.
//
// Vensim's settings-section "22:" line has no equation concept -- it is a flat
// comma-separated list of interchangeable unit names whose first field is
// canonical -- so the mapping between the two is defined here, in one place, by
// ParseMdlPayload / MdlPayload rather than re-derived at each call site.
struct UnitEquiv {
  std::string name;
  std::string eqn;
  std::vector<std::string> aliases;

  // Split a "22:" payload into this shape. The first field is Vensim's
  // canonical name and the rest are aliases, with one exception: a leading bare
  // "$" is Vensim's spelling of the currency unit whose readable name follows
  // it, which XMILE writes as <unit name="Dollar"><eqn>$</eqn>. Recognizing it
  // only in the leading position (and only when a name follows) is what makes
  // MdlPayload an exact inverse -- see the ordering note there.
  static UnitEquiv ParseMdlPayload(const std::string &payload) {
    std::vector<std::string> fields;
    for (size_t start = 0;;) {
      const size_t comma = payload.find(',', start);
      if (comma == std::string::npos) {
        fields.push_back(payload.substr(start));
        break;
      }
      fields.push_back(payload.substr(start, comma - start));
      start = comma + 1;
    }
    UnitEquiv equiv;
    size_t next = 0;
    if (fields.size() >= 2 && fields[0] == "$")
      equiv.eqn = fields[next++];
    equiv.name = fields[next++];
    equiv.aliases.assign(fields.begin() + next, fields.end());
    return equiv;
  }

  // Flatten back to a "22:" payload. A "$" equation leads, reproducing Vensim's
  // own spelling and inverting ParseMdlPayload exactly, so every "22:" line
  // read from a .mdl re-emits byte for byte. Any other equation is a formula
  // Vensim cannot express; it is emitted as a trailing-name field rather than
  // dropped, which is the closest available Vensim reading ("another spelling
  // of this unit") and keeps the text recoverable by a human or a later pass.
  //
  // Sanitizing happens HERE, at the point the .mdl field is built, not when the
  // declaration is read: a ',' would mis-split this line and a '|' or newline
  // would terminate it on re-import (#849), but all three are perfectly legal
  // inside an XMILE <unit>, so neutralizing them at read time would corrupt the
  // XMILE -> XMILE path for a hazard only the .mdl path has.
  std::string MdlPayload() const {
    std::vector<std::string> fields;
    const bool eqnLeads = eqn == "$";
    if (eqnLeads)
      fields.push_back(eqn);
    fields.push_back(name);
    if (!eqn.empty() && !eqnLeads)
      fields.push_back(eqn);
    fields.insert(fields.end(), aliases.begin(), aliases.end());

    std::string payload;
    for (size_t i = 0; i < fields.size(); i++) {
      if (i)
        payload += ',';
      payload += mdl::SanitizeFreeText(fields[i], mdl::FreeTextLineMode::SingleLine, ",");
    }
    return payload;
  }
};

enum Integration_Type { Integration_Type_EULER, Integration_Type_RK2, Integration_Type_RK4 };
class View {
public:
  virtual bool UpgradeGhost(Variable *var) = 0;
  virtual bool AddFlowDefinition(Variable *var, Variable *in, Variable *out) = 0;
  virtual bool AddVarDefinition(Variable *var, int x, int y) = 0;
  virtual void CheckLinksIn() = 0;
  virtual void CheckGhostOwners() = 0;
  virtual bool empty() const = 0;
  // just a placeholder to derive from
};
class Model {
public:
  Model(void);
  ~Model(void);
  bool UnitsCheck(void) {
    return false;
  }
  bool AnalyzeEquations(void);
  bool Simulate(void);
  SymbolNameSpace *GetNameSpace(void) {
    return &mSymbolNameSpace;
  }
  Equation *AddUnnamedVariable(ExpressionFunctionMemory *e);
  bool RenameVariable(Variable *v, const std::string &newname);
  void GenerateCanonicalNames(void);
  void GenerateShortNames(void);
  bool OutputComputable(bool wantshort);
  // ParseXMILE constructs an XmileReader on the fly and drives it. It
  // populates the Model but does not run the post-parse pipeline
  // (MarkVariableTypes/AdjustGroupNames/CheckGhostOwners) -- callers do that,
  // matching the VensimParse contract. errs collects descriptive failure
  // messages for the caller. It also records that this Model's contents came
  // off an XMILE document (bFromXmile below), which PrintXMILE reads back.
  bool ParseXMILE(const std::string &filename, const char *contents, size_t len, std::vector<std::string> &errs);
  // RunPostParsePipeline runs the four-step sequence that every entry point
  // must execute after the parser populates the Model and before the writer
  // serializes it. AdjustGroupNames mutates ModelGroup::sName on collision, so
  // both sides of a round-trip comparison must apply it to stay aligned.
  void RunPostParsePipeline();
  bool MarkVariableTypes(SymbolNameSpace *ns);
  void AdjustGroupNames();
  void CheckGhostOwners();
  void AttachStragglers();  // try to get diagramatic stuff right
  void MakeViewNamesUnique();
  // Give every stock a flow in its own view; see Variable::LocalizeCrossViewFlows.
  // Main namespace only: macros have no views. Called by XMILEGenerator::Print
  // on the module path, not from the post-parse pipeline.
  void LocalizeCrossViewFlows();
  // Serializes through XMILEGenerator. Emits the module decomposition (one
  // <model> per group or per view) only for a model that did NOT come from
  // XMILE; see bFromXmile.
  std::string PrintXMILE(bool isCompact, std::vector<std::string> &errs, double xscale, double yscale);
  std::string PrintMDL(std::vector<std::string> &errs);
  // Diagnostics for bare `*` wildcards that ResolveWildcardSubscripts could not
  // bind to a concrete dimension (a `*` on a non-arrayed or equationless
  // target). Both writers consult this before serializing: an unbound wildcard
  // would render as a literal `*`, which is invalid Vensim MDL and does not
  // re-parse as XMILE, so its presence makes the conversion fail cleanly rather
  // than shipping broken output. Empty for well-formed models and for every
  // Vensim/Dynamo input (those never produce null bang entries).
  const std::vector<std::string> &UnresolvedWildcards() const {
    return vUnresolvedWildcards;
  }

  double GetConstanValue(const char *var, double defval);
  UnitExpression *GetUnits(const char *var);
  std::vector<UnitEquiv> &UnitEquivs() {
    return vUnitEquivs;
  }
  // Provenance for consumers that must know whether a field the .mdl format
  // cannot carry (a UnitEquiv equation) could have survived the trip; see
  // bFromXmile.
  bool FromXmile() const {
    return bFromXmile;
  }
  void SetUnwanted(const char *var, const char *nametouse);
  std::vector<Variable *> GetVariables(SymbolNameSpace *ns = NULL);
  void AddView(View *view) {
    vViews.push_back(view);
  }
  std::vector<View *> &Views() {
    return vViews;
  }

  std::vector<MacroFunction *> &MacroFunctions() {
    return mMacroFunctions;
  }
  void SetMacroFunctions(std::vector<MacroFunction *> set) {
    mMacroFunctions = set;
  }
  void SetIntegrationType(Integration_Type type) {
    iIntegrationType = type;
  }
  Integration_Type IntegrationType() {
    return iIntegrationType;
  }
  std::vector<ModelGroup *> &Groups() {
    return vGroups;
  }

  void SetAsSectors(bool set) {
    bAsSectors = set;
  }
  bool AsSectors() const {
    return bAsSectors;
  }

  void SetLetterPolarity(bool set) {
    bLetterPolarity = set;
  }
  bool LetterPolarity() const {
    return bLetterPolarity;
  }

  double initial_time() const {
    return _initial_time;
  }
  void set_initial_time(double set) {
    _initial_time = set;
  }
  double final_time() const {
    return _final_time;
  }
  void set_finall_time(double set) {
    _final_time = set;
  }
  double dt() const {
    return _dt;
  }
  void set_dt(double set) {
    _dt = set;
  }
  void set_from_dynamo(bool set) {
    bFromDyanmo = set;
  }

private:
  bool OrderEquations(ContextInfo *info, bool tonly);
  bool SetupVariableStates(int pass);
  bool ValidatePlaceholderVars(void);
  bool OrganizeSubscripts(void);
  void ClearCompEquations(void);

  // Bind bare `*` wildcard subscripts to concrete dimensions. The XMILE reader
  // encodes `[*]` as a BANG_SYMBOL entry with a null symbol because the
  // referenced variable's dimensions are not known at equation-parse time; this
  // pass -- run from RunPostParsePipeline after MarkVariableTypes has
  // established element/family ownership -- rewrites each such entry to the
  // referenced variable's dimension family at that position, the shape both
  // writers emit (`SUM(a[*])` -> Vensim `SUM(a[DimA!])`). A no-op for Vensim /
  // Dynamo models, which never produce null bang entries. An unresolvable
  // wildcard (a `*` on a variable that is not arrayed at that position) is
  // recorded in vUnresolvedWildcards; both PrintMDL and PrintXMILE reject such
  // a model rather than emitting an invalid literal `*`.
  void ResolveWildcardSubscripts(SymbolNameSpace *ns);
  void ResolveWildcardsInExpr(Expression *e);
  void ResolveWildcardsInVarRef(ExpressionVariable *ev);
  static Symbol *FamilyAtPosition(Variable *target, int pos);

  SymbolNameSpace mSymbolNameSpace;
  std::vector<ModelGroup *> vGroups;
  std::vector<View *> vViews;
  std::vector<Variable *> vUnamedVars;
  // std::vector<Equation *>vConstantComps ; // actually just assignment
  std::vector<Equation *> vInitialTimeComps;
  std::vector<Equation *> vInitialComps;
  std::vector<Equation *> vUnchangingComps;
  std::vector<Equation *> vActiveComps;
  std::vector<Equation *> vRateComps;
  std::vector<MacroFunction *> mMacroFunctions;
  std::vector<UnitEquiv> vUnitEquivs;
  std::vector<std::string> vUnresolvedWildcards;
  /* the last could be part of active but it is helpful to split
     out when creating equations for a computer language */
  double _initial_time;
  double _final_time;
  double _dt;
  int iNLevel;
  int iNAux;
  Integration_Type iIntegrationType;
  double *dLevel;
  double *dRate;
  double *dAux;
  bool bAsSectors;
  bool bLetterPolarity;
  bool bFromDyanmo;
  // Provenance, set by ParseXMILE, read by PrintXMILE -- deliberately NOT
  // folded into bAsSectors, which is the caller's stated preference and stays
  // the caller's to set in either order. Rolling a model up into Stella modules
  // adds structure Vensim and Dynamo cannot express, so it is a nicety on those
  // inputs; applied to a document that already IS XMILE it is a structural
  // rewrite, and the shape it emits -- sibling <model> elements -- is exactly
  // what XmileReader::ProcessFile refuses as a <module> submodel. Emitting an
  // XMILE-sourced model as sectors regardless is what keeps XMILE -> XMILE a
  // normalization this tool can read back rather than a one-way transformation.
  bool bFromXmile;
};

#endif
