#pragma once

#include <ostream>
#include "CodeGenerator.h"

class AnalyzerGenerator : public CodeGenerator
{
public:
	AnalyzerGenerator(std::ostream &o) : m_out(o), m_ident(0) {}
	~AnalyzerGenerator() {}

	bool startClass(const std::string &name) override;
	bool endClass() override;
	bool declareStaticVariables(DataType type, const std::string &classType, const std::vector<std::string> &names) override;
	bool declareFieldVariables(DataType type, const std::string &classType, const std::vector<std::string> &names) override;
	bool startConstructor(const std::string &className, const std::string &funcName) override;
	bool startMethod(DataType retType, const std::string &retName, const std::string &funcName) override;
	bool startFunction(DataType retType, const std::string &retName, const std::string &funcName) override;
	bool declareParameters(const std::vector<Parameter> &params) override;
	bool startSubroutineBody() override;
	bool declareVariable(DataType type, const std::string &classType, const std::vector<std::string> &names) override;
	bool startStatements() override;
	bool endConstructor() override;
	bool endMethod() override;
	bool endFunction() override;

private:
	void printIdent(const std::string &ident);
	void printSymbol(char sym);
	void printKeyword(const std::string &keyword);
	void doPrintType(DataType type, const std::string &classType);
	void doPrintVar(const std::string &prefix, DataType type, const std::string &classType, const std::vector<std::string> &names);
	void doPrintSubroutineStart(const std::string &prefix, DataType retType, const std::string &retName, const std::string &funcName);
	void doPrintSubroutineEnd();
	void print(const std::string &str);

private:
	std::ostream &m_out;
	int m_ident;
};

class AnalyzerGeneratorFactory : public CodeGeneratorFactory
{
public:
	AnalyzerGenerator *create(std::ostream &out) override {
		return new AnalyzerGenerator(out);
	}
	std::string getOutFileName(std::string baseFileName) const override {
		return baseFileName + ".xml";
	}
};

