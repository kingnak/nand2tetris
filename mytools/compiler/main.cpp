#include <CmdLineHelper.h>
#include <iostream>
#include <fstream>
#include "Compiler.h"
#include "AnalyzerEngine.h"

using namespace std;

int main(int argc, char **argv)
{
	enum {
		AnalyzeOnly = CmdLineHelper::CustomArgBase
	};

	CmdLineHelper c;
	c.allowEmptyIn();
	c.addDefaultArgs(CmdLineHelper::Help | CmdLineHelper::Debug);
	c.addCustomArg("-a", "--analyze", AnalyzeOnly);
	
	if (!c.handleCmdLine(argc, argv, false) || c.isHelp()) {
		cerr << c.usage() << endl;
		return 1;
	}

	AbstractCompilerEngineFactory *factory = nullptr;
	if (c.isFlagSet(AnalyzeOnly)) {
		factory = new AnalyzerEngineFactory;
	}

	if (!factory) {
		cerr << "No generator selected" << endl;
		cerr << c.usage() << endl;
		return 1;
	}

	string inFile = c.inFile();
	if (inFile.empty()) {
		inFile = ".";
	}

	vector<string> files;
	if (CmdLineHelper::isDirectory(inFile)) {
		files = CmdLineHelper::listFilesInDir(inFile, ".jack");
	} else if (CmdLineHelper::isFile(inFile)) {
		files.push_back(inFile);
	} else {
		cerr << "Cannot open " << inFile << endl;
		return 1;
	}

	Compiler cmp(factory);
	for (auto fi : files) {
		ifstream in;
		in.open(fi);
		if (!in.is_open()) {
			cerr << "Cannot open input file: " << fi << endl;
			return 1;
		}

		ofstream out;
		string oName = factory->generateOutName(CmdLineHelper::pathName(fi) + "/" + CmdLineHelper::baseName(fi));
		out.open(oName);
		if (!out.is_open()) {
			cerr << "Cannot open output file: " << oName << endl;
			return 1;
		}

		cmp.compile(in, out);
	}

	return 0;
}