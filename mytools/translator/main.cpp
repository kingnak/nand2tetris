
#include "Trans.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "CmdLineHelper.h"

using namespace  std;

int main(int argc, char **argv)
{
	enum {
		NoBootstrap = CmdLineHelper::CustomArgBase,
		MinBootstrap = CmdLineHelper::CustomArgBase<<1,
		StackOffset = CmdLineHelper::CustomArgBase<<2,
		BareOutput = CmdLineHelper::CustomArgBase<<3
	};


	CmdLineHelper c;
	c.allowEmptyIn();
	c.addDefaultArgs(CmdLineHelper::Help | CmdLineHelper::Debug);
	c.addCustomArg("-nb", "--no-bootstrap", NoBootstrap);
	c.addCustomArg("-mb", "--min-bootstrap", MinBootstrap);
	c.addCustomArg("-os", "--offset-stack", StackOffset);
	c.addCustomArg("", "--bare", BareOutput);

	if (!c.handleCmdLine(argc, argv) || c.isHelp()) {
		cerr << c.usage() << endl;
		return 1;
	}

	string inFile = c.inFile();
	if (inFile.empty()) {
		inFile = ".";
	}

	vector<string> files;
	if (CmdLineHelper::isDirectory(inFile)) {
		files = CmdLineHelper::listFilesInDir(inFile, ".vm");
	} else if (CmdLineHelper::isFile(inFile)) {
		files.push_back(inFile);
	} else {
		cerr << "Cannot open " << inFile << endl;
		return 1;
	}

	string ofile = c.outFile(".asm");

	ofstream out;
	out.open(ofile, ios_base::out | ios_base::trunc);
	if (!out.is_open()) {
		cerr << "Cannot open output file: " << argv[2] << endl;
		return 1;
	}

	Translator t(out);
	t.setDebug(c.isDebug());
	t.setBare(c.isFlagSet(BareOutput));

	if (!c.isFlagSet(NoBootstrap)) {
		if (c.isFlagSet(MinBootstrap)) {
			if (c.isFlagSet(StackOffset))
				t.minimalBootstrap(true);
			else
				t.minimalBootstrap(false);
		} else {
			t.bootstrap();
		}
	}

	for (auto fi : files) {
		ifstream in;
		in.open(fi);
		if (!in.is_open()) {
			cerr << "Cannot open input file: " << fi << endl;
			return 1;
		}

		t.translate(in, CmdLineHelper::baseName(fi));
		if (t.errors().hasFail()) {
			break;
		}
	}

	if (!t.errors().hasFail()) {
		t.teardown();
		t.verifyCalls();
	}

	for_each(t.errors().messages().cbegin(), t.errors().messages().cend(), [](auto s) { cerr << s << endl; });

	if (t.errors().hasError()) {
		out.close();
		remove(ofile.c_str());
		return 1;
	}

	return 0;
}
