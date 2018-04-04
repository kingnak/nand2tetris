#pragma once

#include <iostream>
#include <string>
#include <vector>

struct Term;
class Expression;

class CodeGenerator
{
public:
	enum class DataType {
		None, Void, Int, Char, Boolean, Class
	};

	struct Parameter {
		DataType type;
		std::string classType;
		std::string name;
	};

	virtual ~CodeGenerator() {}

	virtual bool startClass(const std::string &name) = 0;
	virtual bool endClass() = 0;
	virtual bool declareStaticVariables(DataType type, const std::string &classType, const std::vector<std::string> &names) = 0;
	virtual bool declareFieldVariables(DataType type, const std::string &classType, const std::vector<std::string> &names) = 0;
	virtual bool startConstructor(const std::string &className, const std::string &funcName) = 0;
	virtual bool endConstructor() = 0;
	virtual bool startMethod(DataType retType, const std::string &retName, const std::string &funcName) = 0;
	virtual bool endMethod() = 0;
	virtual bool startFunction(DataType retType, const std::string &retName, const std::string &funcName) = 0;
	virtual bool endFunction() = 0;
	virtual bool declareParameters(const std::vector<Parameter> &params) = 0;
	virtual bool startSubroutineBody() = 0;
	virtual bool endSubroutineBody() = 0;
	virtual bool declareVariable(DataType type, const std::string &classType, const std::vector<std::string> &names) = 0;
	virtual bool startStatements() = 0;
	virtual bool endStatements() = 0;
	virtual bool beginWhile(Expression *cond, std::string &token) = 0;
	virtual bool endWhile(const std::string &token) = 0;
	virtual bool beginIf(Expression *cond, std::string &token) = 0;
	virtual bool insertElse(const std::string &token) = 0;
	virtual bool endIf(const std::string &token) = 0;
	
	virtual bool writeLet(Term *lhs, Expression *rhs) = 0;
	virtual bool writeDo(Term *call) = 0;
	virtual bool writeReturn(Expression *ret) = 0;
};

class CodeGeneratorFactory
{
public:
	virtual ~CodeGeneratorFactory() {}
	virtual CodeGenerator *create(std::ostream &out) = 0;
	virtual std::string getOutFileName(std::string baseFileName) const = 0;
};
