#pragma once

#include "CodeGenerator.h"

class SymbolTable;
class CallTracer;

class VmGeneratorFactory : public CodeGeneratorFactory
{
public:
	VmGeneratorFactory(bool debug);
	~VmGeneratorFactory();
	virtual CodeGenerator *create(std::ostream &out) override;
	virtual std::string getOutFileName(std::string baseFileName) const override;

private:
	SymbolTable *m_rootSymbols;
	CallTracer *m_tracer;
	bool m_debug;
};

class VmGenerator : public CodeGenerator
{
public:
	VmGenerator(std::ostream &out, SymbolTable *rootSymbols, CallTracer *tracer);
	~VmGenerator();

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
		return m_symbols;
	}

private:
	bool doStartRoutine(SymbolTable::Kind kind, const std::string &funcName, SymbolTable::Type type, const std::string &clsType, bool asStatic);
	bool doEndRoutine();

	bool writeCall(Term *term);
	bool writeTerm(Term *term);
	bool writeExpression(Expression *ex);
	bool writeOp(char op);
	bool writeUnary(char op);
	bool writeVar(const std::string &var);

	struct CallData {
		std::string target;
		bool asStatic;
		std::string obj;
	};
	CallData prepareCall(Term *term);

private:
	bool setError(const std::string &err);
	std::string fullName(const std::string &funcName) const;
	std::string newLabelToken();

private:
	std::ostream &m_out;
	SymbolTable *m_symbols;
	CallTracer *m_tracer;
	std::string m_error;

	std::string m_classContext;
	std::string m_thisContext;

	std::string m_curFunc;

	int m_nextLabelToken;

	enum class RoutineType {
		None, Ctor, Method, Function
	} m_curFuncType;
};
