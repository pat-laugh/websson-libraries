//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "utils/smartIterator.hpp"

namespace webss
{
	class IteratorSwitcher
	{
	private:
		SmartIterator& oldIt;
		SmartIterator savedIt;
	public:
		IteratorSwitcher(SmartIterator& oldIt, SmartIterator&& newIt);
		~IteratorSwitcher();
	};
}