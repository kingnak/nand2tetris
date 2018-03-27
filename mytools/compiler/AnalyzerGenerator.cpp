#include "AnalyzerGenerator.h"
#include "XMLHelper.hpp"
#include <map>

using namespace std;

static map<CodeGenerator::DataType, string> s_datatypes{
	{ CodeGenerator::DataType::None, "None" },
	{ CodeGenerator::DataType::Void, "void" },
	{ CodeGenerator::DataType::Int, "int" },
	{ CodeGenerator::DataType::Char, "char" },
	{ CodeGenerator::DataType::Boolean, "boolean" },
	{ CodeGenerator::DataType::Class, "Class" }
};


bool AnalyzerGenerator::startClass(const std::string &name)
{
	print("<class>");
	m_ident++;
	printKeyword("class");
	printIdent(name);
	printSymbol('{');
	return true;
}

bool AnalyzerGenerator::endClass()
{
	m_ident--;
	print("</class>");
	return true;
}

bool AnalyzerGenerator::declareStaticVariables(DataType type, const std::string &classType, const std::vector<std::string> &names)
{
	print("<classVarDec>");
	m_ident++;
	doPrintVar("static", type, classType, names);
	m_ident--;
	print("</classVarDec>");
	return true;
}

bool AnalyzerGenerator::declareFieldVariables(DataType type, const std::string &classType, const std::vector<std::string> &names)
{
	print("<classVarDec>");
	m_ident++;
	doPrintVar("field", type, classType, names);
	m_ident--;
	print("</classVarDec>");
	return true;
}

bool AnalyzerGenerator::startConstructor(const std::string &className, const std::string &funcName)
{
	doPrintSubroutineStart("constructor", DataType::Class, className, funcName);
	return true;
}

bool AnalyzerGenerator::startMethod(DataType retType, const std::string &retName, const std::string &funcName)
{
	doPrintSubroutineStart("method", retType, retName, funcName);
	return true;
}

bool AnalyzerGenerator::startFunction(DataType retType, const std::string &retName, const std::string &funcName)
{
	doPrintSubroutineStart("function", retType, retName, funcName);
	return true;
}

bool AnalyzerGenerator::declareParameters(const std::vector<Parameter> &params)
{
	printSymbol('(');
	print("<parameterList>");
	m_ident++;
	for (auto it = params.begin(); it != params.end(); ++it) {
		if (it != params.begin())
			printSymbol(',');
		doPrintType(it->type, it->classType);
		printIdent(it->name);
	}
	m_ident--;
	print("</parameterList>");
	printSymbol(')');
	return true;
}

bool AnalyzerGenerator::endConstructor()
{
	doPrintSubroutineEnd();
	return true;
}

bool AnalyzerGenerator::endMethod()
{
	doPrintSubroutineEnd();
	return true;
}

bool AnalyzerGenerator::endFunction()
{
	doPrintSubroutineEnd();
	return true;
}

void AnalyzerGenerator::printIdent(const std::string &ident)
{
	print("<identifier> " + xmlClean(ident) + " </identifier>");
}

void AnalyzerGenerator::printSymbol(char sym)
{
	print("<symbol> " + string(1, sym) + " </symbol>");
}

void AnalyzerGenerator::printKeyword(const std::string &keyword)
{
	print("<keyword> " + keyword + " </keyword>");
}

void AnalyzerGenerator::doPrintType(DataType type, const std::string &classType)
{
	if (type == DataType::Class) {
		printIdent(classType);
	} else {
		printKeyword(s_datatypes.find(type)->second);
	}
}

void AnalyzerGenerator::doPrintVar(const std::string &prefix, DataType type, const std::string &classType, const std::vector<std::string> &names)
{
	printKeyword(prefix);
	doPrintType(type, classType);
	for (auto it = names.begin(); it != names.end(); ++it) {
		if (it != names.begin()) {
			printSymbol(',');
		}
		printIdent(*it);
	}
	printSymbol(';');
}

void AnalyzerGenerator::doPrintSubroutineStart(const std::string &prefix, DataType retType, const std::string &retName, const std::string &funcName)
{
	print("<subroutineDec>");
	m_ident++;
	printKeyword(prefix);
	doPrintType(retType, retName);
	printIdent(funcName);
}

void AnalyzerGenerator::doPrintSubroutineEnd()
{
	m_ident--;
	print("</subroutineDec>");
}

void AnalyzerGenerator::print(const std::string &str)
{
	m_out << string(2 * m_ident, ' ') << str << "\n";
}
