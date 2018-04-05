#include "VmGenerator.h"
#include "SymbolTable.h"
#include "Expression.h"
#include <algorithm>

VmGeneratorFactory::VmGeneratorFactory()
	: m_rootSymbols(new SymbolTable)
	, m_tracer(new CallTracer)
{
}

VmGeneratorFactory::~VmGeneratorFactory()
{
	delete m_rootSymbols;
}

CodeGenerator *VmGeneratorFactory::create(std::ostream &out)
{
	return new VmGenerator(out, m_rootSymbols, m_tracer);
}

std::string VmGeneratorFactory::getOutFileName(std::string baseFileName) const 
{
	return baseFileName + ".vm";
}

///////////////////////////

VmGenerator::VmGenerator(std::ostream &out, SymbolTable *rootSymbols, CallTracer *tracer)
	: m_out(out)
	, m_tracer(tracer)
{
	m_symbols = rootSymbols->createSubTable();
}

VmGenerator::~VmGenerator()
{
	m_symbols->toParentAndDiscard();
}

bool VmGenerator::startClass(const std::string &name)
{
	m_classContext = name;
	if (!m_symbols->add(SymbolTable::Symbol{ SymbolTable::Kind::Class, SymbolTable::Type::Class, name, name, 0 })) {
		return setError("Duplicate symbol class " + name);
	}
	return true;
}

bool VmGenerator::endClass()
{
	m_classContext.clear();
	return true;
}

bool VmGenerator::declareStaticVariables(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names)
{
	for (auto n : names) {
		if (!m_symbols->add(SymbolTable::Symbol{ SymbolTable::Kind::Static, type, classType, n, 0 })) {
			return setError("Dupicate symbol static " + n);
		}
	}
	return true;
}

bool VmGenerator::declareFieldVariables(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names)
{
	for (auto n : names) {
		if (!m_symbols->add(SymbolTable::Symbol{ SymbolTable::Kind::Field, type, classType, n, 0 })) {
			return setError("Dupicate symbol field " + n);
		}
	}
	return true;
}

bool VmGenerator::startConstructor(const std::string &className, const std::string &funcName)
{
	m_thisContext = m_classContext;
	return doStartRoutine(SymbolTable::Kind::Func, funcName, SymbolTable::Type::Class, m_classContext, true);
}

bool VmGenerator::endConstructor()
{
	return doEndRoutine();
}

bool VmGenerator::startMethod(SymbolTable::Type retType, const std::string &retName, const std::string &funcName)
{
	m_thisContext = m_classContext;
	return doStartRoutine(SymbolTable::Kind::Func, funcName, retType, retName, false);
}

bool VmGenerator::endMethod()
{
	return doEndRoutine();
}

bool VmGenerator::startFunction(SymbolTable::Type retType, const std::string &retName, const std::string &funcName)
{
	return doStartRoutine(SymbolTable::Kind::Func, funcName, retType, retName, true);
}

bool VmGenerator::endFunction()
{
	return doEndRoutine();
}

bool VmGenerator::declareParameters(const std::vector<Parameter> &params)
{
	for (auto p : params) {
		if (!m_symbols->add(SymbolTable::Symbol{ SymbolTable::Kind::Argument, p.type, p.classType, p.name, 0 })) {
			return setError("Dupicate symbol arg " + p.name);
		}
	}
	return true;
}

bool VmGenerator::startSubroutineBody()
{
	return true;
}

bool VmGenerator::finishLocals()
{
	m_out << "function " << fullName(m_curFunc);

	// Get number of locals
	int ct = std::count_if(m_symbols->begin(), m_symbols->end(), [=](auto x)->bool {return x.second.kind == SymbolTable::Kind::Var; });
	m_out << " " << ct << "\n";
	return true;
}

bool VmGenerator::endSubroutineBody()
{
	return true;
}

bool VmGenerator::declareVariable(SymbolTable::Type type, const std::string &classType, const std::vector<std::string> &names)
{
	for (auto n : names) {
		if (!m_symbols->add(SymbolTable::Symbol{ SymbolTable::Kind::Var, type, classType, n, 0 })) {
			return setError("Dupicate symbol var " + n);
		}
	}
	return true;
}

bool VmGenerator::startStatements()
{
	return true;
}

bool VmGenerator::endStatements()
{
	return true;
}

bool VmGenerator::beginWhile(Expression *cond, std::string &token)
{
	return true;
}

bool VmGenerator::endWhile(const std::string &token)
{
	return true;
}

bool VmGenerator::beginIf(Expression *cond, std::string &token)
{
	return true;
}

bool VmGenerator::insertElse(const std::string &token)
{
	return true;
}

bool VmGenerator::endIf(const std::string &token)
{
	return true;
}

bool VmGenerator::writeLet(Term *lhs, Expression *rhs)
{
	return true;
}

bool VmGenerator::writeDo(Term *call)
{
	if (call->type != Term::Call) {
		return setError("Need call in do");
	}
	bool ok = writeCall(call);
	// Pop the return value (not needed in do)
	m_out << "pop temp 0\n";
	return ok;
}

bool VmGenerator::writeReturn(Expression *ret)
{
	if (!ret) {
		// Functions must have a return value
		m_out << "push constant 0\n";
	} else {
		writeExpression(ret);
	}
	m_out << "return\n";
	return true;
}

bool VmGenerator::doStartRoutine(SymbolTable::Kind kind, const std::string &funcName, SymbolTable::Type type, const std::string &clsType, bool asStatic)
{
	m_symbols = m_symbols->createSubTable();
	if (!m_symbols->add(SymbolTable::Symbol{ kind, type, clsType, fullName(funcName), 0 })) {
		return setError("Duplicate symbol " + fullName(funcName));
	}
	m_tracer->addDef(fullName(funcName), asStatic, {});
	m_curFunc = funcName;
	return true;
}

bool VmGenerator::doEndRoutine()
{
	m_symbols = m_symbols->toParentAndDiscard();
	m_thisContext.clear();
	m_curFunc.clear();
	return true;
}

bool VmGenerator::writeCall(Term  *term)
{
	std::string target = prepareCall(term);
	for (auto e : term->data.callTerm.params) {
		if (!writeExpression(e))
			return false;
	}
	m_out << "call " << target << " " << term->data.callTerm.params.size() << "\n";
	return true;
}

bool VmGenerator::writeTerm(Term *term)
{
	switch (term->type) {
	case Term::IntConst:
		m_out << "push constant " << term->data.intTerm.value << "\n";
		return true;
	case Term::StringConst:
	case Term::KeywordConst:
	case Term::Variable:
	case Term::Array:
	case Term::Call:
	case Term::Bracketed:
		return writeExpression(term->data.bracketTerm.content);
	case Term::Unary:
		break;
	}
	return false;
}

bool VmGenerator::writeExpression(Expression *ex)
{
	bool ok = writeTerm(ex->term());
	while (!ex->isSingleTerm()) {
		char op = ex->op();
		ex = ex->right();
		ok &= writeTerm(ex->term());
		ok &= writeOp(op);
	}
	return ok;
}

bool VmGenerator::writeOp(char op)
{
	switch (op) {
	case '+':
		m_out << "add\n";
		return true;
	case '-':
		m_out << "sub\n";
		return true;
	case '&':
		m_out << "and\n";
		return true;
	case '|':
		m_out << "or\n";
		return true;
	case '<':
		m_out << "lt\n";
		return true;
	case '>':
		m_out << "gt\n";
		return true;
	case '=':
		m_out << "eq\n";
		return true;
	case '*':
		m_out << "call Math.multiply 2\n";
		m_tracer->addCall("Math.multiply", true, {}, {}, 0);
		return true;
	case '/':
		m_out << "call Math.divide 2\n";
		m_tracer->addCall("Math.divide", true, {}, {}, 0);
		return true;
	}
	return false;
}

std::string VmGenerator::prepareCall(Term *term)
{
	std::string realTarget;
	bool asStatic;

	if (term->data.callTerm.target.empty()) {
		if (m_thisContext.empty()) {
			asStatic = true;
			realTarget = m_classContext;
		} else {
			asStatic = false;
			realTarget = m_thisContext; // Always same as classContext...
		}
	} else if (term->data.callTerm.target == "this") {
		asStatic = false;
		realTarget = m_thisContext; // Always same as classContext...
	} else {
		SymbolTable::Symbol s = m_symbols->get(term->data.callTerm.target);
		if (s.kind == SymbolTable::Kind::NONE) {
			// Assume a class target
			realTarget = term->data.callTerm.target;
			asStatic = true;
		} else if (s.kind == SymbolTable::Kind::Class) {
			realTarget = term->data.callTerm.target;
			asStatic = true;
		} else {
			realTarget = s.classType;
			asStatic = false;
		}
	}

	std::string fullTarget = realTarget + "." + term->data.callTerm.function;
	m_tracer->addCall(fullTarget, asStatic, {}, {}, 0);
	return fullTarget;
}

bool VmGenerator::setError(const std::string &err)
{
	if (m_error.empty()) m_error = err;
	return false;
}

std::string VmGenerator::fullName(const std::string &funcName) const
{
	return m_classContext + "." + funcName;
}
