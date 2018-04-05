#pragma once

#include <ostream>
#include "CodeGenerator.h"

class SymbolTable;

class AnalyzerGenerator : public CodeGenerator
{
public:
	AnalyzerGenerator(std::ostream &o, SymbolTable *rootSymbols, CallTracer *calls) : m_out(o), m_ident(0), m_symbols(rootSymbols), m_withSymbols(rootSymbols != nullptr), m_calls(calls) {}
	~AnalyzerGenerator() {}

	bool startClass(const std::string &name) override;
	bool endClass() override;
	bool declareStaticVariables(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names) override;
	bool declareFieldVariables(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names) override;
	bool startConstructor(const std::string &className, const std::string &funcName) override;
	bool endConstructor() override;
	bool startMethod(SymbolTable::Type retType, const std::string &retName, const std::string &funcName) override;
	bool endMethod() override;
	bool startFunction(SymbolTable::Type retType, const std::string &retName, const std::string &funcName) override;
	bool endFunction() override;
	bool declareParameters(const std::vector<Parameter> &params) override;
	bool startSubroutineBody() override;
	bool endSubroutineBody() override { return true; }
	bool declareVariable(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names) override;
	bool startStatements() override;
	bool endStatements() override;
	
	bool beginWhile(Expression *cond, std::string &token) override;
	bool endWhile(const std::string &token) override;

	bool beginIf(Expression *cond, std::string &token) override;
	bool insertElse(const std::string &token) override;
	bool endIf(const std::string &token) override;

	bool writeLet(Term *lhs, Expression *rhs) override;
	bool writeDo(Term *call) override;
	bool writeReturn(Expression *ret) override;

private:
	bool printExpression(Expression *e);
	bool printTerm(Term *t, bool wrapped = true);
	void printCall(Term *t);

private:
	void printIdent(const SymbolTable::Symbol &s, bool defining);
	void printIdent(const std::string &ident, bool defining);
	void printSymbol(char sym);
	void printKeyword(const std::string &keyword);
	void printConstant(int i);
	void printConstant(const std::string &s);
	void doPrintType(SymbolTable::Type type, const std::string &classType);
	void doPrintVar(SymbolTable::Kind kind, SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names);
	void doPrintSubroutineStart(const std::string &prefix, SymbolTable::Type retType, const std::string &retName, const std::string &funcName, bool asStatic);
	void doPrintSubroutineEnd();
	void print(const std::string &str);

private:
	std::ostream &m_out;
	int m_ident;
	SymbolTable *m_symbols;
	CallTracer *m_calls;
	bool m_withSymbols;
	std::string m_clsContext;
	std::string m_thisContext;
};

class AnalyzerGeneratorFactory : public CodeGeneratorFactory
{
public:
	AnalyzerGeneratorFactory(bool withSymbols);
	~AnalyzerGeneratorFactory();
	AnalyzerGenerator *create(std::ostream &out) override {
		return new AnalyzerGenerator(out, m_rootSymbols, m_calls);
	}
	std::string getOutFileName(std::string baseFileName) const override {
		return baseFileName + ".xml";
	}

private:
	bool m_withSymbols;
	SymbolTable *m_rootSymbols;
	CallTracer *m_calls;
};

