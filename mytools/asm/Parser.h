#pragma once

#include "AsmTypes.h"
#include "ParserBase.h"
#include "ErrorContainer.h"

class Parser : private ParserBase
{
public:
	Parser(std::istream &in);

	bool parse();

	bool hasMoreCommands() const;
	bool advance();
	void rewind(bool clearErrors = false);

	std::string sourceLine() const;

	Asm::CommandType commandType() const { return m_type; }
	std::string symbol() const { return m_sym; }
	std::string dest() const { return m_dest; }
	std::string comp() const { return m_comp; }
	std::string jump() const { return m_jump; }
	int line() const;

	const ErrorContainer &errors() const { return m_errs; }

private:
	void resetState();
	bool setError(const std::string &err, const Line &line) {
		m_errs.addError(ErrorContainer::Error, err, line.second);
		return false;
	}

private:
	Asm::CommandType m_type;
	std::string m_sym;
	std::string m_dest;
	std::string m_comp;
	std::string m_jump;
	ParserBase::LineList::const_iterator m_it;

	ErrorContainer m_errs;
};
