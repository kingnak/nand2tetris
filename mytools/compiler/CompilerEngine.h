#pragma once

#include <ostream>

class CompilerEngine
{
public:
	virtual bool compileClass() = 0;
	virtual bool compileClassVarDec() = 0;
	virtual bool compileSubroutine() = 0;
	virtual bool compileParameterList() = 0;
	virtual bool compileVarDec() = 0;
	virtual bool compileStatements() = 0;
	virtual bool compileDo() = 0;
	virtual bool compileLet() = 0;
	virtual bool compileWhile() = 0;
	virtual bool compileIf() = 0;
	virtual bool compileExpression() = 0;
	virtual bool compileTerm() = 0;
	virtual bool compileExpressionList() = 0;

	virtual ~CompilerEngine() {}
};

class AbstractCompilerEngineFactory
{
public:
	virtual ~AbstractCompilerEngineFactory() {}
	virtual CompilerEngine *createEngine(std::ostream &out) = 0;
	virtual std::string generateOutName(const std::string &inBase) const = 0;
};
