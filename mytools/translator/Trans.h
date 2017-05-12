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

	bool minimalBootstrap();
	bool bootstrap();
	bool teardown();
	bool translate(std::istream &in, const std::string &fn);
	const ErrorContainer &errors() const { return m_errs; }
	void setDebug(bool dbg);

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
		NoBootstrap,
		MiniBootstrap,
		FullBootstrap
	} m_bootStrapType;
};
