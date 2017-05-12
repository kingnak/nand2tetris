#pragma once
#include <vector>
#include <string>
#include <functional>

class ErrorContainer
{
public:
	typedef std::function<std::string(std::string, int)> ErrorProcessor;

	ErrorContainer();
	ErrorContainer(ErrorProcessor &&errorProcessor);

	enum Severity {
		Info, Debug, Warning, Error, Fail
	};

	bool addError(Severity s, std::string msg);
	bool addError(Severity s, std::string msg, int ln);

	void clear();
	bool empty() const;
	bool hasError() const;
	bool hasFail() const;
	ErrorContainer unified(const ErrorContainer &other) const;
	ErrorContainer &unifyWith(const ErrorContainer &other);

	const std::vector<std::string> &messages() const;

private:
	std::vector<std::string> m_errors;
	ErrorProcessor m_process;
	Severity m_maxLevel;
};
