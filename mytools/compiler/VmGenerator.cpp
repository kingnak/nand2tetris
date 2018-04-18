#include "VmGenerator.h"
#include "DebugGenerator.h"
#include "SymbolTable.h"
#include "Expression.h"
#include <sstream>

VmGeneratorFactory::VmGeneratorFactory(bool debug)
	: m_rootSymbols(new SymbolTable)
	, m_tracer(new CallTracer)
	, m_debug(debug)
{
}

VmGeneratorFactory::~VmGeneratorFactory()
{
	delete m_rootSymbols;
}

CodeGenerator *VmGeneratorFactory::create(std::ostream &out)
{
	CodeGenerator *ret = new VmGenerator(out, m_rootSymbols, m_tracer);
	if (m_debug)
		ret = new DebugGenerator(ret, out);
	return ret;
}

std::string VmGeneratorFactory::getOutFileName(std::string baseFileName) const 
{
	return baseFileName + ".vm";
}

///////////////////////////

VmGenerator::VmGenerator(std::ostream &out, SymbolTable *rootSymbols, CallTracer *tracer)
	: m_out(out)
	, m_tracer(tracer)
	, m_curFuncType(RoutineType::None)
	, m_nextLabelToken(0)
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
	m_curFuncType = RoutineType::Ctor;
	m_thisContext = m_classContext;
	return doStartRoutine(SymbolTable::Kind::Func, funcName, SymbolTable::Type::Class, m_classContext, true);
}

bool VmGenerator::endConstructor()
{
	return doEndRoutine();
}

bool VmGenerator::startMethod(SymbolTable::Type retType, const std::string &retName, const std::string &funcName)
{
	m_curFuncType = RoutineType::Method;
	m_thisContext = m_classContext;
	return doStartRoutine(SymbolTable::Kind::Func, funcName, retType, retName, false);
}

bool VmGenerator::endMethod()
{
	return doEndRoutine();
}

bool VmGenerator::startFunction(SymbolTable::Type retType, const std::string &retName, const std::string &funcName)
{
	m_curFuncType = RoutineType::Function;
	return doStartRoutine(SymbolTable::Kind::Func, funcName, retType, retName, true);
}

bool VmGenerator::endFunction()
{
	return doEndRoutine();
}

bool VmGenerator::declareParameters(const std::vector<Parameter> &params)
{
	int off = 0;
	// methods have this as argument 0, all other offset by 1!
	if (m_curFuncType == RoutineType::Method)
		off = 1;

	for (auto p : params) {
		if (!m_symbols->add(SymbolTable::Symbol{ SymbolTable::Kind::Argument, p.type, p.classType, p.name, 0 }, off)) {
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
	int ct = m_symbols->count(SymbolTable::Kind::Var);
	m_out << " " << ct << "\n";

	switch (m_curFuncType) {
	case RoutineType::Ctor:
	{
		// Allocate this
		int size = m_symbols->toParent()->count(SymbolTable::Kind::Field);
		m_out << "push constant " << size << "\n";
		m_out << "call Memory.alloc 1\n";
		m_out << "pop pointer 0\n";
		
		break;
	}
	case RoutineType::Method:
		// Initialize this
		m_out << "push argument 0\n";
		m_out << "pop pointer 0\n";
		break;
	}

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
	token = newLabelToken();
	m_out << "label " << token << "_Start\n";
	writeExpression(cond);
	m_out << "not\n";
	m_out << "if-goto " << token << "_End\n";
	return true;
}

bool VmGenerator::endWhile(const std::string &token)
{
	m_out << "goto " << token << "_Start\n";
	m_out << "label " << token << "_End\n";
	return true;
}

bool VmGenerator::beginIf(Expression *cond, std::string &token)
{
	token = newLabelToken();
	writeExpression(cond);
	m_out << "not\n";
	m_out << "if-goto " << token << "_False\n";
	return true;
}

bool VmGenerator::insertElse(const std::string &token)
{
	m_out << "goto " << token << "_End\n";
	m_out << "label " << token << "_False\n";
	return true;
}

bool VmGenerator::noElse(const std::string &token)
{
	m_out << "label " << token << "_False\n";
	return true;
}

bool VmGenerator::endIf(const std::string &token)
{
	m_out << "label " << token << "_End\n";
	return true;
}

bool VmGenerator::writeLet(Term *lhs, Expression *rhs)
{
	if (!writeExpression(rhs))
		return false;
	if (lhs->type == Term::Variable) {
		m_out << "pop ";
		return writeVar(lhs->data.variableTerm.identifier);
	} else if (lhs->type == Term::Array) {
		if (!writeArray(lhs))
			return false;
		m_out << "pop that 0\n";
		return true;
	}

	return false;
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
	m_curFuncType = RoutineType::None;
	return true;
}

bool VmGenerator::writeCall(Term  *term)
{
	CallData data = prepareCall(term);
	bool argAdd = 0;
	if (!data.asStatic) {
		// Push target, counts as argument
		argAdd = 1;
		m_out << "push ";
		writeVar(data.obj);
	}
	for (auto e : term->data.callTerm.params) {
		if (!writeExpression(e))
			return false;
	}
	m_out << "call " << data.target << " " << term->data.callTerm.params.size()+argAdd << "\n";
	return true;
}

bool VmGenerator::writeTerm(Term *term)
{
	switch (term->type) {
	case Term::IntConst:
		m_out << "push constant " << term->data.intTerm.value << "\n";
		return true;
	case Term::StringConst:
		return createStringConst(term->data.stringTerm.value);
	case Term::KeywordConst:
		switch (term->data.keywordTerm.value) {
		case Tokenizer::Keyword::True:
			m_out << "push constant 1\n";
			return true;
		case Tokenizer::Keyword::False:
		case Tokenizer::Keyword::Null:
			m_out << "push constant 0\n";
			return true;
		case Tokenizer::Keyword::This:
			m_out << "push pointer 0\n";
			return true;
		default:
			return false;
		}
	case Term::Variable:
		m_out << "push ";
		return writeVar(term->data.variableTerm.identifier);
	case Term::Array:
		if (!writeArray(term))
			return false;
		m_out << "push that 0\n";
		return true;
	case Term::Call:
		return writeCall(term);
	case Term::Bracketed:
		return writeExpression(term->data.bracketTerm.content);
	case Term::Unary:
		if (!writeTerm(term->data.unaryTerm.content))
			return false;
		return writeUnary(term->data.unaryTerm.unaryOp);
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

bool VmGenerator::writeUnary(char op)
{
	switch (op) {
	case '-':
		m_out << "neg\n";
		return true;
	case '~':
		m_out << "not\n";
		return true;
	}
	return false;
}

bool VmGenerator::writeVar(const std::string &var)
{
	if (var == "this") {
		m_out << "pointer 0\n";
		return true;
	}
	auto it = m_symbols->get(var);
	switch (it.kind)
	{
	case SymbolTable::Kind::Static:
		m_out << "static " << it.order << "\n";
		return true;
	case SymbolTable::Kind::Argument:
		m_out << "argument " << it.order << "\n";
		return true;
	case SymbolTable::Kind::Var:
		m_out << "local " << it.order << "\n";
		return true;
	case SymbolTable::Kind::Field:
		m_out << "this " << it.order << "\n";
		return true;
	}
	return false;
}

VmGenerator::CallData VmGenerator::prepareCall(Term *term)
{
	std::string realTarget;
	bool asStatic;
	std::string obj;

	if (term->data.callTerm.target.empty()) {
		if (m_thisContext.empty()) {
			asStatic = true;
			realTarget = m_classContext;
		} else {
			asStatic = false;
			realTarget = m_thisContext; // Always same as classContext...
			obj = "this";
		}
	} else if (term->data.callTerm.target == "this") {
		asStatic = false;
		realTarget = m_thisContext; // Always same as classContext...
		obj = "this";
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
			obj = s.name;
		}
	}

	std::string fullTarget = realTarget + "." + term->data.callTerm.function;
	m_tracer->addCall(fullTarget, asStatic, {}, {}, 0);
	return CallData{ fullTarget, asStatic, obj };
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

std::string VmGenerator::newLabelToken()
{
	std::stringstream ss;
	ss << "$LABEL_";
	ss << (m_nextLabelToken++);
	return ss.str();
}

bool VmGenerator::createStringConst(const std::string &str)
{
	m_out << "push constant " << str.length() << "\n";
	m_out << "call String.new 1\n";
	// new string is on stack
	for (auto c : str) {
		m_out << "push constant " << static_cast<unsigned int>(c) << "\n";
		m_out << "call String.appendChar 2\n";
		// String.appendChar returns the string, so it is still on stack
	}
	return true;
}

bool VmGenerator::writeArray(Term *term)
{
	if (term->type != Term::Array)
		return false;
	m_out << "push ";
	if (!writeVar(term->data.arrayTerm.identifier))
		return false;
	// Write the index
	if (!writeExpression(term->data.arrayTerm.index))
		return false;
	// Get full pointer into pointer
	m_out << "add\n";
	m_out << "pop pointer 1\n";
	return true;
}
