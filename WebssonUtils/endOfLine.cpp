//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "endOfLine.h"

using namespace std;
using namespace webss;

bool webss::isLineEnd(char c, ConType con)
{
	switch (c)
	{
	case '\n': case CHAR_SEPARATOR:
		return true;
	default:
		return con.isEnd(c);
	}
}