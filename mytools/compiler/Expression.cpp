#include "Expression.h"
#include "CompilerEngine.h"

Expression *Expression::compileExpression(CompilerEngine *comp)
{
	return ExpressionCompiler(comp).compile();
}

Expression::~Expression()
{
	delete m_term; 
}

Expression *Expression::ExpressionCompiler::compile()
{
	return compileExpression();
}

Expression *Expression::ExpressionCompiler::compileExpression()
{
	Expression *ret = new Expression;
	ret->m_term = compileTerm();
	return ret;
}

Term *Expression::ExpressionCompiler::compileTerm()
{
	if (m_comp->accept(Tokenizer::TokenType::IntConst)) {
		Term *t = Term::intTerm(m_comp->m_tok->intVal());
		m_comp->consume();
		return t;
	}

	if (m_comp->accept(Tokenizer::TokenType::StringConst)) {
		Term *t = Term::stringTerm(m_comp->m_tok->stringVal());
		m_comp->consume();
		return t;
	}

	if (m_comp->accept(Tokenizer::TokenType::Keyword)) {
		Term *t = nullptr;
		switch (m_comp->m_tok->keyword()) {
		case Tokenizer::Keyword::False:
		case Tokenizer::Keyword::True:
		case Tokenizer::Keyword::Null:
			t = Term::keywordTerm(m_comp->m_tok->keyword());
			m_comp->consume();
			return t;
		case Tokenizer::Keyword::This:
			m_comp->consume();
			if (m_comp->accept('.')) {
				// Subroutine call on this
				if (!m_comp->expect(Tokenizer::TokenType::Identifier)) {
					return nullptr;
				}
				std::string func = m_comp->m_tok->identifier();
				if (!m_comp->expect('(')) {
					return nullptr;
				}
				return Term::callTerm("this", func, compileParamList());
			}
			return Term::keywordTerm(Tokenizer::Keyword::This);
		default:
			m_comp->setError("Unexpected keyword");
			return nullptr;
		}
	}

	if (m_comp->accept('-')) {
		return Term::unaryTerm('-', compileTerm());
	}

	if (m_comp->accept('~')) {
		return Term::unaryTerm('~', compileTerm());
	}

	if (m_comp->accept('(')) {
		Term *t = Term::bracketedTerm(compileTerm());
		if (!m_comp->expect(')')) {
			delete t;
			return nullptr;
		}
		return t;
	}

	if (m_comp->accept(Tokenizer::TokenType::Identifier)) {
		std::string v = m_comp->m_tok->identifier();
		m_comp->consume();

		if (m_comp->accept('.')) {
			if (!m_comp->expect(Tokenizer::TokenType::Identifier)) {
				return false;
			}
			std::string func = m_comp->m_tok->identifier();
			m_comp->consume();
			if (!m_comp->expect('(')) {
				return nullptr;
			}
			return Term::callTerm(v, func, compileParamList());
		}

		if (m_comp->accept('(')) {
			return Term::callTerm("", v, compileParamList());
		}

		if (m_comp->accept('[')) {
			Term *t = Term::arrayTerm(v, compileExpression());
			if (!m_comp->expect(']')) {
				delete t;
				return nullptr;
			}
			return t;
		}

		return Term::variableTerm(v);
	}

	return nullptr;
}

std::vector<Expression *> Expression::ExpressionCompiler::compileParamList()
{
	std::vector<Expression*> ret;
	if (!m_comp->accept(')')) {
		do {
			ret.push_back(compileExpression());
		} while (m_comp->accept(','));
		m_comp->expect(')');
	}
	return ret;
}
