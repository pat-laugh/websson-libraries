#pragma once

#include "typeContainers.h"
#include "constants.h"

namespace webss
{
	//return true if c is end of line, separator or end of container, else false
	bool isLineEnd(char c, ConType con);
}