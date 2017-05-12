#include "Asm.h"
#include "Code.h"
#include <sstream>
#include <climits>
#include <iostream>

using namespace std;

Assembler::Assembler(istream &in, ostream &o)
	: m_parser(in), m_out(o)
{

}

void Assembler::initSymbols()
{
	m_sym.clear();
	m_sym.addVariableEntry("R0");
	m_sym.addVariableEntry("R1");
	m_sym.addVariableEntry("R2");
	m_sym.addVariableEntry("R3");
	m_sym.addVariableEntry("R4");
	m_sym.addVariableEntry("R5");
	m_sym.addVariableEntry("R6");
	m_sym.addVariableEntry("R7");
	m_sym.addVariableEntry("R8");
	m_sym.addVariableEntry("R9");
	m_sym.addVariableEntry("R10");
	m_sym.addVariableEntry("R11");
	m_sym.addVariableEntry("R12");
	m_sym.addVariableEntry("R13");
	m_sym.addVariableEntry("R14");
	m_sym.addVariableEntry("R15");
	if (m_sym.getAddress("R15") != 15) {
		cerr << "ERROR INITIALIZING SYMBOL TABLE" << endl;
		exit(-1);
	}
	m_sym.addFixedEntry("SP", 0);
	m_sym.addFixedEntry("LCL", 1);
	m_sym.addFixedEntry("ARG", 2);
	m_sym.addFixedEntry("THIS", 3);
	m_sym.addFixedEntry("THAT", 4);
	m_sym.addFixedEntry("SCREEN", 16384);
	m_sym.addFixedEntry("KBD", 24576);
}

bool Assembler::assemble()
{
	initSymbols();

	if (!m_parser.parse()) {
		m_errs.unifyWith(m_parser.errors());
		return false;
	}
	if (!firstPass()) {
		m_errs.unifyWith(m_parser.errors());
		return false;
	}
	m_parser.rewind();
	if (!secondPass()) {
		m_errs.unifyWith(m_parser.errors());
		return false;
	}

	return true;
}

bool Assembler::firstPass()
{
	uint32_t addr = 0;
	while (m_parser.hasMoreCommands()) {
		if (!m_parser.advance()) return false;

		Code::Instruction instr = 0;

		if (m_parser.commandType() == Asm::CommandType::L_COMMAND) {
			const string &s = m_parser.symbol();
			if (s.empty()) return setError("Empty label");
			if (m_sym.contains(s)) return setError("Duplicate or reserved label " + s);
			if (isdigit(s[0])) return setError("Label must not start with digit");
			m_sym.addFixedEntry(s, addr);
		} else {
			addr++;
		}
	}

	if (addr-1 > INT16_MAX) {
		m_errs.addError(ErrorContainer::Fail, "File too large");
		return false;
	}

	return true;
}

bool Assembler::secondPass()
{
	while (m_parser.hasMoreCommands()) {
		if (!m_parser.advance()) return false;

		Code::Instruction instr = 0;

		switch (m_parser.commandType()) {
		case Asm::CommandType::L_COMMAND:
			continue;
			break;
		case Asm::CommandType::A_COMMAND:
			if (!handleA(instr)) return false;
			break;
		case Asm::CommandType::C_COMMAND:
			if (!handleC(instr)) return false;
			break;
		default:
			return setError("Unrecognized command type");
		}

		output(instr);
	}

	return !m_parser.errors().hasError();
}

bool Assembler::handleA(Code::Instruction &i)
{
	i = Code::aBase();
	int16_t addr = -1;

	const string &s = m_parser.symbol();
	if (s.empty()) return setError("Empty load");
	if (m_sym.contains(s)) {
		addr = m_sym.getAddress(s);
	} else if (isdigit(s[0])) {
		bool ok;
		addr = ParserBase::parseNumber(s, &ok);
		if (!ok) return setError("Invalid number");
		if (addr < 0) return setError("Value must not be negative");
	} else {
		addr = m_sym.addVariableEntry(s);
	}
	
	if (!Code::aValue(addr, i)) {
		return setError("Invalid load");
	}
	return true;
}

bool Assembler::handleC(Code::Instruction &i)
{
	i = Code::cBase();
	if (!Code::comp(m_parser.comp(), i)) return setError("Invalid computation");
	if (!Code::dest(m_parser.dest(), i)) return setError("Invalid destination");
	if (!Code::jump(m_parser.jump(), i)) return setError("Invalid jump");
	return true;
}

void Assembler::output(Code::Instruction i)
{
	m_out << compile(i) << endl;
}

string Assembler::compile(Code::Instruction i)
{
	stringstream s;
	for (int c = 0; c < 16; ++c, i <<= 1)
		s << char( '0' + ((i & 0b1000000000000000)>>15) );
	return s.str();
}
