#pragma once

#include <ostream>
#include "CompilerEngine.h"

class AnalyzerEngine : public CompilerEngine
{
public:
	AnalyzerEngine(std::ostream &o) : m_out(o) {}

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

	virtual ~AnalyzerEngine() {}

private:
	std::ostream &m_out;
};

class AnalyzerEngineFactory : public AbstractCompilerEngineFactory
{
public:
	virtual AnalyzerEngine *createEngine(std::ostream &out) override {
		return new AnalyzerEngine(out);
	}
	virtual std::string generateOutName(const std::string &inBase) const override {
		return inBase + ".xml";
	}
};

