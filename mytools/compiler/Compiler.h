#pragma once
#include <istream>
#include <ostream>

class AbstractCompilerEngineFactory;

class Compiler
{
public:
	Compiler(AbstractCompilerEngineFactory *factory);
	~Compiler();

	bool compile(std::istream &in, std::ostream &out);

private:
	AbstractCompilerEngineFactory *m_factory;
};
