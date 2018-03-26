#pragma once

#include <istream>
#include <string>
#include <map>

class Tokenizer
{
public:
	enum class TokenType : char {
		Nothing, Keyword, Symbol, Identifier, IntConst, StringConst, End, Error
	};

	enum class Keyword : char {
		None,
		Class, Method, Function, Constructor,
		Int, Boolean, Char, Void, 
		Var, Static, Field,
		Let, Do, While, If, Else, Return,
		True, False, Null, This
	};

	Tokenizer(std::istream &in);

	bool hasMoreTokens() const;
	void advance();
	TokenType tokenType() const { return m_token; }
	Keyword keyword() const { return m_keyword; }
	char symbol() const { return m_sym; }
	std::string identifier() const { return m_str; }
	int intVal() const { return m_int; }
	std::string stringVal() const { return m_str; }

	std::string error() const;
	bool hasError() const;

private:
	char readChar();
	void pushBackChar(char c);
	void skipComment();
	void skipBlockComment();
	void parseString();
	void parseNumber();
	void parseIdentifier();
	void setError(const std::string &err);

	static const std::map<std::string, Keyword> s_keywords;

private:
	std::istream &m_in;
	TokenType m_token;
	Keyword m_keyword;
	char m_sym;
	std::string m_str;
	int m_int;
	char m_pushBack;
	std::string m_err;
	int m_line;
	int m_pos;
};
