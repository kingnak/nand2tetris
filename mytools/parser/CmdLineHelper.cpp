#include "CmdLineHelper.h"
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;

void CmdLineHelper::addDefaultArgs(int args)
{
	if (args & Help) {
		addCustomArg("-?", "--help", Help);
	}
	if (args & Debug) {
		addCustomArg("-d", "--debug", Debug);
	}
	if (args & Verbose) {
		addCustomArg("-v", "--verbose", Verbose);
	}
}

void CmdLineHelper::addCustomArg(const std::string &shortName, const std::string &longName, int arg)
{
	if (m_custArgFlags.find(arg) != m_custArgFlags.end()) {
		abort();
	}
	m_custArgFlags.insert(make_pair(arg, make_pair(shortName, longName)));
	if (!shortName.empty()) {
		m_custArgDef.insert(make_pair(shortName, arg));
	}
	if (!longName.empty()) {
		m_custArgDef.insert(make_pair(longName, arg));
	}
}

bool CmdLineHelper::handleCmdLine(int argc, char **argv, bool hasOut)
{
	if (argc < 1) return false;

	m_cmd = baseName(argv[0]);

	vector<string> args(argc-1);
	for (int i = 1; i < argc; ++i) {
		args[i-1] = argv[i];
	}

	// parse flags
	for (auto it = begin(args); it != end(args); ) {
		if (it->at(0) == '-') {
			/*
			if (*it == "-?" || *it == "--help") {
				m_flags |= Help;
			} else if (*it == "-d" || *it == "--debug") {
				m_flags |= Debug;
			} else if (*it == "-v" || *it == "--verbose") {
				m_flags |= Verbose;
			} else {
				cerr << "Unsupported flag: " << *it << endl;
			}
			*/
			auto flag = m_custArgDef.find(*it);
			if (flag != m_custArgDef.end()) {
				m_flags |= flag->second;
			} else {
				cerr << "Unsupported flag: " << *it << endl;
				return false;
			}

			it = args.erase(it);
		} else {
			++it;
		}
	}

	if (args.empty()) {
		if (isHelp()) return true;
		if (!m_allowEmptyIn) return false;
		m_in = absolutePath(".");
	} else {
		m_in = args[0];
	}

	if (m_in.back() == '/' || m_in.back() == '\\') m_in.pop_back();

	uint16_t off = 1;
	if (hasOut) {
		if (args.size() > 1) {
			m_out = args[1];
			off = 2;
		}
	}
	if (args.size() > off) {
		cerr << "Unsupported flags:";
		for (auto f = off; f < args.size(); ++f) cerr << ' ' << args[f];
		cerr << endl;
		return false;
	}
	return true;
}

string CmdLineHelper::usage() const
{
	stringstream s;
	s << "Usage: " << m_cmd << " <input> <output>";

	for (auto it : m_custArgFlags) {
		s << " [";
		if (!it.second.first.empty()) {
			s << it.second.first;
			if (!it.second.second.empty())
				s << " | ";
		}
		s << it.second.second << ']';
	}
	return s.str();
}

string CmdLineHelper::inFile() const
{
	return m_in;
}

string CmdLineHelper::outFile(const string &suffix /* = {} */) const
{
	if (!m_out.empty()) return m_out;

	auto p = parse(m_in);
	if (isDirectory(m_in)) {
		p[0] += p[1] + "/";
	}
	return p[0] + p[1] + suffix;
}

string CmdLineHelper::baseName(string fn)
{
	return parse(fn)[1];
}

string CmdLineHelper::pathName(string fn)
{
	return parse(fn)[0];
}

string CmdLineHelper::suffix(string fn)
{
	return parse(fn)[2];
}

vector<string> CmdLineHelper::parse(string fn)
{
	vector<string> ret(3);

	auto p = fn.find_last_of("/\\");
	if (p != string::npos) {
		ret[0] = fn.substr(0, p + 1);
		fn = fn.substr(p + 1);
	}
	p = fn.find('.');
	if (p != string::npos)
		ret[2] = fn.substr(p);
	ret[1] = fn.substr(0, p);
	return ret;
}

#include <sys/stat.h>
#include <dirent.h>

bool CmdLineHelper::isDirectory(const string &fn)
{
	struct stat s;
	if (!stat(fn.c_str(), &s)) {
		return (s.st_mode & S_IFDIR);
	}
	return false;
}

bool CmdLineHelper::isFile(const string &fn)
{
	struct stat s;
	if (!stat(fn.c_str(), &s)) {
		return (s.st_mode & S_IFREG);
	}
	return false;
}

#include <windows.h>
string CmdLineHelper::absolutePath(const string &fn)
{
#define BUFSIZE 4096
	char outpath[BUFSIZE + 1];
	char *namePtr = 0;
	GetFullPathNameA(fn.c_str(), BUFSIZE, outpath, &namePtr);
	return string{ outpath };
#undef BUFSIZE
}

vector<string> CmdLineHelper::listFilesInDir(string dir, const std::string &suffixFilter)
{
	vector<string> ret;
	if (!isDirectory(dir)) return ret;

	DIR *d = opendir(dir.c_str());
	if (!d) return ret;

	if (dir.back() != '\\' && dir.back() != '/') {
		dir += '/';
	}
	while (dirent *e = readdir(d)) {
		string curName(e->d_name);
		if (curName == "." || curName == "..") continue;
		string lcurName = curName;
		transform(lcurName.begin(), lcurName.end(), lcurName.begin(), tolower);
		if (!suffixFilter.empty()) {
			auto p = lcurName.rfind(suffixFilter);
			if (p == string::npos) continue;
			if (p != lcurName.length() - suffixFilter.length()) continue;
		}

		string curPath = dir + curName;
		if (isFile(curPath)) {
			ret.push_back(curPath);
		}
	}

	closedir(d);
	return ret;
}
