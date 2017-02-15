//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "constants.h"
#include "typeContainers.h"

namespace webss
{
	//return true if c is end of line, separator or end of container, else false
	bool isLineEnd(char c, ConType con);
}