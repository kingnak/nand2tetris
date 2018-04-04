#pragma once
#include <string>
#include <vector>
#include "Tokenizer.h"

struct Term;
class CompilerEngine;

class Expression {
public:
	Expression() : m_term(nullptr) {}
	~Expression();
	Term *term() { return m_term; }

	static Expression *compileExpression(CompilerEngine *comp);

	friend class CompilerEngine;

private:
	struct ExpressionCompiler {
		ExpressionCompiler(CompilerEngine *comp) : m_comp(comp) { }
		Expression *compile();
	private:
		Expression *compileExpression();
		Term *compileTerm();
		std::vector<Expression *> compileParamList();
		CompilerEngine *m_comp;
	};

private:
	Term *m_term;
};

struct Term
{
	enum {
		IntConst, StringConst, KeywordConst, Variable, Array, Call, Bracketed, Unary
	} type;
	union Content_ {
		struct IntTerm_ {
			int value;
		} intTerm;
		struct StringTerm_ {
			std::string value;
			~StringTerm_() = default;
		} stringTerm;
		struct KeywordTerm_ {
			Tokenizer::Keyword value;
		} keywordTerm;
		struct VariableTerm_ {
			std::string identifier;
			~VariableTerm_() = default;
		} variableTerm;
		struct ArrayTerm_ {
			std::string identifier;
			Expression *expression = nullptr;
			~ArrayTerm_() { delete expression; }
		} arrayTerm;
		struct CallTerm_ {
			std::string target;
			std::string function;
			std::vector<Expression*> params;
			~CallTerm_() { for (auto e : params) delete e; }
			CallTerm_(CallTerm_&&) = default;
			CallTerm_&operator=(CallTerm_&&) = default;
		} callTerm;
		struct BracketTerm_ {
			Term *content;
			~BracketTerm_() { delete content; }
		} bracketTerm;
		struct UnaryTerm_ {
			char unaryOp;
			Term *content;
			~UnaryTerm_() { delete content; }
		} unaryTerm;
		~Content_() {}
	} data;

	~Term() {
		switch (type) {
		case StringConst:
			data.stringTerm.~StringTerm_();
			break;
		case Variable:
			data.variableTerm.~VariableTerm_();
			break;
		case Bracketed:
			data.bracketTerm.~BracketTerm_();
			break;
		case Unary:
			data.unaryTerm.~UnaryTerm_();
			break;
		case Call:
			data.callTerm.~CallTerm_();
			break;
		}
	}

	static Term *intTerm(int i) {
		Term *t = new Term{ Term::IntConst };
		t->data.intTerm = Content_::IntTerm_{ i };
		return t;
	}

	static Term *stringTerm(const std::string &s) {
		Term *t = new Term{ Term::StringConst };
		t->data.stringTerm = Content_::StringTerm_{ s };
		return t;
	}

	static Term *keywordTerm(Tokenizer::Keyword kw) {
		Term *t = new Term{ Term::KeywordConst };
		t->data.keywordTerm = Content_::KeywordTerm_{ kw };
		return t;
	}

	static Term *unaryTerm(char op, Term *sub) {
		Term *t = new Term{ Term::Unary };
		t->data.unaryTerm = Content_::UnaryTerm_{ op,sub };
		return t;
	}

	static Term *bracketedTerm(Term *sub) {
		Term *t = new Term{ Term::Bracketed };
		t->data.bracketTerm = Content_::BracketTerm_{ sub };
		return t;
	}

	static Term *variableTerm(const std::string &var) {
		Term *t = new Term{ Term::Variable };
		t->data.variableTerm = Content_::VariableTerm_{ var };
		return t;
	}

	static Term *callTerm(const std::string &target, const std::string &func, const std::vector<Expression*> &params) {
		Term *t = new Term{ Term::Call };
		t->data.callTerm = std::move(Content_::CallTerm_{ target, func, params });
		return t;
	}

	static Term *arrayTerm(const std::string &var, Expression *idx) {
		Term *t = new Term{ Term::Array };
		t->data.arrayTerm = Content_::ArrayTerm_{ var,idx };
		return t;
	}
};
