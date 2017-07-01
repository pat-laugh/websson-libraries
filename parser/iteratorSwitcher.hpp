//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "various/smartIterator.hpp"

namespace webss
{
	class IteratorSwitcher
	{
	private:
		various::SmartIterator& oldIt;
		various::SmartIterator savedIt;
	public:
		IteratorSwitcher(various::SmartIterator& oldIt, various::SmartIterator&& newIt);
		~IteratorSwitcher();
	};
}