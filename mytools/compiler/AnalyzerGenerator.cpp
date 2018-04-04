#include "AnalyzerGenerator.h"
#include "XMLHelper.hpp"
#include <map>
#include <sstream>
#include "Expression.h"

using namespace std;

static map<CodeGenerator::DataType, string> s_datatypes{
	{ CodeGenerator::DataType::None, "None" },
	{ CodeGenerator::DataType::Void, "void" },
	{ CodeGenerator::DataType::Int, "int" },
	{ CodeGenerator::DataType::Char, "char" },
	{ CodeGenerator::DataType::Boolean, "boolean" },
	{ CodeGenerator::DataType::Class, "Class" }
};

static map<Tokenizer::Keyword, string> s_kw{
	{ Tokenizer::Keyword::Class, "class" },
	{ Tokenizer::Keyword::Method, "method" },
	{ Tokenizer::Keyword::Function, "function" },
	{ Tokenizer::Keyword::Constructor, "constructor" },
	{ Tokenizer::Keyword::Int, "int" },
	{ Tokenizer::Keyword::Boolean, "boolean" },
	{ Tokenizer::Keyword::Char, "char" },
	{ Tokenizer::Keyword::Void, "void" },
	{ Tokenizer::Keyword::Var, "var" },
	{ Tokenizer::Keyword::Static, "static" },
	{ Tokenizer::Keyword::Field, "field" },
	{ Tokenizer::Keyword::Let, "let" },
	{ Tokenizer::Keyword::Do, "do" },
	{ Tokenizer::Keyword::While, "while" },
	{ Tokenizer::Keyword::If, "if" },
	{ Tokenizer::Keyword::Else, "else" },
	{ Tokenizer::Keyword::Return, "return" },
	{ Tokenizer::Keyword::True, "true" },
	{ Tokenizer::Keyword::False, "false" },
	{ Tokenizer::Keyword::Null, "null" },
	{ Tokenizer::Keyword::This, "this" }
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
	printSymbol('}');
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

bool AnalyzerGenerator::startSubroutineBody()
{
	print("<subroutineBody>");
	m_ident++;
	printSymbol('{');
	return true;
}

bool AnalyzerGenerator::declareVariable(DataType type, const std::string &classType, const std::vector<std::string> &names)
{
	print("<varDec>");
	m_ident++;
	doPrintVar("var", type, classType, names);
	m_ident--;
	print("</varDec>");
	return true;
}

bool AnalyzerGenerator::startStatements()
{
	print("<statements>");
	m_ident++;
	return true;
}

bool AnalyzerGenerator::endStatements()
{
	m_ident--;
	print("</statements>");	
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

bool AnalyzerGenerator::beginWhile(Expression *cond, std::string &token)
{
	print("<whileStatement>");
	m_ident++;

	printKeyword("while");
	printSymbol('(');
	printExpression(cond);
	printSymbol(')');
	printSymbol('{');
	return true;
}

bool AnalyzerGenerator::endWhile(const std::string &token)
{
	printSymbol('}');
	m_ident--;
	print("</whileStatement>");
	return true;
}

bool AnalyzerGenerator::beginIf(Expression *cond, std::string &token)
{
	print("<ifStatement>");
	m_ident++;

	printKeyword("if");
	printSymbol('(');
	printExpression(cond);
	printSymbol(')');
	printSymbol('{');
	return true;
}

bool AnalyzerGenerator::insertElse(const std::string &token) 
{
	printSymbol('}');
	printKeyword("else");
	printSymbol('{');
	return true;
}

bool AnalyzerGenerator::endIf(const std::string &token) 
{
	printSymbol('}');
	m_ident--;
	print("</ifStatement>");
	return true;
}

bool AnalyzerGenerator::writeLet(Term *lhs, Expression *rhs)
{
	print("<letStatement>");
	m_ident++;
	printKeyword("let");
	if (lhs->type == Term::Variable) {
		printIdent(lhs->data.variableTerm.identifier);
	} else if (lhs->type == Term::Array) {
		printIdent(lhs->data.arrayTerm.identifier);
		printSymbol('[');
		if (!printExpression(lhs->data.arrayTerm.index))
			return false;
		printSymbol(']');
	} else {
		return false;
	}
	printSymbol('=');
	bool ok = printExpression(rhs);
	printSymbol(';');
	m_ident--;
	print("</letStatement>");
	return ok;
}

bool AnalyzerGenerator::writeDo(Term *call) 
{
	print("<doStatement>");
	m_ident++;
	
	printKeyword("do");
	bool ok = printTerm(call, false);
	printSymbol(';');

	m_ident--;
	print("</doStatement>");
	
	return ok;
}

bool AnalyzerGenerator::writeReturn(Expression *ret)
{
	print("<returnStatement>");
	m_ident++;

	printKeyword("return");
	bool ok = true;
	if (ret) ok = printExpression(ret);
	printSymbol(';');

	m_ident--;
	print("</returnStatement>");
	return ok;
}


bool AnalyzerGenerator::printExpression(Expression *e)
{
	print("<expression>");
	m_ident++;
	bool ok = printTerm(e->term());
	m_ident--;
	print("</expression>");
	return ok;
}

bool AnalyzerGenerator::printTerm(Term *t, bool wrapped)
{
	if (wrapped) {
		print("<term>");
		m_ident++;
	}
	bool ok = true;
	switch (t->type) {
	case Term::IntConst:
		printConstant(t->data.intTerm.value);
		break;
	case Term::StringConst:
		printConstant(t->data.stringTerm.value);
		break;
	case Term::KeywordConst:
		printKeyword(s_kw.find(t->data.keywordTerm.value)->second);
		break;
	case Term::Variable:
		printIdent(t->data.variableTerm.identifier);
		break;
	case Term::Unary:
		printSymbol(t->data.unaryTerm.unaryOp);
		ok = printTerm(t->data.unaryTerm.content);
		break;
	case Term::Bracketed:
		printSymbol('(');
		ok = printExpression(t->data.bracketTerm.content);
		printSymbol(')');
		break;
	case Term::Array:
		printIdent(t->data.arrayTerm.identifier);
		printSymbol('[');
		ok = printExpression(t->data.arrayTerm.index);
		printSymbol(']');
		break;
	case Term::Call:
		if (t->data.callTerm.target.length() > 0) {
			// Special case: This
			if (t->data.callTerm.target == "this") {
				printKeyword("this");
			} else {
				printIdent(t->data.callTerm.target);
			}
			printSymbol('.');
		}
		printIdent(t->data.callTerm.function);
		printSymbol('(');
		print("<expressionList>");
		m_ident++;
		for (auto it = t->data.callTerm.params.begin(); it != t->data.callTerm.params.end(); ++it) {
			if (it != t->data.callTerm.params.begin()) {
				printSymbol(',');
			}
			ok &= printExpression(*it);
		}
		m_ident--;
		print("</expressionList>");
		printSymbol(')');
		break;
	default:
		ok = false;
	}

	if (wrapped) {
		m_ident--;
		print("</term>");
	}

	return ok;
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

void AnalyzerGenerator::printConstant(int i)
{
	stringstream si;
	si << i;
	print("<integerConstant> " + si.str() + " </integerConstant>");
}

void AnalyzerGenerator::printConstant(const std::string &s)
{
	print("<stringConstant> " + s + " </stringConstant>");
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
	//m_ident--;
	//print("</statements>");
	printSymbol('}');
	m_ident--;
	print("</subroutineBody>");
	m_ident--;
	print("</subroutineDec>");
}

void AnalyzerGenerator::print(const std::string &str)
{
	m_out << string(2 * m_ident, ' ') << str << "\n";
}
