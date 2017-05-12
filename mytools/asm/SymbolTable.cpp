#include "SymbolTable.h"

using namespace std;

void SymbolTable::addFixedEntry(const string &symbol, int16_t address)
{
	m_map.insert(make_pair(symbol, address));
}

int16_t SymbolTable::addVariableEntry(const std::string &symbol)
{
	m_map.insert(make_pair(symbol, m_nextVariable));
	return m_nextVariable++;
}

bool SymbolTable::contains(const string &symbol) const
{
	return m_map.find(symbol) != m_map.cend();
}

int16_t SymbolTable::getAddress(const string &symbol) const
{
	auto it = m_map.find(symbol);
	if (it == m_map.cend()) return -1;
	return it->second;
}
