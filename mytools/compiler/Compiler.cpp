#include "Compiler.h"
#include "Tokenizer.h"
#include "CompilerEngine.h"

using namespace std;

Compiler::Compiler(AbstractCompilerEngineFactory *factory)
	: m_factory(factory)
{}

Compiler::~Compiler()
{
	delete m_factory;
}

bool Compiler::compile(istream &in, ostream &o)
{
	Tokenizer t(in);
	CompilerEngine *engine = m_factory->createEngine(o);
	while (t.hasMoreTokens()) {
		t.advance();
		switch (t.tokenType()) {
		case Tokenizer::TokenType::Keyword:
			o << "KEYWORD " << int(t.keyword()) << "\n";
			break;
		case Tokenizer::TokenType::Symbol:
			o << "SYMBOL " << t.symbol() << "\n";
			break;
		case Tokenizer::TokenType::StringConst:
			o << "STRING " << t.stringVal() << "\n";
			break;
		case Tokenizer::TokenType::IntConst:
			o << "INT " << t.intVal() << "\n";
			break;
		case Tokenizer::TokenType::Identifier:
			o << "ID " << t.identifier() << "\n";
			break;
		case Tokenizer::TokenType::End:
			o << "END\n";
			break;
		case Tokenizer::TokenType::Error:
			o << "ERROR " << t.error() << "\n";
			break;
		case Tokenizer::TokenType::Nothing:
			o << "NOTHING\n";
			break;
		default:
			o << "???? " << int(t.tokenType()) << "\n";
			break;
		}
	}

	o.flush();
	delete engine;
	return !t.hasError();
}