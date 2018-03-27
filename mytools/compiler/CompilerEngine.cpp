#include "CompilerEngine.h"
#include "CodeGenerator.h"
#include "Tokenizer.h"
#include "XMLHelper.hpp"
#include <map>

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
/*
bool CompilerEngine::compileClass();
bool CompilerEngine::compileClassVarDec();
bool CompilerEngine::compileSubroutine();
bool CompilerEngine::compileParameterList();
bool CompilerEngine::compileVarDec();
bool CompilerEngine::compileStatements();
bool CompilerEngine::compileDo();
bool CompilerEngine::compileLet();
bool CompilerEngine::compileWhile();
bool CompilerEngine::compileIf();
bool CompilerEngine::compileExpression();
bool CompilerEngine::compileTerm();
bool CompilerEngine::compileExpressionList();
*/