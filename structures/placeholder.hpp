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
		std::string name;
		int index;
		Tuple** ptr;
		
		bool operator==(const Placeholder& o) const { return name == o.name && index == o.index; }
		bool operator!=(const Placeholder& o) const { return !(*this == o); }
	};
}