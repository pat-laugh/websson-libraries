//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "base.hpp"

namespace webss
{
	class Placeholder
	{
	private:
		int index;
		const Tuple** ptr;

	public:
		Placeholder(int index, const Tuple** ptr);
		
		bool operator==(const Placeholder& o) const;
		bool operator!=(const Placeholder& o) const;

		const Webss& getValue() const;
	};
}