#pragma once

#include "ParserBase.h"
#include "TransTypes.h"
#include <unordered_set>
#include <string>
#include "ErrorContainer.h"

class Parser : private ParserBase
{
public:
	Parser(std::istream &in);

	bool parse();

	bool hasMoreCommands() const;
	bool advance();
	void rewind();

	Trans::CommandType commandType() const { return m_type; }
	std::string arg1() const { return m_arg1; }
	int16_t arg2() const { return m_arg2; }

	std::string sourceLine() const;
	int line() const;

	const ErrorContainer &errors() const { return m_errs; }
	
private:
	void resetState();
	std::string splitBySpace(std::string &s);
	bool setFail(const std::string &err, const Line &line) {
		m_errs.addError(ErrorContainer::Fail, err, line.second);
		return false;
	}
	bool setError(const std::string &err, const Line &line) {
		m_errs.addError(ErrorContainer::Error, err, line.second);
		return false;
	}
	void addWarning(const std::string &err, const Line &line) {
		m_errs.addError(ErrorContainer::Warning, err, line.second);
	}

private:
	ParserBase::LineList::const_iterator m_it;

	Trans::CommandType m_type;
	std::string m_arg1;
	int16_t m_arg2;
	ErrorContainer m_errs;

private:
	static const std::unordered_set<std::string> s_arith;
	static const std::unordered_set<std::string> s_segs;
};
