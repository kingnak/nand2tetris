#pragma once

#include <string>
#include <unordered_map>
#include <cinttypes>

class Code
{
public:
	typedef uint16_t Instruction;

	static constexpr Instruction aBase();
	static constexpr Instruction cBase();
	static bool aValue(int16_t v, Instruction &i);

	static bool dest(const std::string &d, Instruction &i);
	static bool comp(const std::string &c, Instruction &i);
	static bool jump(const std::string &j, Instruction &i);

private:
	static constexpr Instruction memAccess();

	static const std::unordered_map<std::string, Instruction> s_dest;
	static const std::unordered_map<std::string, std::pair<Instruction, bool>> s_comp;
	static const std::unordered_map<std::string, Instruction> s_jump;
};

constexpr Code::Instruction Code::aBase()
{
	return 0x0u;
}

constexpr Code::Instruction Code::cBase()
{
	return 0b1110000000000000;
}