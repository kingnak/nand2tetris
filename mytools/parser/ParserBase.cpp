#include "ParserBase.h"
#include <climits>

using namespace std;

int16_t ParserBase::parseNumber(const std::string &s, bool *ok /* = nullptr */)
{
	bool ok_;
	bool &rok = ok ? *ok : ok_;

	rok = false;
	char *c;
	long v = strtol(s.c_str(), &c, 10);
	if (*c != '\0') return 0;
	if (v > SHRT_MAX) return 0;
	rok = true;
	return static_cast<int16_t>(v);
}

bool ParserBase::parse()
{
	m_lines.clear();
	if (m_addSentinel) m_lines.push_back(make_pair(std::string{}, 0));

	bool inComment = false;
	int ln = 1;
	do {
		string s = readline();

		if (inComment) {
			auto p = s.find("*/");
			if (p != string::npos) {
				s = s.substr(p+2);
				inComment = false;
			}
		}

		if (!inComment) {
			auto p = s.find("//");
			if (p != string::npos)
				s = s.substr(0, p);

			removeInlineComments(s);

			p = s.find("/*");
			if (p != string::npos) {
				s = s.substr(0, p);
				inComment = true;
			}

			trim(s);

			if (!s.empty())
				m_lines.push_back(make_pair(s, ln));
		}

		ln++;
	} while (m_in.good());

	if (inComment) {
		return setError("Unexpected end of file in comment", ln);
	}
	if (m_in.bad()) {
		return setError("I/O Error", ln);
	}
	return true;
}

string ParserBase::readline()
{
	string ret;
	while (m_in.good()) {
		int c = m_in.get();
		if (c < 0) break;
		if (c == '\n') break;
		ret.push_back(static_cast<char>(c));
	}
	return ret;
}

void ParserBase::removeInlineComments(string &s)
{
	do {
		auto spos = s.find("/*");
		auto epos = s.find("*/");
		if (spos != string::npos && epos != string::npos) {
			s.erase(spos, epos+1);
		} else {
			break;
		}
	} while (true);
}

void ParserBase::trim(string &s)
{
	auto spos = s.find_first_not_of(" \t");
	if (spos != string::npos)
		s = s.substr(spos);
	else s.clear();
	auto epos = s.find_last_not_of(" \t");
	if (spos != string::npos)
		s = s.substr(0, epos+1);
}

ParserBase::LineList::const_iterator ParserBase::cend() const
{
	auto i = m_lines.cend();
	if (m_addSentinel) --i;
	return i;
}
