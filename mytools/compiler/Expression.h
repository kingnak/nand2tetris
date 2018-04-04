#pragma once
#include <string>
#include <vector>
#include "Tokenizer.h"

struct Term;
class CompilerEngine;

class Expression {
public:
	Expression() : m_root(nullptr) {}
	~Expression();
	bool isSingleTerm() { return m_root->type == Node::Leaf; }
	Term *term() { return m_root->t; }
	char op() { return m_root->op; }
	Expression *right() { return m_root->right; }

	static Expression *compileExpression(CompilerEngine *comp);
	static Term *compileSingleTerm(CompilerEngine *comp);

	friend class CompilerEngine;

private:
	struct ExpressionCompiler {
		ExpressionCompiler(CompilerEngine *comp) : m_comp(comp) { }
		Expression *compileExpression();
		Term *compileTerm();
		std::vector<Expression *> compileParamList();
		CompilerEngine *m_comp;
	};

	struct Node {
		enum { Leaf, Op } type;
		Term *t;
		char op;
		Expression *right;
		~Node();
		Node() :type(Leaf), t(nullptr), op('\0'), right(nullptr) {}
	};

private:
	Node *m_root;
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
			StringTerm_(StringTerm_&&) = default;
			StringTerm_&operator=(StringTerm_&&) = default;
		} stringTerm;
		struct KeywordTerm_ {
			Tokenizer::Keyword value;
		} keywordTerm;
		struct VariableTerm_ {
			std::string identifier;
			~VariableTerm_() = default;
			VariableTerm_(VariableTerm_&&) = default;
			VariableTerm_&operator=(VariableTerm_&&) = default;
		} variableTerm;
		struct ArrayTerm_ {
			std::string identifier;
			Expression *index = nullptr;
			ArrayTerm_(const std::string i, Expression *e) : identifier(std::move(i)), index(e) {}
			~ArrayTerm_() { delete index; }
			ArrayTerm_(ArrayTerm_&&o) : index(o.index), identifier(std::move(o.identifier)) { o.index = nullptr; }
			ArrayTerm_&operator=(ArrayTerm_&&o) { index = o.index; o.index = nullptr; identifier = std::move(o.identifier); return *this; }
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
			Expression *content;
			BracketTerm_(Expression *e) : content(e) {}
			~BracketTerm_() { delete content; }
			BracketTerm_(BracketTerm_&&o) : content(o.content) { o.content = nullptr; }
			BracketTerm_&operator=(BracketTerm_&&o) { content = o.content; o.content= nullptr; return *this; }
		} bracketTerm;
		struct UnaryTerm_ {
			char unaryOp;
			Term *content;
			UnaryTerm_(char o, Term *e) : unaryOp(o), content(e) {}
			~UnaryTerm_() { delete content; }
			UnaryTerm_(UnaryTerm_&&o) : content(o.content), unaryOp(o.unaryOp) { o.content = nullptr; }
			UnaryTerm_&operator=(UnaryTerm_&&o) { content = o.content; unaryOp = o.unaryOp; o.content = nullptr; return *this; }
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
		t->data.unaryTerm = Content_::UnaryTerm_( op,sub );
		return t;
	}

	static Term *bracketedTerm(Expression *sub) {
		Term *t = new Term{ Term::Bracketed };
		t->data.bracketTerm = Content_::BracketTerm_( sub );
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
		t->data.arrayTerm = Content_::ArrayTerm_( var, idx );
		return t;
	}
};
