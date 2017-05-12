#pragma once

#include <istream>
#include <utility>
#include <string>
#include <list>

class ParserBase
{
public:
	static int16_t parseNumber(const std::string &s, bool *ok = nullptr);

protected:
	ParserBase(std::istream &i, bool usePreSentinel = false) : m_in(i), m_addSentinel(usePreSentinel), m_errLine(-1) {}

	bool parse();

	typedef std::pair<std::string, int> Line;
	typedef std::list<Line> LineList;
	const LineList &lines() const { return m_lines; }
	std::string error() const { return m_err; }
	int errorLine() const { return m_errLine; }
	bool hasError() const { return m_errLine >= 0; }

protected:
	static void trim(std::string &s);
	LineList::const_iterator cend() const;
	void clearError() {
		setError(std::string{}, -1);
	}
	/*
	bool setError(const std::string &err, const Line &line) {
		return setError(err, line.second);
	}*/

private:
	static void removeInlineComments(std::string &s);
	bool setError(const std::string &err, int line) {
		m_err = err;
		m_errLine = line;
		return false;
	}
	std::string readline();

private:
	std::istream &m_in;
	LineList m_lines;
	bool m_addSentinel;
	std::string m_err;
	int m_errLine;
};