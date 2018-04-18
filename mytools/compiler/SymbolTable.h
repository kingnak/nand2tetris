#pragma once
#include <string>
#include <map>
#include <vector>

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

	bool contains(const std::string &name) const;
	bool containsRecursive(const std::string &name) const;
	Symbol get(const std::string &name) const;

	bool add(Symbol &s, int ordinalOffset = 0);

	SymbolTable *createSubTable();
	SymbolTable *toParent();
	SymbolTable *toParentAndDiscard();

	int count(Kind k) const;

private:
	std::map<std::string, Symbol> m_syms;
	SymbolTable *m_parent;
};

class CallTracer
{
public:
	struct Type {
		using DataType = SymbolTable::Type;
		DataType dataType;
		std::string classType;
	};

	void addCall(const std::string &method, bool asStatic, const std::vector<Type> &params, const std::string &site, int loc);
	void addDef(const std::string &method, bool asStatic, const std::vector<Type> &params);

	bool verify();
	void injectDefaults();

private:
	struct Def {
		std::string name;
		std::vector<Type> params;
		bool isStatic;
	};
	struct Call {
		std::string name;
		std::vector<Type> params;
		bool isStatic;
		std::string site;
		int loc;
	};
	std::vector<Call> m_calls;
	std::map<std::string, Def> m_defs;
};
