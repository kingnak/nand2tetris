#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

class CmdLineHelper
{
public:
	CmdLineHelper() : m_flags(0), m_allowEmptyIn(false) {}

	void allowEmptyIn() { m_allowEmptyIn = true; }

	bool handleCmdLine(int argc, char **argv, bool hasOut = true);
	std::string usage() const;

	std::string inFile() const;
	std::string outFile(const std::string &suffix = {}) const;

	static std::string baseName(std::string fn);
	static std::string pathName(std::string fn);
	static std::string suffix(std::string fn);

	static std::vector<std::string> parse(std::string fn);

	static bool isDirectory(const std::string &fn);
	static bool isFile(const std::string &fn);
	static std::string absolutePath(const std::string &fn);
	static std::vector<std::string> listFilesInDir(std::string dir, const std::string &suffixFilter = {});

	bool isHelp() const { return (m_flags & Help) != 0; }
	bool isDebug() const { return (m_flags & Debug) != 0; }
	bool isVerbose() const { return (m_flags & Verbose) != 0; }

	bool isFlagSet(int arg) const { return (m_flags & arg) != 0; }

	enum {
		Help = 0x0001,
		Debug = 0x0002,
		Verbose = 0x0004,
		CustomArgBase = 0x0100
	};

	void addDefaultArgs(int args);
	void addCustomArg(const std::string &shortName, const std::string &longName, int arg);

private:
	std::string m_cmd;
	std::string m_in;
	std::string m_out;

	std::unordered_map<std::string, int> m_custArgDef;
	std::map<int, std::pair<std::string, std::string>> m_custArgFlags;

	int m_flags;
	bool m_allowEmptyIn;
};