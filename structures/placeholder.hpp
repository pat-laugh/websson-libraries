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
		int index;
		const Tuple** ptr;
		
		Placeholder(int index, const Tuple** ptr) : index(index), ptr(ptr) {}
		
		bool operator==(const Placeholder& o) const { return index == o.index; }
		bool operator!=(const Placeholder& o) const { return !(*this == o); }
	};
}