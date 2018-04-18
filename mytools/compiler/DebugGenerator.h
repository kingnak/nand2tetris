#pragma once

#include "CodeGenerator.h"

class DebugGenerator : public CodeGenerator
{
public:
	DebugGenerator(CodeGenerator *inner, std::ostream &out);
	virtual bool startClass(const std::string &name) override;
	virtual bool endClass() override;
	virtual bool declareStaticVariables(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names) override;
	virtual bool declareFieldVariables(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names) override;
	virtual bool startConstructor(const std::string &className, const std::string &funcName) override;
	virtual bool endConstructor() override;
	virtual bool startMethod(SymbolTable::Type retType, const std::string &retName, const std::string &funcName) override;
	virtual bool endMethod() override;
	virtual bool startFunction(SymbolTable::Type retType, const std::string &retName, const std::string &funcName) override;
	virtual bool endFunction() override;
	virtual bool declareParameters(const std::vector<Parameter> &params) override;
	virtual bool startSubroutineBody() override;
	virtual bool finishLocals() override;
	virtual bool endSubroutineBody() override;
	virtual bool declareVariable(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names) override;
	virtual bool startStatements() override;
	virtual bool endStatements() override;
	virtual bool beginWhile(Expression *cond, std::string &token) override;
	virtual bool endWhile(const std::string &token) override;
	virtual bool beginIf(Expression *cond, std::string &token) override;
	virtual bool insertElse(const std::string &token) override;
	virtual bool noElse(const std::string &token) override;
	virtual bool endIf(const std::string &token) override;

	virtual bool writeLet(Term *lhs, Expression *rhs) override;
	virtual bool writeDo(Term *call) override;
	virtual bool writeReturn(Expression *ret) override;

	virtual const SymbolTable *symbols() const override {
		return m_inner->symbols();
	}

private:
	void writeExpression(Expression *e);
	void writeTerm(Term *t);
	void writeCall(Term *t);

private:
	std::ostream &m_out;
	CodeGenerator *m_inner;

	std::string m_curFunc;
	std::string m_curClass;
};