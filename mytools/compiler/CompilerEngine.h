#pragma once

#include <ostream>

class CodeGeneratorFactory;
class CodeGenerator;
class Tokenizer;

class CompilerEngine
{
public:
	CompilerEngine(CodeGeneratorFactory *factory);
	~CompilerEngine();

	bool compile(std::istream &in, std::ostream &out) { return false; }
	bool tokenize(std::istream &in, std::ostream &out);

private:
	bool compileClass();
	bool compileClassVarDec();
	bool compileSubroutine();
	bool compileParameterList();
	bool compileVarDec();
	bool compileStatements();
	bool compileDo();
	bool compileLet();
	bool compileWhile();
	bool compileIf();
	bool compileExpression();
	bool compileTerm();
	bool compileExpressionList();

private:
	CodeGeneratorFactory *m_fac;
	CodeGenerator *m_gen;
	Tokenizer *m_tok;
};
