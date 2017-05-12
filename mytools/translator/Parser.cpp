#include "Parser.h"
#include <sstream>

using namespace std;

const std::unordered_set<std::string> Parser::s_arith = {
	"add", "sub", "neg",
	"eq", "gt", "lt",
	"and", "or", "not"
};
const std::unordered_set<std::string> Parser::s_segs = {
	"static", "constant", "temp", "pointer",
	"local", "argument", "this", "that"
};

Parser::Parser(istream &in)
	: ParserBase(in, true)
	, m_type(Trans::CommandType::Invalid)
	, m_arg2(0)
	, m_errs([](string s, int l)->string { stringstream ss; ss << s << " at line " << l; return ss.str(); })
{
}

bool Parser::parse()
{
	if (ParserBase::parse()) {
		rewind();
		return true;
	} else {
		m_errs.addError(ErrorContainer::Error, ParserBase::error(), ParserBase::errorLine());
		m_it = cend();
		return false;
	}
}

void Parser::resetState()
{
	m_type = Trans::CommandType::Invalid;
	m_arg1.clear();
	m_arg2 = 0;
}

bool Parser::hasMoreCommands() const
{
	return m_it != cend();
}

bool Parser::advance()
{
	if (m_errs.hasFail()) return false;
	++m_it;

	resetState();

	string s = m_it->first;
	string p = splitBySpace(s);

	if (s_arith.find(s) != s_arith.cend()) {
		m_arg1 = s;
		m_type = Trans::CommandType::Arith;
		return true;
	}

	if (s == "return") {
		m_type = Trans::CommandType::Return;
		return true;
	}

	if (p.empty()) {
		return setError("Parameter required", *m_it);
	}

	if (s == "label") {
		m_type = Trans::CommandType::Label;
		m_arg1 = p;
		return true;
	}

	if (s == "goto") {
		m_type = Trans::CommandType::Goto;
		m_arg1 = p;
		return true;
	}

	if (s == "if-goto") {
		m_type = Trans::CommandType::If;
		m_arg1 = p;
		return true;
	}

	// TWO PARAMETER CALLS
	string nr = splitBySpace(p);
	bool nrOk;
	int16_t v = ParserBase::parseNumber(nr, &nrOk);

	if (s == "call") {
		// second argument optional (defaults to ?)
		if (nr.empty()) {
			addWarning("Defaulting argument count to 0 for call to " + p, *m_it);
			v = 0;
		}
		else if (!nrOk || v < 0) {
			return setError("Invalid argument count", *m_it);
		}
		m_type = Trans::CommandType::Call;
		m_arg1 = p;
		m_arg2 = v;
		return true;
	}

	if (s == "function") {
		if (nr.empty()) {
			return setError("Argument count required", *m_it);
		}

		if (!nrOk || v < 0) {
			return setError("Invalid local vars count", *m_it);
		}

		m_type = Trans::CommandType::Function;
		m_arg1 = p;
		m_arg2 = v;
		return true;
	}

	if (s == "push" || s == "pop") {
		m_type = Trans::CommandType::Label;

		if (nr.empty()) {
			return setError("Offset required" , *m_it);
		}

		if (!nrOk) {
			return setError("Invalid offset", *m_it);
		}

		if (s_segs.find(p) == s_segs.cend()) {
			return setError("Invalid segment", *m_it);
		}

		if (v < 0) {
			return setError("Offset must not be negative", *m_it);
		}

		m_type = (s == "push") ? Trans::CommandType::Push : Trans::CommandType::Pop;
		m_arg1 = p;
		m_arg2 = v;
		return true;
	} else {
		return setFail("Invalid instruction", *m_it);
	}

	return false;
}

string Parser::splitBySpace(string &s)
{
	string p;
	// find parameter, if any
	auto p1 = s.find_first_of(" \t");
	auto p2 = s.find_first_not_of(" \t", p1);
	if (p2 != string::npos) {
		p = s.substr(p2);
	}
	s = s.substr(0, p1);
	return p;
}

void Parser::rewind()
{
	m_it = lines().cbegin();
}

string Parser::sourceLine() const
{
	if (m_it != lines().cend()) return m_it->first;
	return "";
}

int Parser::line() const
{
	if (m_it != lines().cend()) return m_it->second;
	return 0;
}
