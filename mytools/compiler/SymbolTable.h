#pragma once
#include <string>
#include <map>

class SymbolTable
{
public:
	enum class Kind {
		NONE, Static, Field, Argument, Var, Class, Func
	};
	enum class Type {
		None, Int, String, Char, Class, Boolean, Void
	};

	struct Symbol {
		Kind kind = Kind::NONE;
		Type type = Type::None;
		std::string classType;
		std::string name;
		int order = -1;
	};

	bool contains(const std::string &name);
	bool containsRecursive(const std::string &name);
	Symbol get(const std::string &name);

	bool add(Symbol &s);

	SymbolTable *createSubTable();
	SymbolTable *toParent();
	SymbolTable *toParentAndDiscard();

private:
	std::map<std::string, Symbol> m_syms;
	SymbolTable *m_parent;
};
