#pragma once

namespace Trans {
	enum CommandType : int
	{
		Invalid,
		Arith, Push, Pop,
		Label, Goto, If,
		Function, Return, Call
	};
}