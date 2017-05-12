#pragma once

#include <istream>
#include <ostream>
#include "Parser.h"
#include "Code.h"
#include "SymbolTable.h"
#include "ErrorContainer.h"

class Assembler
{
public:
	Assembler(std::istream &in, std::ostream &o);

	bool assemble();

	const ErrorContainer &errors() const { return m_errs;  }

private:
	bool setError(const std::string &err) {
		m_errs.addError(ErrorContainer::Error, err);
		return false;
	}
	bool handleA(Code::Instruction &i);
	bool handleC(Code::Instruction &i);

	void output(Code::Instruction i);
	static std::string compile(Code::Instruction i);
	void initSymbols();

private:
	bool firstPass();
	bool secondPass();

private:
	Parser m_parser;
	SymbolTable m_sym;
	std::ostream &m_out;

	ErrorContainer m_errs;
};