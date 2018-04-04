#include "CompilerEngine.h"
#include "CodeGenerator.h"
#include "Tokenizer.h"
#include "XMLHelper.hpp"
#include "Expression.h"
#include <map>
#include <vector>
#include <memory>

using namespace std;

CompilerEngine::CompilerEngine(CodeGeneratorFactory *factory)
	: m_fac(factory)
	, m_gen(nullptr)
	, m_tok(nullptr)
{
}

CompilerEngine::~CompilerEngine()
{
	delete m_fac;
	delete m_gen;
	delete m_tok;
}

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

static map<Tokenizer::TokenType, string> s_tokens{
	{ Tokenizer::TokenType::Keyword, "Keyword" },
	{ Tokenizer::TokenType::Symbol, "Symbol" },
	{ Tokenizer::TokenType::Identifier, "Identifier" },
	{ Tokenizer::TokenType::IntConst, "IntConst" },
	{ Tokenizer::TokenType::StringConst, "StringConst" },
	{ Tokenizer::TokenType::End, "End" }
};

bool CompilerEngine::tokenize(std::istream &in, std::ostream &out)
{
	delete m_gen;
	delete m_tok;
	m_tok = new Tokenizer(in);
	m_gen = nullptr;

	out << "<tokens>\n";
	while (m_tok->hasMoreTokens()) {
		m_tok->advance();
		switch (m_tok->tokenType()) {
		case Tokenizer::TokenType::Keyword:
			out << "<keyword> " << s_kw.find(m_tok->keyword())->second << " </keyword>\n";
			break;
		case Tokenizer::TokenType::Symbol:
			out << "<symbol> " << xmlClean(m_tok->symbol()) << " </symbol>\n";
			break;
		case Tokenizer::TokenType::StringConst:
			out << "<stringConstant> " << xmlClean(m_tok->stringVal()) << " </stringConstant>\n";
			break;
		case Tokenizer::TokenType::IntConst:
			out << "<integerConstant> " << m_tok->intVal() << " </integerConstant>\n";
			break;
		case Tokenizer::TokenType::Identifier:
			out << "<identifier> " << m_tok->identifier() << " </identifier>\n";
			break;
		case Tokenizer::TokenType::End:
			out << "</tokens>\n";
			break;
		case Tokenizer::TokenType::Error:
			out << "ERROR " << m_tok->error() << "\n";
			break;
		case Tokenizer::TokenType::Nothing:
			out << "NOTHING\n";
			break;
		default:
			out << "???? " << int(m_tok->tokenType()) << "\n";
			break;
		}
	}

	out.flush();
	return !m_tok->hasError();
}

bool CompilerEngine::compile(istream &in, ostream &out)
{
	m_error.clear();
	delete m_tok;
	delete m_gen;
	m_tok = new Tokenizer(in);
	m_gen = m_fac->create(out);

	return compileClass();
}


bool CompilerEngine::compileClass()
{
	if (!consume()) {
		return false;
	}

	if (!expect(Tokenizer::Keyword::Class)) {
		return false;
	}

	if (!expect(Tokenizer::TokenType::Identifier)) {
		return false;
	}

	string className = m_tok->identifier();
	consume();

	if (!expect('{')) {
		return false;
	}

	m_gen->startClass(className);
	while (compileClassVarDec())
		;
	while (compileSubroutine())
		;

	m_gen->endClass();
	return expect(Tokenizer::TokenType::End);
}

bool CompilerEngine::compileClassVarDec()
{
	bool isStatic = accept(Tokenizer::Keyword::Static);
	if (!isStatic) {
		if (!accept(Tokenizer::Keyword::Field)) {
			// Not static nor field => no more class var decs
			return false;
		}
	}

	VarDef def;
	if (!parseVarDef(def)) {
		return false;
	}

	if (isStatic)
		m_gen->declareStaticVariables(def.type, def.className, def.vars);
	else
		m_gen->declareFieldVariables(def.type, def.className, def.vars);

	return true;
}

bool CompilerEngine::compileSubroutine()
{
	bool isCtr = false, isMeth = false, isFunc = false;
	isCtr = accept(Tokenizer::Keyword::Constructor);
	isMeth = accept(Tokenizer::Keyword::Method);
	isFunc = accept(Tokenizer::Keyword::Function);
	if (!isCtr && !isMeth && !isFunc) {
		return false;
	}

	string clsName;
	CodeGenerator::DataType dt = CodeGenerator::DataType::None;
	if (accept(Tokenizer::TokenType::Identifier)) {
		clsName = m_tok->identifier();
		dt = CodeGenerator::DataType::Class;
		consume();
	} else {
		dt = parseType();
		if (dt == CodeGenerator::DataType::None) {
			return false;
		}
	}
	
	if (!expect(Tokenizer::TokenType::Identifier)) {
		return false;
	}

	string funcName = m_tok->identifier();
	consume();

	if (isCtr)
		m_gen->startConstructor(clsName, funcName);
	else if (isFunc)
		m_gen->startFunction(dt, clsName, funcName);
	else
		m_gen->startMethod(dt, clsName, funcName);

	if (!compileParameterList())
		return false;
	
	if (!expect('{'))
		return false;

	m_gen->startSubroutineBody();
	while (compileVarDec())
		;

	if (!compileStatements())
		return false;

	if (isCtr)
		m_gen->endConstructor();
	else if (isFunc)
		m_gen->endFunction();
	else
		m_gen->endMethod();

	return true;
}

bool CompilerEngine::compileParameterList()
{
	if (!expect('(')) {
		return false;
	}

	vector<CodeGenerator::Parameter> params;

	if (!accept(')')) {
		do {
			CodeGenerator::Parameter p;
			if (accept(Tokenizer::TokenType::Identifier)) {
				p.classType = m_tok->identifier();
				p.type = CodeGenerator::DataType::Class;
				consume();
			} else {
				p.type = parseType();
				if (p.type == CodeGenerator::DataType::None || p.type == CodeGenerator::DataType::Void) {
					return setError("Expected Type");
				}
			}

			if (!expect(Tokenizer::TokenType::Identifier)) {
				return false;
			}
			p.name = m_tok->identifier();
			consume();
			params.push_back(p);
		} while (accept(','));

		if (!expect(')'))
			return false;
	}

	m_gen->declareParameters(params);

	return true;
}

bool CompilerEngine::compileVarDec()
{
	if (!accept(Tokenizer::Keyword::Var))
		return false;

	VarDef def;
	if (!parseVarDef(def)) {
		return false;
	}

	m_gen->declareVariable(def.type, def.className, def.vars);
	return true;
}


bool CompilerEngine::compileStatements()
{
	m_gen->startStatements();
	do {
		if (accept(Tokenizer::Keyword::Let)) {
			if (!compileLet()) return false;
		} else if (accept(Tokenizer::Keyword::Do)) {
			if (!compileDo()) return false;
		} else if (accept(Tokenizer::Keyword::While)) {
			if (!compileWhile()) return false;
		} else if (accept(Tokenizer::Keyword::If)) {
			if (!compileIf()) return false;
		} else if (accept(Tokenizer::Keyword::Return)) {
			if (!compileReturn()) return false;
		} else {
			break;
		}
	} while (true);

	m_gen->endStatements();
	return expect('}');
}

bool CompilerEngine::compileDo()
{
	auto call = std::unique_ptr<Expression>(Expression::compileExpression(this));
	if (call->term()->type != Term::Call) {
		return setError("Do requires function call");
	}
	m_gen->writeDo(call->term());
	return expect(';');
}

bool CompilerEngine::compileLet()
{
	auto lhs = std::unique_ptr<Expression>(Expression::compileExpression(this));
	if (lhs->term()->type != Term::Variable && lhs->term()->type != Term::Array)
		return setError("LHS of let must be variable or array");
	
	if (!expect('='))
		return false;

	auto rhs = std::unique_ptr<Expression>(Expression::compileExpression(this));
	if (!expect(';'))
		return false;

	m_gen->writeLet(lhs->term(), rhs.get());

	return true;
}

bool CompilerEngine::compileWhile()
{
	if (!expect('('))
		return false;
	
	auto cond = std::unique_ptr<Expression> (Expression::compileExpression(this));

	if (!expect(')'))
		return false;
	
	if (!expect('{'))
		return false;

	std::string token;
	m_gen->beginWhile(cond.get(), token);

	if (!compileStatements())
		return false;

	m_gen->endWhile(token);
	return true;
}

bool CompilerEngine::compileReturn()
{
	if (accept(';')) {
		return m_gen->writeReturn(nullptr);
	}

	auto ret = std::unique_ptr<Expression>(Expression::compileExpression(this));
	m_gen->writeReturn(ret.get());
	return expect(';');
}

bool CompilerEngine::compileIf()
{
	if (!expect('('))
		return false;

	auto cond = std::unique_ptr<Expression> (Expression::compileExpression(this));

	if (!expect(')'))
		return false;

	if (!expect('{'))
		return false;

	std::string token;
	m_gen->beginIf(cond.get(), token);

	if (!compileStatements())
		return false;

	if (accept(Tokenizer::Keyword::Else)) {
		if (!expect('{'))
			return false;
		m_gen->insertElse(token);
		if (!compileStatements())
			return false;
	}

	m_gen->endIf(token);
	return true;
}

bool CompilerEngine::parseVarDef(VarDef &def)
{
	string classType;
	if (accept(Tokenizer::TokenType::Identifier)) {
		def.className = m_tok->identifier();
		def.type = CodeGenerator::DataType::Class;
		consume();
	} else {
		def.type = parseType();
		if (def.type == CodeGenerator::DataType::None || def.type == CodeGenerator::DataType::Void)
			return setError("Expected type");
	}

	if (!expect(Tokenizer::TokenType::Identifier)) {
		return false;
	}

	def.vars.clear();
	def.vars.push_back(m_tok->identifier());
	consume();
	while (accept(',')) {
		if (!expect(Tokenizer::TokenType::Identifier)) {
			return false;
		}
		def.vars.push_back(m_tok->identifier());
		consume();
	}

	return expect(';');
}

CodeGenerator::DataType CompilerEngine::parseType()
{
	if (accept(Tokenizer::Keyword::Void)) return CodeGenerator::DataType::Void;
	if (accept(Tokenizer::Keyword::Int)) return CodeGenerator::DataType::Int;
	if (accept(Tokenizer::Keyword::Char)) return CodeGenerator::DataType::Char;
	if (accept(Tokenizer::Keyword::Boolean)) return CodeGenerator::DataType::Boolean;
	return CodeGenerator::DataType::None;
}

bool CompilerEngine::accept(Tokenizer::TokenType token)
{
	if (hasError()) return false;
	return m_tok->tokenType() == token;
}

bool CompilerEngine::expect(Tokenizer::TokenType token)
{
	if (hasError()) return false;
	if (!accept(token)) {
		return setError("Expected " + s_tokens.find(token)->second);
	}
	return true;
}

bool CompilerEngine::accept(char symbol)
{
	if (accept(Tokenizer::TokenType::Symbol) && m_tok->symbol() == symbol) {
		consume();
		return true;
	}
	return false;
}

bool CompilerEngine::expect(char symbol)
{
	if (hasError()) return false;
	if (!accept(symbol)) {
		return setError("Expected Symbol \"" + string(1, symbol) + "\"");
	}
	return true;
}

bool CompilerEngine::accept(Tokenizer::Keyword kw)
{
	if (accept(Tokenizer::TokenType::Keyword) && m_tok->keyword() == kw) {
		consume();
		return true;
	}
	return false;
}

bool CompilerEngine::expect(Tokenizer::Keyword kw)
{
	if (hasError()) return false;
	if (!accept(kw)) {
		return setError("Expected " + s_kw.find(kw)->second);
	}
	return true;
}

bool CompilerEngine::consume()
{
	if (!m_tok->hasMoreTokens()) {
		return setError("Premature end of stream");
	}
	m_tok->advance();
	if (m_tok->hasError()) {
		return setError(m_tok->error());
	}
	return true;
}

bool CompilerEngine::setError(std::string error)
{
	if (!hasError()) m_error = error;
	return false;
}

bool CompilerEngine::hasError() const {
	return m_error.length() > 0;
}

string CompilerEngine::error() const {
	return m_error;
}
