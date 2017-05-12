#include "ErrorContainer.h"
#include <algorithm>

using namespace std;

ErrorContainer::ErrorContainer()
	: ErrorContainer([](string s,int)->string {return s; })
{
}

ErrorContainer::ErrorContainer(ErrorProcessor &&errorProcessor)
	: m_process(move(errorProcessor))
	, m_maxLevel(Info)
{

}

bool ErrorContainer::addError(Severity s, string msg)
{
	switch (s) {
	case Debug: msg = "Debug: " + msg; break;
	case Info: msg = "Info: " + msg; break;
	case Warning: msg = "Warning: " + msg; break;
	case Error: msg = "Error: " + msg; break;
	case Fail: msg = "FATAL: " + msg; break;
	}
	m_errors.push_back(msg);
	m_maxLevel = max(m_maxLevel, s);
	return s != Error;
}

bool ErrorContainer::addError(Severity s, string msg, int ln)
{
	return addError(s, m_process(msg, ln));
}

void ErrorContainer::clear()
{
	m_maxLevel = Info;
	m_errors.clear();
}

bool ErrorContainer::empty() const
{
	return m_errors.empty();
}

bool ErrorContainer::hasError() const
{
	return m_maxLevel >= Error;
}

bool ErrorContainer::hasFail() const
{
	return m_maxLevel >= Fail;
}

ErrorContainer &ErrorContainer::unifyWith(const ErrorContainer &other)
{
	m_errors.insert(m_errors.end(), other.m_errors.cbegin(), other.m_errors.cend());
	m_maxLevel = max(m_maxLevel, other.m_maxLevel);
	return *this;
}

ErrorContainer ErrorContainer::unified(const ErrorContainer &other) const
{
	return ErrorContainer(ErrorProcessor(m_process)).unifyWith(*this).unifyWith(other);
}

const vector<string> &ErrorContainer::messages() const
{
	return m_errors;
}
