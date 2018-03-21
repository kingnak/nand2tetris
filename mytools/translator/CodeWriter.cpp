#include "CodeWriter.h"
#include <sstream>

using namespace std;

static string stringify(int16_t v) {
	stringstream s;
	s << v;
	return s.str();
}

const CodeWriter::ArithCodes CodeWriter::s_arith = {
	{ "add", make_pair(true, "M=D+M") },
	{ "sub", make_pair(true, "M=M-D") },
	{ "neg", make_pair(false, "M=-M") },
	{ "and", make_pair(true, "M=D&M") },
	{ "or", make_pair(true, "M=D|M") },
	{ "not", make_pair(false, "M=!M") }
};

const CodeWriter::CompCodes CodeWriter::s_comp = {
	{ "eq", make_pair(true, "JEQ") },
	{ "gt", make_pair(false, "JGE") },
	{ "lt", make_pair(false, "JLE") }
};

const CodeWriter::SegCodes CodeWriter::s_segs = {
	{ "local", "LCL" },
	{ "argument", "ARG" },
	{ "this", "THIS" },
	{ "that", "THAT" }
};

const CodeWriter::StaticSegCodes CodeWriter::s_statSegs = {
	{ "pointer", make_pair(3, 2) },
	{ "temp", make_pair(5, 8) }
};

CodeWriter::CodeWriter(ostream &o) 
	: m_out(o)
	, m_dbg(false) 
	, m_bare(false)
{
}

bool CodeWriter::writeMinimalBootstrap(bool stackOffset)
{
	if (m_dbg) m_out << "// Bootstrap\n";

	// No frame for Sys.init.
	// Just set SP and LCL, and call
	writeACmd(256 + (stackOffset ? 5 : 0));
	m_out << "D=A\n";
	writeDToMem("SP");
	writeDToMem("LCL");
	writeGotoInt("Sys.init");
	return true;
}

bool CodeWriter::writeBootstrap()
{
	if (m_dbg) m_out << "// Bootstrap\n";

#if 1
	// Create stack frame and call Sys.init
	// RAM[0-5] = 261 261 256 0 0, RAM[256] = @ret
	
	// TODO: enable an external sys.halt function.
	// For that to work, we need a valid LCL in the frame,
	// so that the Sys.halt can use locals 
	// (Sys.init returns to it, doesn't call it)

	writeACmd("Sys.halt");
	m_out << "D=A\n";
	writeDToMem("256"); // Place ret at final TOS
	m_out << "D=A\n";	// D = 256
	m_out << "A=A+1\n";	// Return frame's LCL (at 257) is 257
	m_out << "M=D+1\n";
	writeDToMem("ARG"); // The argument pointer is TOS
	
	writeACmd(256 + 5);	// +Size of Frame
	m_out << "D=A\n";
	writeDToMem("SP");	// SP and LCL are located here
	writeDToMem("LCL");

	// Make THIS + THAT 0 (Optional...)
	m_out <<
		"@THIS\n"
		"M=0\n"
		"@THAT\n"
		"M=0\n"
		;
	writeGotoInt("Sys.init");

#else
	//Full Bootstrap:
	// Set SP to 256 and call Sys.init
	writeACmd(256);
	m_out << "D=A\n";
	writeDToMem("SP");
	writeCall("Sys.init", 0);
#endif

	return true;
}

bool CodeWriter::writeHalt()
{
	if (m_dbg) m_out << "\n// Halt\n";

	writePlaceLabel("Sys.halt");
	writeGotoInt("Sys.halt");
	return true;
}

bool CodeWriter::writeBackend()
{
	if (m_bare) return true;

	// Implementation for long operations, like call and return
	if (m_dbg) m_out << "\n// CORE:\n";

	////////////////////////////////////
	// Call:
	// - Return address must be on stack
	// - Destination address must be in R13
	// - D must be nArgs + 1
	
	if (m_dbg) m_out << "\n // CALL\n";
	writePlaceLabel("_call");
	writeACmd("SP");
	m_out << "D=-D\n";
	m_out << "D=D+M\n";
	writeDToMem("R14");	// This is where to put ARG

	writePushReg("LCL");
	writePushReg("ARG");
	writePushReg("THIS");
	writePushReg("THAT");

	writeACmd("R14");
	m_out << "D=M\n";
	writeDToMem("ARG");

	// Place LCL to current SP
	writeACmd("SP");
	m_out << "D=M\n";
	writeDToMem("LCL");

	// Write goto
	writeACmd("R13");
	m_out << "A=M;JMP\n";

	////////////////////////////////////
	// Return
	if (m_dbg) m_out << "\n // RETURN\n";
	writePlaceLabel("_return");
	// store frame = LCL in R13
	writeStoreAddressInR("LCL", 0, "R13");

	// Store return address = *(frame-5) in R14 (Could be overridden by *ARG = XXX for 0 arg functions...)
	// (D = LCL)
	writeACmd(5);
	m_out <<
		"A=D-A\n"
		"D=M\n"
		;
	writeDToMem("R14");

	// *ARG = TOS (D)
	writeLoadSPInto('D');

	m_out <<
		"@ARG\n"
		"A=M\n"
		"M=D\n"
		;

	// SP = ARG+1
	m_out <<
		"@ARG\n"
		"D=M+1\n"
		;
	writeDToMem("SP");

	writeRestoreRegDecR("R13", "THAT");
	writeRestoreRegDecR("R13", "THIS");
	writeRestoreRegDecR("R13", "ARG");
	writeRestoreRegDecR("R13", "LCL");

	// Jump to ret addr (stored in R14)
	m_out <<
		"@R14\n"
		"A=M;JMP\n"
		;

#if 0
	/////////////////////////////////
	// Init locals
	// R13 must be return address
	// D must be nr locals
	if (m_dbg) m_out << "\n // INIT LOCALS\n";
	writePlaceLabel("_init_lcl");
	writePushConst('0');
	writeACmd("_init_lcl");
	m_out << "D=D-1;JGT\n";
	m_out <<
		"@R13\n"
		"A=M;JMP\n"
		;
#endif
	if (m_dbg) m_out << "\n// END CORE\n";

	return true;
}

bool CodeWriter::writeArithmetic(const string &n)
{
	if (m_dbg) m_out << "\n// " << n << "\n";

	auto itA = s_arith.find(n);
	if (itA != s_arith.cend()) {
		if (itA->second.first) {
			writeBinary(itA->second.second);
		}
		else {
			writeUnary(itA->second.second);
		}
		return true;
	}
	auto itC = s_comp.find(n);
	if (itC != s_comp.cend()) {
		if (itC->second.first) {
			writeCompareSimple(itC->second.second);
		} else {
			writeCompare(itC->second.second);
		}
		return true;
	}
	return false;
}

bool CodeWriter::writePush(const string &seg, int16_t off)
{
	if (m_dbg) m_out << "\n// push " << seg << " " << off << "\n";

	if (seg == "constant") {
		writePushConstant(off);
		return true;
	}

	auto itS = s_statSegs.find(seg);
	if (itS != s_statSegs.cend()) {
		if (off >= itS->second.second) return false;
		writePushDirect(stringify(itS->second.first + off));
		return true;
	}
	
	if (seg == "static") {
		if (off >= STATIC_MAX) return false;
		writePushDirect(staticName(off));
		return true;
	}

	auto it = s_segs.find(seg);
	if (it == s_segs.end()) {
		return false;
	}
	writePushIndirect(it->second, off);

	return true;
}

bool CodeWriter::writePop(const string &seg, int16_t off)
{
	if (m_dbg) m_out << "\n// pop " << seg << " " << off << "\n";

	if (seg == "constant") return false;
	auto itS = s_statSegs.find(seg);
	if (itS != s_statSegs.cend()) {
		if (off >= itS->second.second) return false;
		writePopDirect(stringify(itS->second.first + off));
		return true;
	}

	if (seg == "static") {
		if (off > STATIC_MAX) return false;
		writePopDirect(staticName(off));
		return true;
	}

	auto it = s_segs.find(seg);
	if (it == s_segs.end()) {
		return false;
	}
	writePopIndirect(it->second, off);

	return true;
}

bool CodeWriter::writeLabel(const string &lbl)
{
	if (m_dbg) m_out << "\n// label " << lbl << "\n";
	writePlaceLabel(lbl);
	return true;
}

bool CodeWriter::writeGoto(const string &lbl)
{
	if (m_dbg) m_out << "\n// goto " << lbl << "\n";
	writeGotoInt(lbl);
	return true;
}

bool CodeWriter::writeIfGoto(const string &lbl)
{
	if (m_dbg) m_out << "\n// if-goto " << lbl << "\n";
	writePopInto('D');
	writeLoadLabel(lbl);
	m_out << "D;JNE\n";
	return true;
}

bool CodeWriter::writeCall(const string &func, int16_t nArgs)
{
	if (m_dbg) m_out << "\n// call " << func << ' ' << nArgs << '\n';

	if (m_bare)
		writeCallBare(func, nArgs);
	else
		writeCallBacked(func, nArgs);
	return true;
}

void CodeWriter::writeCallBacked(const string &func, int16_t nArgs)
{
	// Push return address + registers
	string retAddress = newLabel(func + "_return_", false, false);
	writePushVar(retAddress);
	
	writeACmd(func);
	m_out << "D=A\n";
	writeDToMem("R13");
	writeACmd(nArgs);
	m_out << "D=A+1\n";
	writeGotoInt("_call");

	// Write return label
	writePlaceLabel(retAddress);
}

void CodeWriter::writeCallBare(const string &func, int16_t nArgs)
{
	// Push return address + registers
	string retAddress = newLabel(func + "_return_", false, false);
	writePushVar(retAddress);
	writePushReg("LCL");
	writePushReg("ARG");
	writePushReg("THIS");
	writePushReg("THAT");

	// reposition ARG to SP-nArgs-5
	writeACmd("SP");
	m_out << "D=M\n";
	writeACmd(5 + nArgs);
	m_out << "D=D-A\n";
	writeDToMem("ARG");

	// Place LCL to current SP
	writeACmd("SP");
	m_out << "D=M\n";
	writeDToMem("LCL");
	// Write goto
	writeGotoInt(func);

	// Write return label
	writePlaceLabel(retAddress);
}

bool CodeWriter::writeFunction(const string &func, int16_t nVars)
{
	if (m_dbg) m_out << "\n// function " << func << ' ' << nVars << '\n';

	// Write entry point
	writePlaceLabel(func);

	// Optimized nVars times "push constant 0"

	switch (nVars)
	{
	case 0:
		// Nothing to do
		break;
	case 1:
		// 4 commands
		writePushConst('0');
		break;
	case 2:
		// 7 commands
		writeACmd("SP");
		m_out << 
			"M=M+1\n"
			"M=M+1\n"
			"A=M-1\n"
			"M=0\n"
			"A=A-1\n"
			"M=0\n";
		break;
	default:
#if 0
		if (!m_bare) {
			// Jump to default local init
			// 8 Commands
			auto lbl = writeLoadNewLabel(func + "_init_", false, false);
			m_out << "D=A\n";
			writeDToMem("R13");
			writeACmd(nVars);
			m_out << "D=A\n";
			writeGotoInt("_init_lcl");
			writePlaceLabel(lbl);
		} else
#endif
		{
			// Loop push on stack
			// 8 Commands
			writeACmd(stringify(nVars));
			m_out << "D=A\n";
			writePlaceLabel(func + "_init_lcl");
			writePushConst('0');
			writeACmd(func + "_init_lcl");
			m_out << "D=D-1;JGT\n";
		}
		break;
	}
	return true;
}

bool CodeWriter::writeReturn()
{
	if (m_dbg) m_out << "\n// return\n";

	if (m_bare)
		writeReturnBare();
	else
		writeGotoInt("_return");
	
	return true;
}

void CodeWriter::writeReturnBare()
{
	// store frame = LCL in R13
	writeStoreAddressInR("LCL", 0, "R13");

	// Store return address = *(frame-5) in R14 (Could be overridden by *ARG = XXX for 0 arg functions...)
	// (D = LCL)
	writeACmd(5);
	m_out <<
		"A=D-A\n"
		"D=M\n"
		;
	writeDToMem("R14");

	// *ARG = TOS (D)
	writeLoadSPInto('D');

	m_out <<
		"@ARG\n"
		"A=M\n"
		"M=D\n"
		;

	// SP = ARG+1
	m_out <<
		"@ARG\n"
		"D=M+1\n"
		;
	writeDToMem("SP");

	writeRestoreRegDecR("R13", "THAT");
	writeRestoreRegDecR("R13", "THIS");
	writeRestoreRegDecR("R13", "ARG");
	writeRestoreRegDecR("R13", "LCL");

	// Jump to ret addr (stored in R14)
	m_out <<
		"@R14\n"
		"A=M\n"
		"0;JMP\n"
		;
}

void CodeWriter::writeBinary(const string &op)
{
	writeBinaryHeader();
	m_out << op << "\n";
}

void CodeWriter::writeUnary(const string &op)
{
	writeUnaryHeader();
	m_out << op << "\n";
}

void CodeWriter::writeCompare(const std::string &cmp)
{
	writeBinaryHeader();
	m_out << "D=D-M\n";
	writeConditional("D", cmp, "D=0", "D=-1");
	writeNaryFooter();
}

void CodeWriter::writeCompareSimple(const string &cmp)
{
	writeBinaryHeader();
	m_out << "D=D-M\n";
	writeOptionalFallthrough("D", cmp, "D=-1");
	m_out << "D=!D\n";
	writeNaryFooter();
}

void CodeWriter::writeUnaryHeader()
{
	m_out <<
		"@SP\n"
		"A=M-1\n"
		;
}

void CodeWriter::writeBinaryHeader()
{
	// Optimized pop into D + Point M to tos
	writePopInto('D');
	m_out <<
		"A=A-1\n"
		;
}

void CodeWriter::writeNaryFooter()
{
	writeOverwriteTosFrom('D');
}

void CodeWriter::writeConditional(const string &check, const string &cmp, const string &t, const string &f)
{
	auto lt = writeLoadNewLabel("_cond_t_");
	m_out << check << ';' << cmp << '\n';
	m_out << f << '\n';

	auto le = newLabel("_cond_e_");
	writeGotoInt(le);
	writePlaceLabel(lt);
	m_out << t << '\n';
	writePlaceLabel(le);
}

void CodeWriter::writeOptionalFallthrough(const string &check, const string &cmp, const string &code)
{
	auto lbl = writeLoadNewLabel("_cond_");
	m_out << check << ';' << cmp << '\n';
	m_out << code << '\n';
	writePlaceLabel(lbl);
}

void CodeWriter::writePushConstant(int16_t v)
{
	writeLoadConstant(v);
	writePushD();
}

void CodeWriter::writePushDirect(const string &mem)
{
	writeLoadAddressDirect(mem, 'D');
	writePushD();
}

void CodeWriter::writePushIndirect(const string &base, int16_t off)
{
	writeLoadAddressIndirect(base, off, 'A');
	m_out << "D=M\n";
	writePushD();
}

void CodeWriter::writePopDirect(const string &mem)
{
	writePopInto('D');
	writeDToMem(mem);
}

void CodeWriter::writePopIndirect(const string &base, int16_t off)
{
	if (off > 1) {
		writeStoreAddressInR(base, off, "R13");
		writePopInto('D');
		writeLoadAddressFromR("R13");
		m_out << "M=D\n";
	} else {
		writePopInto('D');
		writeLoadAddressIndirect(base, off);
		m_out << "M=D\n";
	}
}

void CodeWriter::writeLoadConstant(int16_t v)
{
	writeACmd(v);
	m_out << "D=A\n";
}

void CodeWriter::writeLoadAddressDirect(const string &mem, char into)
{
	writeACmd(mem);
	m_out << into << "=M\n";
}

void CodeWriter::writeLoadAddressIndirect(const string &base, int16_t off, char into /* = 'A' */)
{
	writeACmd(base);
	switch (off) {
	case 0:
		m_out << into << "=M\n";
		break;
	case 1:
		m_out << into << "=M+1\n";
		break;
	default:
		m_out << "D=M\n";
		writeACmd(off);
		m_out << into << "=D+A\n";
	}
}

void CodeWriter::writeStoreAddressInR(const string &base, int16_t off, const string &r)
{
	writeLoadAddressIndirect(base, off, 'D');
	writeDToMem(r);
}

void CodeWriter::writeLoadAddressFromR(const string &r)
{
	writeLoadAddressDirect(r, 'A');
}

void CodeWriter::writeRestoreRegDecR(const string &r, const string &mem)
{
	writeACmd(r);
	m_out << 
		"AM=M-1\n"
		"D=M\n";	// Decrease R and dereference
	writeDToMem(mem);	// Restore mem
}

void CodeWriter::writePopInto(char c)
{
	m_out <<
		"@SP\n"
		"AM=M-1\n"
		<< c << "=M\n";
}

void CodeWriter::writeLoadSPInto(char c)
{
	m_out <<
		"@SP\n"
		"A=M-1\n"
		<< c << "=M\n";
}

void CodeWriter::writeOverwriteTosFrom(char c)
{
	m_out <<
		"@SP\n"
		"A=M-1\n"
		"M=" << c << "\n";
}

void CodeWriter::writePushConst(char v)
{
	m_out <<
		"@SP\n"
		"M=M+1\n"
		// Optimized writeOverwriteTosFrom(v), as A-1 is dest address (no need to load @SP)
		"A=M-1\n"
		"M=" << v << '\n';
}

void CodeWriter::writePushD()
{
	writePushConst('D');
}

void CodeWriter::writePushA()
{
	m_out << "D=A\n";
	writePushD();
}

void CodeWriter::writePushVar(const string &var)
{
	writeACmd(var);
	writePushA();
}

void CodeWriter::writePushReg(const string &reg)
{
	writeACmd(reg);
	m_out << "D=M\n";
	writePushD();
}

void CodeWriter::writeDToMem(const string &mem)
{
	writeACmd(mem);
	m_out << "M=D\n";
}

string CodeWriter::staticName(int16_t off) const
{
	stringstream s;
	s << m_fn << '.' << off;
	return s.str();
}

void CodeWriter::writeGotoInt(const string &lbl)
{
	writeLoadLabel(lbl);
	m_out << "0;JMP\n";
}

string CodeWriter::newLabel(const string &key, bool pre, bool withFile)
{
	stringstream s;
	auto off = m_nextLbl[key]++;
	if (pre) {
		s << key;
		if (withFile) s << m_fn;
		s << '.' << off;
	} else {
		if (withFile) s << m_fn << '.';
		s << key << off;
	}
	return s.str();
}

void CodeWriter::writeLoadLabel(const string &lbl)
{
	writeACmd(lbl);
}

void CodeWriter::writePlaceLabel(const string &lbl)
{
	m_out << '(' << lbl << ")\n";
}

string CodeWriter::writeLoadNewLabel(const string &key, bool pre, bool withFile)
{
	auto l = newLabel(key, pre, withFile);
	writeLoadLabel(l);
	return l;
}

string CodeWriter::writePlaceNewLabel(const string &key, bool pre, bool withFile)
{
	auto l = newLabel(key, pre, withFile);
	writePlaceLabel(l);
	return l;
}

void CodeWriter::writeACmd(const string &v)
{
	m_out << '@' << v << '\n';
}

void CodeWriter::writeACmd(int16_t v)
{
	m_out << '@' << v << '\n';
}
