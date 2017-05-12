#include "Parser.h"
#include <istream>
#include <regex>

using namespace std;

Parser::Parser(istream &in)
	: ParserBase(in, true),
	m_type(Asm::CommandType::INVALID)
{
}

bool Parser::parse()
{
	resetState();
	if (ParserBase::parse()) {
		rewind();
		return true;
	} else {
		m_it = cend();
		return false;
	}
}

bool Parser::hasMoreCommands() const
{
	return m_it != cend();
}

int Parser::line() const
{
	if (m_it != lines().cend()) {
		return m_it->second;
	}
	return 0;
}

std::string Parser::sourceLine() const
{
	if (m_it != lines().cend()) {
		return m_it->first;
	}
	return "";
}

void Parser::rewind(bool clearErrors)
{
	if (clearErrors) {
		ParserBase::clearError();
		m_errs.clear();
	}
	m_it = lines().cbegin();
}

void Parser::resetState()
{
	m_type = Asm::CommandType::INVALID;
	m_sym.clear();
	m_dest.clear();
	m_comp.clear();
	m_jump.clear();
}

bool Parser::advance()
{
	if (hasError()) return false;
	++m_it;
	string s = m_it->first;

	resetState();
	
	if (s.front() == '(') {
		if (s.back() != ')') return setError("Label end error", *m_it);
		m_type = Asm::CommandType::L_COMMAND;
		s.pop_back();
		s = s.substr(1);
		m_sym = s;
		return true;
	}

	if (s.front() == '@') {
		m_type = Asm::CommandType::A_COMMAND;
		m_sym = s.substr(1);
		if (m_sym.empty()) return setError("Empty symbol", *m_it);
		trim(m_sym);
		return true;
	}

	m_type = Asm::CommandType::C_COMMAND;
	auto p = s.find('=');
	if (p != string::npos) {
		m_dest = s.substr(0, p);
		s = s.substr(p+1);
		if (m_dest.empty()) return setError("No destination", *m_it);
	}
	trim(m_dest);

	p = s.find(';');
	if (p != string::npos) {
		m_jump = s.substr(p+1);
		s = s.substr(0, p);
		if (m_jump.empty()) return setError("No jump", *m_it);
	}
	trim(m_jump);

	trim(s);
	m_comp = s;

	if (m_comp.empty()) return setError("No computation", *m_it);

	return true;
}
