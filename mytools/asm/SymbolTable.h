#pragma once

#include <unordered_map>
#include <string>

class SymbolTable
{
public:
	SymbolTable() : m_nextVariable(0) {}

	void clear() {
		m_map.clear(); 
		m_nextVariable = 0;
	}

	void addFixedEntry(const std::string &symbol, int16_t address);
	int16_t addVariableEntry(const std::string &symbol);
	bool contains(const std::string &symbol) const;
	int16_t getAddress(const std::string &symbol) const;

private:
	std::unordered_map<std::string, int16_t> m_map;
	int16_t m_nextVariable;
};