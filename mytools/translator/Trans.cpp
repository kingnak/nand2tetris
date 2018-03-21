#include "Trans.h"
#include "Parser.h"
#include "CodeWriter.h"
#include <sstream>
#include <algorithm>

using namespace std;

Translator::Translator(ostream &out)
	:  m_cw(new CodeWriter(out))
	, m_bare(false)
	, m_curFuncLocalsDef(0)
	, m_curFuncLocals(0)
	, m_curFuncArgs(0)
	, m_lastWasReturn(false)
	, m_bootStrapType(NoBootstrap)
	, m_errs([this](string s, int l)->string {stringstream ss; ss << s << "\n\tin file " << this->m_fn << ", line " << l; return ss.str(); })
{
}

Translator::~Translator()
{
	delete m_cw;
}

bool Translator::minimalBootstrap(bool stackOffset)
{
	if (m_bootStrapType != NoBootstrap) return m_errs.addError(ErrorContainer::Fail, "Bootstrapping code already written", 0);
	m_bootStrapType = MiniBootstrap;
	addCall("Sys.init", CallSite{ "{Bootstrap}",0 }, 0);
	bool ok = m_cw->writeMinimalBootstrap(stackOffset);
	ok &= m_cw->writeBackend();
	return ok;
}

bool Translator::bootstrap()
{
	if (m_bootStrapType != NoBootstrap) return m_errs.addError(ErrorContainer::Fail, "Bootstrapping code already written", 0);
	m_bootStrapType = FullBootstrap;
	addCall("Sys.init", CallSite{ "{Bootstrap}", 0 }, 0);
	addCall("Sys.halt", CallSite{ "{Bootstrap}", 0 }, 0);
	bool ok = m_cw->writeBootstrap();
	ok &= m_cw->writeBackend();
	return ok;
}

bool Translator::teardown()
{
	if (m_bootStrapType == FullBootstrap) {
		// Check for custom Sys.halt
		if (m_funcs.find("Sys.halt") == m_funcs.end()) {
			m_funcs.emplace(make_pair("Sys.halt", 0));
			if (!m_cw->writeHalt()) return false;
		}
	}
	return true;
}

bool Translator::translate(istream &in, const string &fn)
{
	struct FNSetter {
		FNSetter(string &fn, const string &toFn) : fn_(fn) { fn_ = toFn; }
		~FNSetter() { fn_.clear(); }
		string &fn_;
	} fnsetter_(m_fn, fn);

	m_lastWasReturn = true;
	m_curFunc.clear();
	m_curFuncLocals = 0;
	m_curFuncArgs = 0;

	Parser p(in);
	if (!p.parse()) {
		m_errs.unifyWith(p.errors());
		return !m_errs.hasError();
	}

	CodeWriter &w = *m_cw;
	w.setFileName(fn);
	while (p.hasMoreCommands())
	{
		bool ok = false;
		p.advance();

		if (!m_bare) {
			if (m_curFunc.empty() && p.commandType() != Trans::CommandType::Function) {
				return setFail("Code before function in " + fn, p.line());
			}
		}

		bool curReturn = false;

		switch (p.commandType()) {
		case Trans::CommandType::Arith:
			ok = w.writeArithmetic(p.arg1());
			break;
		case Trans::CommandType::Push:
			ok = w.writePush(p.arg1(), p.arg2());
			statPushPop(p.arg1(), p.arg2());
			break;
		case Trans::CommandType::Pop:
			ok = w.writePop(p.arg1(), p.arg2());
			statPushPop(p.arg1(), p.arg2());
			break;
		case Trans::CommandType::Label:
			if (m_funcLabels.find(p.arg1()) != m_funcLabels.end()) {
				addError("Duplicate label " + p.arg1() + " in " + m_curFunc, p.line());
				continue;
			}
			m_funcLabels.insert(p.arg1());
			ok = w.writeLabel(m_curFunc + "$" + p.arg1());
			break;
		case Trans::CommandType::Goto:
			ok = w.writeGoto(m_curFunc + "$" + p.arg1());
			break;
		case Trans::CommandType::If:
			ok = w.writeIfGoto(m_curFunc + "$" + p.arg1());
			break;
		case Trans::CommandType::Function:
			if (!checkEndFunction(p.line())) {
				continue;
			}
			if (!startNewFunction(p.arg1(), p.arg2())) {
				addError("Function " + p.arg1() + " already defined", p.line());
				continue;
			}
			ok = w.writeFunction(p.arg1(), p.arg2());
			break;
		case Trans::CommandType::Return:
			if (!checkReturn()) {
				addWarning(m_curFunc + " should never return", p.line());
				continue;
			}
			ok = w.writeReturn();
			curReturn = true;
			break;
		case Trans::CommandType::Call:
			ok = w.writeCall(p.arg1(), p.arg2());
			addCall(p.arg1(), CallSite{ m_curFunc, p.line() }, p.arg2());
			break;
		}
		if (!ok)
			addError("Command error: " + p.sourceLine(), p.line());

		m_lastWasReturn = curReturn;
	}

	m_errs.unifyWith(p.errors());
	return checkEndFunction(p.line()) && !m_errs.hasError();
}

bool Translator::verifyCalls()
{
	bool ret = true;

	auto dumpAllLocations = [](const list<CallSite> &lst) -> string {
		stringstream err;
		for (auto carg : lst) {
			err << "\n\tat " << carg.function << ", Line " << carg.line;
		}
		return err.str();
	};

	for (auto c : m_calls) {
		auto f = m_funcs.find(c.first);

		if (f == m_funcs.end()) {
			stringstream err;
			err << "Function " << c.first << " not found";
			for (auto call : c.second) {
				err << dumpAllLocations(call.second);
			}
			m_errs.addError(ErrorContainer::Error, err.str());
			continue;
		}
		
		int16_t minArgs = f->second;
		for (auto call : c.second) {
			if (call.first < minArgs) {
				stringstream err;
				err << "Function " << c.first << " requires " << minArgs << " Arguments. "
					<< call.first << " provided";

				err << dumpAllLocations(call.second);
				ret = false;
				m_errs.addError(ErrorContainer::Error, err.str());
			}
		}

		// Call completely handled. Remove from function list
		m_funcs.erase(f);
	}

	// Uncalled functions
	for (auto f : m_funcs) {
		m_errs.addError(ErrorContainer::Warning, f.first + " is never called");
		// Not an Error
	}

	/*
	string allErrs;
	for_each(errs.begin(), errs.end(), [&](auto it) {allErrs.append("\n"); allErrs.append(it); });
	setError(m_err + allErrs, m_errLine);
	*/
	return ret;
}

void Translator::setDebug(bool dbg)
{
	m_cw->setDebug(dbg);
}

void Translator::setBare(bool bare)
{
	m_bare = bare;
	m_cw->setBare(bare);
}

bool Translator::checkEndFunction(int16_t line)
{
	if (m_curFunc.empty()) 
		return true;

	m_funcs[m_curFunc] = m_curFuncArgs;

	// EXCEPTION FOR Sys.init!
	if (!m_lastWasReturn)
		if (m_curFunc != "Sys.init" && m_curFunc != "Sys.halt")
			return addError("Function " + m_curFunc + " requires return at end", line);
	
	if (!checkFunctionLocals()) 
		return addError("Function " + m_curFunc + " uses too many locals", line);
	
	return true;
}

bool Translator::startNewFunction(const string &func, int16_t locls)
{
	m_curFunc = func;
	m_curFuncLocalsDef = locls;
	m_curFuncLocals = 0;
	m_curFuncArgs = 0;
	m_funcLabels.clear();

	if (m_funcs.find(func) != m_funcs.end()) return false;

	m_funcs.emplace(make_pair(func, 0));
	return true;
}

void Translator::addCall(const string &func, CallSite &&call, int16_t args)
{
	m_calls[func][args].emplace_back(call);
}

bool Translator::checkFunctionLocals()
{
	if (!m_curFunc.empty())
		// m_curFuncLocals is 1 rebased (i.e. 0 means NO local access, 1 means access to local 0, ...)
		if (m_curFuncLocals > m_curFuncLocalsDef) 
			return false;

	return true;
}

bool Translator::checkReturn()
{
	switch (m_bootStrapType) {
	case MiniBootstrap:
		if (m_curFunc == "Sys.init") return false;
		return true;
	case FullBootstrap:
		if (m_curFunc == "Sys.halt") return false;
		return true;
	}
	return true;
}

void Translator::statPushPop(const string &seg, int16_t off)
{
	if (seg == "argument") {
		m_curFuncArgs = max(int16_t(off+1), m_curFuncArgs);
	} else if (seg == "local") {
		m_curFuncLocals = max(int16_t(off+1), m_curFuncLocals);
	}
}
