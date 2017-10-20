//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <string>

#include "various/smartIterator.hpp"

namespace webss
{
	various::SmartIterator decodeBase64(various::SmartIterator& it);
	std::string encodeBase64(various::SmartIterator& it);
}