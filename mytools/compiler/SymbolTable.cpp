#include "SymbolTable.h"

bool operator == (const SymbolTable::Symbol &s1, const std::string &s2)
{
	return s1.name == s2;
}

bool operator < (const SymbolTable::Symbol &s1, const std::string &s2)
{
	return s1.name < s2;
}

bool SymbolTable::contains(const std::string &name)
{
	return m_syms.find(name) != m_syms.end();
}

bool SymbolTable::containsRecursive(const std::string &name)
{
	if (!contains(name)) {
		if (m_parent)
			return m_parent->containsRecursive(name);
		return false;
	}
	return true;
}

SymbolTable::Symbol SymbolTable::get(const std::string &name)
{
	auto it = m_syms.find(name);
	if (it != m_syms.end()) {
		return it->second;
	}
	if (m_parent) {
		return m_parent->get(name);
	}
	return Symbol();
}

bool SymbolTable::add(Symbol &s)
{
	if (containsRecursive(s.name)) {
		return false;
	}

	int maxOrder = -1;
	for (auto it = m_syms.begin(); it != m_syms.end(); ++it) {
		if (it->second.kind == s.kind) {
			if (it->second.order > maxOrder) maxOrder = it->second.order;
		}
	}

	s.order = maxOrder + 1;
	m_syms.insert(make_pair(s.name, s));
	return true;
}

SymbolTable *SymbolTable::createSubTable()
{
	SymbolTable *s = new SymbolTable;
	s->m_parent = this;
	return s;
}

SymbolTable *SymbolTable::toParent()
{
	return m_parent;
}

SymbolTable *SymbolTable::toParentAndDiscard()
{
	SymbolTable *ret = m_parent;
	delete this;
	return ret;
}
