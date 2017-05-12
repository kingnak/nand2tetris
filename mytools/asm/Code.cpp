#include "Code.h"

const std::unordered_map<std::string, Code::Instruction> Code::s_dest = { 
	{ "",    0b000 },
	{ "M",   0b001 << 3 },
	{ "D",   0b010 << 3 },
	{ "MD",  0b011 << 3 },
	{ "A",   0b100 << 3 }, 
	{ "AM",  0b101 << 3 },
	{ "AD",  0b110 << 3 },
	{ "AMD", 0b111 << 3 }
};

const std::unordered_map<std::string, Code::Instruction> Code::s_jump = {
	{ "",    0b000 },
	{ "JGT", 0b001 },
	{ "JEQ", 0b010 },
	{ "JGE", 0b011 },
	{ "JLT", 0b100 },
	{ "JNE", 0b101 },
	{ "JLE", 0b110 },
	{ "JMP", 0b111 }
};

const std::unordered_map<std::string, std::pair<Code::Instruction,bool>> Code::s_comp = {
	{ "0",   { 0b101010 << 6, false } },
	{ "1",   { 0b111111 << 6, false } },
	{ "-1",  { 0b111010 << 6, false } },
	{ "D",   { 0b001100 << 6, false } },
	{ "A",   { 0b110000 << 6, false } }, { "M",   { 0b110000 << 6, true } },
	{ "!D",  { 0b001101 << 6, false } },
	{ "!A",  { 0b110001 << 6, false } }, { "!M",  { 0b110001 << 6, true } },
	{ "-D",  { 0b001111 << 6, false } },
	{ "-A",  { 0b110011 << 6, false } }, { "-M",  { 0b110011 << 6, true } },
	{ "D+1", { 0b011111 << 6, false } },
	{ "A+1", { 0b110111 << 6, false } }, { "M+1", { 0b110111 << 6, true } },
	{ "D-1", { 0b001110 << 6, false } },
	{ "A-1", { 0b110010 << 6, false } }, { "M-1", { 0b110010 << 6, true } },
	{ "D+A", { 0b000010 << 6, false } }, { "D+M", { 0b000010 << 6, true } },
	{ "D-A", { 0b010011 << 6, false } }, { "D-M", { 0b010011 << 6, true } },
	{ "A-D", { 0b000111 << 6, false } }, { "M-D", { 0b000111 << 6, true } },
	{ "D&A", { 0b000000 << 6, false } }, { "D&M", { 0b000000 << 6, true } },
	{ "D|A", { 0b010101 << 6, false } }, { "D|M", { 0b010101 << 6, true } }
};

bool Code::aValue(int16_t v, Instruction &i)
{
	if (i != aBase()) return false;
	if (v < 0) return false;
	i |= v;
	return true;
}

constexpr Code::Instruction Code::memAccess()
{
	return 0b0001000000000000;
}

bool Code::dest(const std::string &d, Instruction &i)
{
	if ((i & cBase()) != cBase()) return false;
	auto it = s_dest.find(d);
	if (it == s_dest.cend()) return false;
	i |= it->second;
	return true;
}

bool Code::comp(const std::string &c, Instruction &i)
{
	if ((i & cBase()) != cBase()) return false;
	auto it = s_comp.find(c);
	if (it == s_comp.end()) return false;
	i |= it->second.first;
	if (it->second.second) i |= memAccess();
	return true;
}

bool Code::jump(const std::string &j, Instruction &i)
{
	if ((i & cBase()) != cBase()) return false;
	auto it = s_jump.find(j);
	if (it == s_jump.end()) return false;
	i |= it->second;
	return true;
}
