#pragma once

#include <iostream>
#include <string>
#include <vector>

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
	virtual bool startMethod(DataType retType, const std::string &retName, const std::string &funcName) = 0;
	virtual bool startFunction(DataType retType, const std::string &retName, const std::string &funcName) = 0;
	virtual bool declareParameters(const std::vector<Parameter> &params) = 0;
	virtual bool startSubroutineBody() = 0;
	virtual bool declareVariable(DataType type, const std::string &classType, const std::vector<std::string> &names) = 0;
	virtual bool startStatements() = 0;
	virtual bool endConstructor() = 0;
	virtual bool endMethod() = 0;
	virtual bool endFunction() = 0;
};

class CodeGeneratorFactory
{
public:
	virtual ~CodeGeneratorFactory() {}
	virtual CodeGenerator *create(std::ostream &out) = 0;
	virtual std::string getOutFileName(std::string baseFileName) const = 0;
};
