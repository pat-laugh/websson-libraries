//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <string>

#include "base.hpp"

namespace webss
{
	class Placeholder
	{
	public:
		string name;
		int index;
		Tuple** ptr;
	};
}