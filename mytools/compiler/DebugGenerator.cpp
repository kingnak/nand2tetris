#include "DebugGenerator.h"
#include "Expression.h"

static std::map<SymbolTable::Type, std::string> s_datatypes{
	{ SymbolTable::Type::None, "None" },
	{ SymbolTable::Type::Void, "void" },
	{ SymbolTable::Type::Int, "int" },
	{ SymbolTable::Type::Char, "char" },
	{ SymbolTable::Type::Boolean, "boolean" },
	{ SymbolTable::Type::Class, "Class" }
};
std::string dataTypeName(SymbolTable::Type t, const std::string &n) {
	if (t == SymbolTable::Type::Class) return n;
	return s_datatypes.find(t)->second;
}

DebugGenerator::DebugGenerator(CodeGenerator *inner, std::ostream &out)
	: m_inner(inner)
	, m_out(out)
{
}

bool DebugGenerator::startClass(const std::string &name)
{
	m_out << "// class " << name << "\n";
	m_curClass = name;
	return m_inner->startClass(name);
}

bool DebugGenerator::endClass()
{
	m_out << "// end class " << m_curClass << "\n";
	m_curClass.clear();
	return m_inner->endClass();
}

bool DebugGenerator::declareStaticVariables(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names)
{
	bool ret = m_inner->declareStaticVariables(type, classType, names);
	for (auto n : names)
		m_out << "// static " << dataTypeName(type, classType) << " " << n << " (static " << symbols()->get(n).order << ")" << "\n";
	return ret;
}

bool DebugGenerator::declareFieldVariables(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names)
{
	bool ret = m_inner->declareFieldVariables(type, classType, names);
	for (auto n : names)
		m_out << "// field " << dataTypeName(type, classType) << " " << n << " (this " << symbols()->get(n).order << ")" << "\n";
	return ret;
}

bool DebugGenerator::startConstructor(const std::string &className, const std::string &funcName)
{
	m_out << "\n\n// constructor " << className << " " << funcName << "\n";
	m_curFunc = funcName;
	return m_inner->startConstructor(className, funcName);
}

bool DebugGenerator::endConstructor()
{
	m_out << "// End of constructor " << m_curFunc << "\n";
	m_curFunc.clear();
	return m_inner->endConstructor();
}

bool DebugGenerator::startMethod(SymbolTable::Type retType, const std::string &retName, const std::string &funcName)
{
	m_out << "\n\n// method " << dataTypeName(retType, retName) << " " << funcName << "\n";
	m_curFunc = funcName;
	return m_inner->startMethod(retType, retName, funcName);
}

bool DebugGenerator::endMethod()
{
	m_out << "// End of method " << m_curFunc << "\n";
	m_curFunc.clear();
	return m_inner->endMethod();
}

bool DebugGenerator::startFunction(SymbolTable::Type retType, const std::string &retName, const std::string &funcName)
{
	m_out << "\n\n// function " << dataTypeName(retType, retName) << " " << funcName << "\n";
	m_curFunc = funcName;
	return m_inner->startFunction(retType, retName, funcName);
}

bool DebugGenerator::endFunction()
{
	m_out << "// End of function " << m_curFunc << "\n";
	m_curFunc.clear();
	return m_inner->endFunction();
}

bool DebugGenerator::declareParameters(const std::vector<Parameter> &params)
{
	bool ret = m_inner->declareParameters(params);
	for (auto p : params)
		m_out << "// param " << dataTypeName(p.type, p.classType) << " " << p.name << " (arg " << symbols()->get(p.name).order << ")" << "\n";
	return ret;
}

bool DebugGenerator::startSubroutineBody()
{
	return m_inner->startSubroutineBody();
}

bool DebugGenerator::finishLocals()
{
	return m_inner->finishLocals();
}

bool DebugGenerator::endSubroutineBody()
{
	return m_inner->endSubroutineBody();
}

bool DebugGenerator::declareVariable(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names)
{
	bool ret = m_inner->declareVariable(type, classType, names);
	for (auto n : names)
		m_out << "// var " << dataTypeName(type, classType) << " " << n << " (local " << symbols()->get(n).order << ")" << "\n";
	return ret;
}

bool DebugGenerator::startStatements()
{
	return m_inner->startStatements();
}

bool DebugGenerator::endStatements()
{
	return m_inner->endStatements();
}

bool DebugGenerator::beginWhile(Expression *cond, std::string &token)
{
	m_out << "\n// while (";
	writeExpression(cond);
	m_out << ") {\n";
	return m_inner->beginWhile(cond, token);
}

bool DebugGenerator::endWhile(const std::string &token)
{
	m_out << "// }\n// end while\n";
	return m_inner->endWhile(token);
}

bool DebugGenerator::beginIf(Expression *cond, std::string &token)
{
	m_out << "\n// if (";
	writeExpression(cond);
	m_out << ") {\n";
	return m_inner->beginIf(cond, token);
}

bool DebugGenerator::insertElse(const std::string &token)
{
	m_out << "// } else { \n";
	return m_inner->insertElse(token);
}

bool DebugGenerator::noElse(const std::string &token)
{
	return m_inner->noElse(token);
}

bool DebugGenerator::endIf(const std::string &token)
{
	m_out << "// }\n// end if\n";
	return m_inner->endIf(token);
}

bool DebugGenerator::writeLet(Term *lhs, Expression *rhs)
{
	m_out << "\n// let ";
	if (lhs->type == Term::Variable) {
		m_out << lhs->data.variableTerm.identifier;
	} else if (lhs->type == Term::Array) {
		m_out << lhs->data.arrayTerm.identifier;
		m_out << '[';
		writeExpression(lhs->data.arrayTerm.index);
		m_out << ']';
	}
	m_out << " = ";
	writeExpression(rhs);
	m_out << ";\n";
	return m_inner->writeLet(lhs, rhs);
}

bool DebugGenerator::writeDo(Term *call)
{
	m_out << "\n// do "; 
	writeCall(call);
	m_out << ";\n";
	return m_inner->writeDo(call);
}

bool DebugGenerator::writeReturn(Expression *ret)
{
	m_out << "\n// return ";
	if (ret) writeExpression(ret);
	m_out << ";\n";
	return m_inner->writeReturn(ret);
}

void DebugGenerator::writeExpression(Expression *e)
{
	writeTerm(e->term());
	while (!e->isSingleTerm()) {
		m_out << ' ' << e->op() << ' ';
		e = e->right();
		writeTerm(e->term());
	}
}

void DebugGenerator::writeTerm(Term *term)
{
	switch (term->type) {
	case Term::IntConst:
		m_out << term->data.intTerm.value;
		return;
	case Term::StringConst:
		m_out << '"' << term->data.stringTerm.value << '"';
		return;
	case Term::KeywordConst:
		switch (term->data.keywordTerm.value) {
		case Tokenizer::Keyword::True:
			m_out << "true";
			return;
		case Tokenizer::Keyword::False:
			m_out << "false";
			return;
		case Tokenizer::Keyword::Null:
			m_out << "null";
			return;
		case Tokenizer::Keyword::This:
			m_out << "this";
			return;
		default:
			return;
		}
	case Term::Variable:
		m_out << term->data.variableTerm.identifier;
		return;
	case Term::Array:
		m_out << term->data.arrayTerm.identifier;
		m_out << '[';
		writeExpression(term->data.arrayTerm.index);
		m_out << ']';
		return;
	case Term::Call:
		writeCall(term);
		return;
	case Term::Bracketed:
		m_out << '(';
		writeExpression(term->data.bracketTerm.content);
		m_out << ')';
		return;
	case Term::Unary:
		m_out << term->data.unaryTerm.unaryOp;
		writeTerm(term->data.unaryTerm.content);
		return;
	}
}

void DebugGenerator::writeCall(Term *call)
{
	char c = ' ';
	m_out << call->data.callTerm.target << "." << call->data.callTerm.function << "(";
	for (auto p : call->data.callTerm.params) {
		m_out << c;
		writeExpression(p);
		c = ',';
	}
	m_out << " )";
}