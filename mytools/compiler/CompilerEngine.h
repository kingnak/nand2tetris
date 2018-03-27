#pragma once

#include <ostream>
#include <string>
#include <vector>
#include "Tokenizer.h"
#include "CodeGenerator.h"

class CompilerEngine
{
public:
	CompilerEngine(CodeGeneratorFactory *factory);
	~CompilerEngine();

	bool compile(std::istream &in, std::ostream &out);
	bool tokenize(std::istream &in, std::ostream &out);

	bool hasError() const;
	std::string error() const;

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
	struct VarDef {
		CodeGenerator::DataType type;
		std::string className;
		std::vector<std::string> vars;
	};
	bool parseVarDef(VarDef &def);
	CodeGenerator::DataType parseType();

private:
	bool accept(Tokenizer::TokenType token);
	bool expect(Tokenizer::TokenType token);
	bool accept(char symbol);
	bool expect(char symbol);
	bool accept(Tokenizer::Keyword kw);
	bool expect(Tokenizer::Keyword kw);
	bool consume();

	bool setError(std::string error);

private:
	CodeGeneratorFactory *m_fac;
	CodeGenerator *m_gen;
	Tokenizer *m_tok;
	std::string m_error;
};
