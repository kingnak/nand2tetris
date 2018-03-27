#include "Tokenizer.h"
#include "ParserBase.h"
#include <sstream>

using namespace std;

const map<string, Tokenizer::Keyword> Tokenizer::s_keywords{
	{ "class", Tokenizer::Keyword::Class },
	{ "method", Tokenizer::Keyword::Method },
	{ "function", Tokenizer::Keyword::Function },
	{ "constructor", Tokenizer::Keyword::Constructor },
	{ "int", Tokenizer::Keyword::Int },
	{ "boolean", Tokenizer::Keyword::Boolean },
	{ "char", Tokenizer::Keyword::Char },
	{ "void", Tokenizer::Keyword::Void },
	{ "var", Tokenizer::Keyword::Var },
	{ "static", Tokenizer::Keyword::Static },
	{ "field", Tokenizer::Keyword::Field },
	{ "let", Tokenizer::Keyword::Let },
	{ "do", Tokenizer::Keyword::Do },
	{ "while", Tokenizer::Keyword::While },
	{ "if", Tokenizer::Keyword::If },
	{ "else", Tokenizer::Keyword::Else },
	{ "return", Tokenizer::Keyword::Return },
	{ "true", Tokenizer::Keyword::True },
	{ "false", Tokenizer::Keyword::False },
	{ "null", Tokenizer::Keyword::Null },
	{ "this", Tokenizer::Keyword::This }
};

Tokenizer::Tokenizer(istream &in)
	: m_in(in)
	, m_token(TokenType::Nothing)
	, m_keyword(Keyword::None)
	, m_sym('\0')
	, m_int(0)
	, m_pushBack('\0')
	, m_line(1)
	, m_pos(0)
{
}

bool Tokenizer::hasMoreTokens() const
{
	return !hasError() && m_token != TokenType::End;
}

void Tokenizer::advance()
{
	if (m_token == TokenType::End) {
		setError("Parse beyond end of stream");
		return;
	}

	if (m_token == TokenType::Error) {
		return;
	}

	do {
		char c = readChar();
		if (isspace(c)) continue;
		if (c == '\0') {
			m_token = TokenType::End;
			return;
		}
		
		switch (c) {
		case '+':
		case '-':
		case '*':
		case '~':
		case '&':
		case '|':
		case '=':
		case '.':
		case ',':
		case '{':
		case '}':
		case '(':
		case ')':
		case '[':
		case ']':
		case '<':
		case '>':
		case ';':
			m_sym = c;
			m_token = TokenType::Symbol;
			return;
		case '/':
			c = readChar();
			if (c == '/') {
				skipComment();
				break;
			} else if (c == '*') {
				skipBlockComment();
				break;
			} else {
				m_sym = '/';
				m_token = TokenType::Symbol;
				pushBackChar(c);
				return;
			}

		case '"':
			parseString();
			return;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			pushBackChar(c);
			parseNumber();
			return;

		default:
			if (isalpha(c) || c == '_') {
				pushBackChar(c);
				parseIdentifier();

				auto it = s_keywords.find(m_str);
				if (it != s_keywords.end()) {
					m_token = TokenType::Keyword;
					m_keyword = it->second;
					return;
				}

				m_token = TokenType::Identifier;
				return;
			}
			setError(string("Unknown character: ") + c);
			return;
		}
	} while (true);
}

char Tokenizer::readChar()
{
	m_pos++;
	if (m_pushBack != '\0') {
		char c = m_pushBack;
		m_pushBack = '\0';
		return c;
	}

	int c = m_in.get();
	if (c < 0) {
		if (m_in.bad()) setError("I/O Error");
		return '\0';
	}
	if (c == '\n') {
		m_line++;
		m_pos = 0;
	}
	return char(c);
}

void Tokenizer::pushBackChar(char c)
{
	if (m_pushBack != '\0') {
		setError("Internal Error: push back");
		return;
	}
	m_pushBack = c;
	m_pos--;
}

void Tokenizer::skipComment()
{
	while (readChar() != '\n');
}

void Tokenizer::skipBlockComment()
{
	char c1 = readChar();
	char c2 = readChar();
	while (!(c1 == '*' && c2 == '/') && !hasError()) {
		c1 = c2;
		c2 = readChar();
	}
}

void Tokenizer::parseString()
{
	m_token = TokenType::StringConst;
	m_str.clear();
	char c = readChar();
	while (c != '"' && c != '\0' && c != '\n') {
		m_str.push_back(c);
		c = readChar();
	}
	if (c != '"') {
		setError("End of line in string constant");
	}
}

void Tokenizer::parseNumber()
{
	m_token = TokenType::IntConst;
	m_str.clear();
	char c = readChar();
	while (isdigit(c)) {
		m_str.push_back(c);
		c = readChar();
	}
	pushBackChar(c);
	bool ok;
	m_int = ParserBase::parseNumber(m_str, &ok);
	if (!ok) {
		setError("Invalid number");
	}
}

void Tokenizer::parseIdentifier()
{
	m_str.clear();
	char c = readChar();

	while (isalnum(c) || c == '_') {
		m_str.push_back(c);
		c = readChar();
	}

	pushBackChar(c);
}

void Tokenizer::setError(const string &err)
{
	m_token = TokenType::Error;
	if (!hasError())
		m_err = err;
}

string Tokenizer::error() const
{
	if (hasError()) {
		stringstream ss;
		ss << "Line " << m_line << ", col " << m_pos << ": " << m_err;
		return ss.str();
	}
	return string();
}

bool Tokenizer::hasError() const
{
	return m_err.length() > 0;
}