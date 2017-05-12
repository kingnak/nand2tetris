#include "Asm.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include "CmdLineHelper.h"

using namespace std;

int main(int argc, char **argv)
{
	CmdLineHelper c;
	c.addDefaultArgs(CmdLineHelper::Help);
	if (!c.handleCmdLine(argc, argv) || c.isHelp()) {
		cerr << c.usage() << endl;
		return 1;
	}

	ifstream in;
	ofstream out;
	in.open(c.inFile());
	out.open(c.outFile(".hack"), ios_base::out | ios_base::trunc);

	if (!in.is_open()) {
		cerr << "Cannot open input file: " << argv[1] << endl;
		return 1;
	}
	if (!out.is_open()) {
		cerr << "Cannot open output file: " << argv[2] << endl;
		return 1;
	}

	Assembler a(in, out);
	int ret = a.assemble() ? 0 : 1;
	for_each(a.errors().messages().cbegin(), a.errors().messages().cend(), [](auto s) { cerr << s << endl; });
	return ret;

	/*
	if (argc < 3) {
		cerr << "Usage: " << argv[0] << " <input> <output>" << endl;
		return 1;
	}

	ifstream in;
	ofstream out;
	in.open(argv[1]);
	out.open(argv[2], ios_base::out | ios_base::trunc);

	if (!in.is_open()) {
		cerr << "Cannot open input file: " << argv[1] << endl;
		return 1;
	}
	if (!out.is_open()) {
		cerr << "Cannot open output file: " << argv[2] << endl;
		return 1;
	}

	Assembler a(in, out);
	if (!a.assemble()) {
		cerr << "Error in line " << a.errorLine() << ": " << a.error() << endl;
		return 1;
	}

	return 0;
	*/
}