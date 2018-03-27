#pragma once

#include <iostream>

class CodeGenerator
{
public:
	virtual ~CodeGenerator() {}

};

class CodeGeneratorFactory
{
public:
	virtual ~CodeGeneratorFactory() {}
	virtual CodeGenerator *create(std::ostream &out) = 0;
	virtual std::string getOutFileName(std::string baseFileName) const = 0;
};
