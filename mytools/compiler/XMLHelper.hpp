#pragma once

#include <string>

std::string xmlClean(char c)
{
	switch (c) {
	case '<': return "&lt;";
	case '>': return "&gt;";
	case '&': return "&amp;";
	default: return std::string(1, c);
	}
}

std::string xmlClean(std::string s)
{
	std::string ret;
	for (char c : s) {
		ret += xmlClean(c);
	}
	return ret;
}
