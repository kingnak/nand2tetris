#include "SymbolTable.h"

bool operator == (const SymbolTable::Symbol &s1, const std::string &s2)
{
	return s1.name == s2;
}

bool operator < (const SymbolTable::Symbol &s1, const std::string &s2)
{
	return s1.name < s2;
}

bool SymbolTable::contains(const std::string &name) const
{
	return m_syms.find(name) != m_syms.end();
}

bool SymbolTable::containsRecursive(const std::string &name) const
{
	if (!contains(name)) {
		if (m_parent)
			return m_parent->containsRecursive(name);
		return false;
	}
	return true;
}

SymbolTable::Symbol SymbolTable::get(const std::string &name) const
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

	// Classes and functions only in top level table
	if (m_parent && (s.kind == Kind::Func || s.kind == Kind::Class)) {
		return m_parent->add(s);
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

///////////////////

void CallTracer::addCall(const std::string &method, bool asStatic, const std::vector<Type> &params, const std::string &site, int loc)
{
	m_calls.emplace_back(Call{ method, params, asStatic, site, loc });
}

void CallTracer::addDef(const std::string &method, bool asStatic, const std::vector<Type> &params)
{
	m_defs.insert(make_pair(method, Def{ method, params, asStatic }));
}

#include <iostream>
using namespace std;
bool CallTracer::verify()
{
	bool ok = true;
	for (auto c : m_calls) {
		auto d = m_defs.find(c.name);
		if (d == m_defs.end()) {
			cout << "E: Call to undefined function " << c.name << (c.isStatic ? " (static)" : "") << ", at " << c.site << " @ " << c.loc << endl;
			ok = false;
			continue;
		}

		if (d->second.isStatic != c.isStatic) {
			if (c.isStatic) {
				cout << "E: Calling " << c.name << " as static method" << ", at " << c.site << " @ " << c.loc << endl;
				ok = false;
			} else {
				cout << "W: Calling static " << c.name << " with object" << ", at " << c.site << " @ " << c.loc << endl;
			}
		}

	}
	char c;
	cin >> c;
	return ok;
}

void CallTracer::injectDefaults()
{
	vector<string> defStats = {
		"Math.multiply", "Math.divide", "Math.min", "Math.max", "Math.sqrt",
		"String.new", "String.backSpace", "String.doubleQuote", "String.newLine",
		"Array.new",
		"Output.moveCursor", "Output.printChar", "Output.printString", "Output.printInt", "Output.println", "Output.backSpace",
		"Screen.clearScreen", "Screen.setColor", "Screen.drawPixel", "Screen.drawLine", "Screen.drawRectangle", "Screen.drawCircle",
		"Keyboard.keyPressed", "Keyboard.readChar", "Keyboard.readLine", "Keyboard.readInt",
		"Memory.peek", "Memory.poke", "Memory.alloc", "Memory.deAlloc",
		"Sys.halt", "Sys.error", "Sys.wait"
	};
	vector<string> defMeths = {
		"String.dispose", "String.length", "String.charAt", "String.setCharAt", "String.appendChar", "String.eraseLastChar", "String.intValue", "String.setInt", 
		"Array.dispose"
	};

	for (auto it : defStats) {
		if (m_defs.find(it) == m_defs.end()) {
			m_defs.insert(make_pair(it, Def{ it, {}, true }));
		}
	}

	for (auto it : defMeths) {
		if (m_defs.find(it) == m_defs.end()) {
			m_defs.insert(make_pair(it, Def{ it,{}, false }));
		}
	}
}
