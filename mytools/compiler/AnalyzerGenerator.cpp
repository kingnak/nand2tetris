#include "AnalyzerGenerator.h"
#include "XMLHelper.hpp"
#include <map>
#include <sstream>
#include "Expression.h"
#include "SymbolTable.h"

using namespace std;

AnalyzerGeneratorFactory::AnalyzerGeneratorFactory(bool withSymbols)
	: m_rootSymbols(nullptr)
	, m_calls(nullptr)
{
	if (withSymbols) {
		m_rootSymbols = new SymbolTable;
		m_calls = new CallTracer;
	}
}

AnalyzerGeneratorFactory::~AnalyzerGeneratorFactory()
{
	if (m_calls) {
		m_calls->injectDefaults();
		m_calls->verify();
	}

	delete m_rootSymbols;
	delete m_calls;
}

static map<SymbolTable::Type, string> s_datatypes{
	{ SymbolTable::Type::None, "None" },
	{ SymbolTable::Type::Void, "void" },
	{ SymbolTable::Type::Int, "int" },
	{ SymbolTable::Type::Char, "char" },
	{ SymbolTable::Type::Boolean, "boolean" },
	{ SymbolTable::Type::Class, "Class" }
};

static map<SymbolTable::Kind, string> s_varkinds{
	{ SymbolTable::Kind::NONE, "NONE" },
	{ SymbolTable::Kind::Static, "Static" },
	{ SymbolTable::Kind::Field, "Field" },
	{ SymbolTable::Kind::Argument, "Argument" },
	{ SymbolTable::Kind::Var, "Var" },
	{ SymbolTable::Kind::Class, "Class" },
	{ SymbolTable::Kind::Func, "Func" }
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
	if (m_symbols)
		m_symbols = m_symbols->createSubTable(); 
	else 
		m_symbols = new SymbolTable;

	print("<class>");
	m_ident++;
	printKeyword("class");
	m_symbols->add(SymbolTable::Symbol{ SymbolTable::Kind::Class, SymbolTable::Type::Class, name, name, 0 });
	printIdent(name, true);
	printSymbol('{');
	m_clsContext = name;
	return true;
}

bool AnalyzerGenerator::endClass()
{
	printSymbol('}');
	m_ident--;
	print("</class>");
	m_clsContext = "";
	m_symbols = m_symbols->toParentAndDiscard();
	return true;
}

bool AnalyzerGenerator::declareStaticVariables(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names)
{
	print("<classVarDec>");
	m_ident++;
	doPrintVar(SymbolTable::Kind::Static, type, classType, names);
	m_ident--;
	print("</classVarDec>");
	return true;
}

bool AnalyzerGenerator::declareFieldVariables(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names)
{
	print("<classVarDec>");
	m_ident++;
	doPrintVar(SymbolTable::Kind::Field, type, classType, names);
	m_ident--;
	print("</classVarDec>");
	return true;
}

bool AnalyzerGenerator::startConstructor(const std::string &className, const std::string &funcName)
{
	m_thisContext = m_clsContext;
	doPrintSubroutineStart("constructor", SymbolTable::Type::Class, className, funcName, true);
	return true;
}

bool AnalyzerGenerator::startMethod(SymbolTable::Type retType, const std::string &retName, const std::string &funcName)
{
	m_thisContext = m_clsContext;
	doPrintSubroutineStart("method", retType, retName, funcName, false);
	return true;
}

bool AnalyzerGenerator::startFunction(SymbolTable::Type retType, const std::string &retName, const std::string &funcName)
{
	doPrintSubroutineStart("function", retType, retName, funcName, true);
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
		m_symbols->add(SymbolTable::Symbol{ SymbolTable::Kind::Argument, it->type, it->classType, it->name, 0 });
		printIdent(it->name, true);
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

bool AnalyzerGenerator::declareVariable(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names)
{
	print("<varDec>");
	m_ident++;
	doPrintVar(SymbolTable::Kind::Var, type, classType, names);
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
		printIdent(lhs->data.variableTerm.identifier, false);
	} else if (lhs->type == Term::Array) {
		printIdent(lhs->data.arrayTerm.identifier, false);
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
	while (!e->isSingleTerm()) {
		printSymbol(e->op());
		e = e->right();
		ok &= printTerm(e->term());
	}
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
		printIdent(t->data.variableTerm.identifier, false);
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
		printIdent(t->data.arrayTerm.identifier, false);
		printSymbol('[');
		ok = printExpression(t->data.arrayTerm.index);
		printSymbol(']');
		break;
	case Term::Call:
		printCall(t);
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

void AnalyzerGenerator::printCall(Term *t)
{
	std::string realTarget;
	bool asStatic = false;
	if (t->data.callTerm.target.length() > 0) {
		// Special case: This
		if (t->data.callTerm.target == "this") {
			printKeyword("this");
			realTarget = m_thisContext;
		} else {
			// If this is an unknown variable, assume a static call
			if (!m_symbols->containsRecursive(t->data.callTerm.target)) {
				asStatic = true;
				printIdent(SymbolTable::Symbol{ SymbolTable::Kind::Class, SymbolTable::Type::Class, t->data.callTerm.target, t->data.callTerm.target, 0 }, false);
				realTarget = t->data.callTerm.target;
			} else {
				printIdent(t->data.callTerm.target, false);
				realTarget = m_symbols->get(t->data.callTerm.target).classType;
				asStatic = m_symbols->get(t->data.callTerm.target).kind == SymbolTable::Kind::Class;
			}
		}
		printSymbol('.');
	} else if (m_thisContext.length() > 0) {
		realTarget = m_thisContext;
	} else {
		realTarget = m_clsContext;
		asStatic = true;
	}

	if (m_withSymbols)
		realTarget = realTarget + "." + t->data.callTerm.function;
	else
		realTarget = t->data.callTerm.function;

	if (m_calls) m_calls->addCall(realTarget, asStatic, {}, {}, 0);

	printIdent(SymbolTable::Symbol{ SymbolTable::Kind::Func, SymbolTable::Type::None, "", realTarget, 0 }, false);
	printSymbol('(');
	print("<expressionList>");
	m_ident++;
	for (auto it = t->data.callTerm.params.begin(); it != t->data.callTerm.params.end(); ++it) {
		if (it != t->data.callTerm.params.begin()) {
			printSymbol(',');
		}
		printExpression(*it);
	}
	m_ident--;
	print("</expressionList>");
	printSymbol(')');
}

void AnalyzerGenerator::printIdent(const SymbolTable::Symbol &s, bool defining)
{
	if (m_withSymbols) {
		stringstream ss;
		ss << "<identifier ";
		if (s.kind == SymbolTable::Kind::NONE) {
			ss << "type=\"undefined\" ";
		} else {
			if (s.type == SymbolTable::Type::Class)
				ss << "type=\"" << s.classType << "\" ";
			else
				ss << "type=\"" << s_datatypes.find(s.type)->second << "\" ";
			ss << "kind=\"" << s_varkinds.find(s.kind)->second << "\" ";
			switch (s.kind) {
			case SymbolTable::Kind::Static:
			case SymbolTable::Kind::Field:
			case SymbolTable::Kind::Argument:
			case SymbolTable::Kind::Var:
				ss << "nr=\"" << s.order << "\" ";
			}
		}
		if (defining)
			ss << "access=\"define\">";
		else
			ss << "access=\"use\">";
		ss << xmlClean(s.name);
		ss << "</identifier>";
		print(ss.str());
	} else {
		print("<identifier> " + xmlClean(s.name) + " </identifier>");
	}
}

void AnalyzerGenerator::printIdent(const std::string &ident, bool defining)
{
	if (!m_symbols->containsRecursive(ident)) {
		SymbolTable::Symbol s;
		s.name = ident;
		printIdent(s, defining);
	} else 
		printIdent(m_symbols->get(ident), defining);
}

void AnalyzerGenerator::printSymbol(char sym)
{
	print("<symbol> " + xmlClean(sym) + " </symbol>");
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

void AnalyzerGenerator::doPrintType(SymbolTable::Type type, const std::string &classType)
{
	if (type == SymbolTable::Type::Class) {
		printIdent(SymbolTable::Symbol{SymbolTable::Kind::Class, SymbolTable::Type::Class, classType, classType, 0}, false);
	} else {
		printKeyword(s_datatypes.find(type)->second);
	}
}

void AnalyzerGenerator::doPrintVar(SymbolTable::Kind kind, SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names)
{
	switch (kind) {
	case SymbolTable::Kind::Static:
		printKeyword("static");
		break;
	case SymbolTable::Kind::Field:
		printKeyword("field");
		break;
	case SymbolTable::Kind::Var:
		printKeyword("var");
		break;
	} 

	doPrintType(type, classType);
	for (auto it = names.begin(); it != names.end(); ++it) {
		if (it != names.begin()) {
			printSymbol(',');
		}
		m_symbols->add(SymbolTable::Symbol{ kind, type, classType, *it, 0 });
		printIdent(*it, true);
	}
	printSymbol(';');
}

void AnalyzerGenerator::doPrintSubroutineStart(const std::string &prefix, SymbolTable::Type retType, const std::string &retName, const std::string &funcName, bool asStatic)
{
	print("<subroutineDec>");
	m_ident++;
	printKeyword(prefix);
	doPrintType(retType, retName);
	m_symbols->add(SymbolTable::Symbol{ SymbolTable::Kind::Func, retType, retName, m_clsContext + "." + funcName, 0 });
	if (m_calls) m_calls->addDef(m_clsContext + "." + funcName, asStatic, {});
	printIdent(funcName, true);
	m_symbols = m_symbols->createSubTable();
}

void AnalyzerGenerator::doPrintSubroutineEnd()
{
	printSymbol('}');
	m_ident--;
	print("</subroutineBody>");
	m_ident--;
	print("</subroutineDec>");
	m_thisContext = "";
	m_symbols = m_symbols->toParentAndDiscard();
}

void AnalyzerGenerator::print(const std::string &str)
{
	m_out << string(2 * m_ident, ' ') << str << "\n";
}
