#pragma once

#include <string>
#include <ostream>
#include <unordered_map>
#include <functional>

class CodeWriter
{
	using string = std::string;
public:
	CodeWriter(std::ostream &o);
	void setDebug(bool dbg) { m_dbg = dbg;  }
	void setFileName(const string &n) { m_fn = n; }
	void setBare(bool bare) { m_bare = bare; }
	void setReturnValOpt(bool opt) { m_retValOpt = opt; }

	bool writeCompactBootstrap();
	bool writeHaltBootstrap();
	bool writeMinimalBootstrap(bool stackOffset);
	bool writeFullBootstrap();
	bool writeHalt();
	bool writeBackend();

	bool writeArithmetic(const string &n);
	bool writePush(const string &seg, int16_t off);
	bool writePop(const string &seg, int16_t off);
	bool writeLabel(const string &lbl);
	bool writeGoto(const string &lbl);
	bool writeIfGoto(const string &lbl);
	bool writeCall(const string &func, int16_t nArgs);
	bool writeFunction(const string &func, int16_t nVars);
	bool writeReturn();

private:
	void writeUnary(const string &op);
	void writeBinary(const string &op);
	void writeCompare(const string &cmp);
	void writeCompareSimple(const string &cmp);

	void writeCallBacked(const string &func, int16_t nArgs);
	void writeCallBare(const string &func, int16_t nArgs);
	void writeReturnBare();

	void writeUnaryHeader();
	void writeBinaryHeader();
	void writeNaryFooter();
	
	void writePushD();
	void writePushA();
	void writePushConst(char v);
	void writePushVar(const string &var);
	void writePushReg(const string &reg);
	void writeDToMem(const string &mem);
	void writePopInto(char c);
	void writeLoadSPInto(char c);
	void writeOverwriteTosFrom(char c);

	void writePushConstant(int16_t v);
	void writePushDirect(const string &mem);
	void writePushIndirect(const string &base, int16_t off);

	void writePopDirect(const string &mem);
	void writePopIndirect(const string &base, int16_t off);

	void writeLoadConstant(int16_t v);
	void writeLoadAddressDirect(const string &mem, char into);
	void writeLoadAddressIndirect(const string &base, int16_t off, char into = 'A');
	void writeStoreAddressInR(const string &base, int16_t off, const string &r);
	void writeLoadAddressFromR(const string &r);

	void writeRestoreRegDecR(const string &r, const string &mem);

	void writeConditional(const string &check, const string &cmp, const string &t, const string &f);
	void writeOptionalFallthrough(const string &check, const string &cmp, const string &code);
	
	string staticName(int16_t off) const;

	void writeGotoInt(const string &lbl);
	string newLabel(const string &key, bool pre = true, bool withFile = true);
	void writeLoadLabel(const string &lbl);
	void writePlaceLabel(const string &lbl);
	string writeLoadNewLabel(const string &key, bool pre = true, bool withFile = true);
	string writePlaceNewLabel(const string &key, bool pre = true, bool withFile = true);
	void writeACmd(const string &v);
	void writeACmd(int16_t v);
	
private:
	std::ostream &m_out;
	string m_fn;
	bool m_dbg;
	bool m_bare;
	bool m_retValOpt;

	std::unordered_map<string, int16_t> m_nextLbl;

	typedef std::unordered_map<string, std::pair<bool, string>> ArithCodes;
	typedef std::unordered_map<string, std::pair<bool, string>> CompCodes;
	typedef std::unordered_map<string, string> SegCodes;
	typedef std::unordered_map<string, std::pair<int16_t, int16_t>> StaticSegCodes;
	static const ArithCodes s_arith;
	static const CompCodes s_comp;
	static const SegCodes s_segs;
	static const StaticSegCodes s_statSegs;

	static constexpr int16_t STATIC_MAX = 240;
};
