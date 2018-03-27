#pragma once

#include <ostream>
#include "CodeGenerator.h"

class AnalyzerGenerator : public CodeGenerator
{
public:
	AnalyzerGenerator(std::ostream &o) : m_out(o) {}

	/*
	virtual bool compileClass() override { return false; }
	virtual bool compileClassVarDec() override { return false; }
	virtual bool compileSubroutine() override { return false; }
	virtual bool compileParameterList() override { return false; }
	virtual bool compileVarDec() override { return false; }
	virtual bool compileStatements() override { return false; }
	virtual bool compileDo() override { return false; }
	virtual bool compileLet() override { return false; }
	virtual bool compileWhile() override { return false; }
	virtual bool compileIf() override { return false; }
	virtual bool compileExpression() override { return false; }
	virtual bool compileTerm() override { return false; }
	virtual bool compileExpressionList() override { return false; }
	*/
	virtual ~AnalyzerGenerator() {}

private:
	std::ostream &m_out;
};

class AnalyzerGeneratorFactory : public CodeGeneratorFactory
{
public:
	virtual AnalyzerGenerator *create(std::ostream &out) override {
		return new AnalyzerGenerator(out);
	}
	virtual std::string getOutFileName(std::string baseFileName) const override {
		return baseFileName + ".xml";
	}
};

