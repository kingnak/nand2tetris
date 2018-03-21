#pragma once

#include <istream>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include "ErrorContainer.h"

class CodeWriter;

class Translator
{
public:
	Translator(std::ostream &out);
	~Translator();

	bool minimalBootstrap(bool stackOffset);
	bool fullBootstrap();
	bool haltBootstrap();
	bool compactBootstrap();
	bool teardown();
	bool translate(std::istream &in, const std::string &fn);
	const ErrorContainer &errors() const { return m_errs; }
	void setDebug(bool dbg);
	void setBare(bool bare);

	bool verifyCalls();

private:
	struct CallSite {
		std::string function;
		int line;
	};
	// Map from function to used args/locals
	typedef std::unordered_map<std::string, int16_t> FunctionArgMap;
	// Map from call to passed args
	typedef std::unordered_map<std::string, std::unordered_map<int16_t, std::list<CallSite>>> CallMap;

private:
	bool checkEndFunction(int16_t line);
	bool startNewFunction(const std::string &func, int16_t locls);
	void addCall(const std::string &func, CallSite &&call, int16_t args);
	bool checkFunctionLocals();
	void statPushPop(const std::string &seg, int16_t off);
	bool checkReturn();

	bool setFail(const std::string &err, int line) {
		m_errs.addError(ErrorContainer::Fail, err, line);
		return false;
	}
	bool addError(const std::string &err, int line) {
		m_errs.addError(ErrorContainer::Error, err, line);
		return false;
	}
	void addWarning(const std::string &err, int line) {
		m_errs.addError(ErrorContainer::Warning, err, line);
	}

private:
	ErrorContainer m_errs;

	CodeWriter *m_cw;

	bool m_bare;
	std::string m_fn;
	FunctionArgMap m_funcs;
	CallMap m_calls;
	std::string m_curFunc;
	int16_t m_curFuncLocalsDef;
	int16_t m_curFuncLocals;
	int16_t m_curFuncArgs;
	bool m_lastWasReturn;
	std::unordered_set<std::string> m_funcLabels;

	enum {
		NoBootstrap,	// No bootstrapping active
		MiniBootstrap,	// Minimal bootstrap. Calls Sys.init without return frame. Can simulate SP as if a call frame was present
		FullBootstrap,	// Fully compliant Bootstrap. Sets SP and calls Sys.init (cannot return!)
		CompactBootstrap,	// Optimized full bootstrap
		HaltBootstrap	// Optimized bootstrap, where Sys.init will return to Sys.Halt (default provided if not given)
	} m_bootStrapType;
};
