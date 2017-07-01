//MIT License
//Copyright 2017 Patrick Laughrea
#include "iteratorSwitcher.hpp"

using namespace std;
using namespace various;
using namespace webss;

IteratorSwitcher::IteratorSwitcher(SmartIterator& oldIt, SmartIterator&& newIt) : oldIt(oldIt), savedIt(move(oldIt))
{
	oldIt = move(newIt);
}
IteratorSwitcher::~IteratorSwitcher()
{
	oldIt = move(savedIt);
}