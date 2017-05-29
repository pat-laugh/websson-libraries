//MIT License
//Copyright 2017 Patrick Laughrea
#include "parserNumbers.hpp"

#include "errors.hpp"
#include "utilsNumbers.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

Webss webss::parseNumber(Parser& parser)
{
	auto& it = parser.getIt();
	bool negative = checkNumberNegative(it);
	auto base = checkNumberBase(it);
	auto num = parseInt(it, base);
	if (negative)
		num = -num;

	if (!it || isNumberEnd(*it, base))
		return num;

	double numDouble = (double)num;

	if (*it == CHAR_DECIMAL_SEPARATOR)
	{
		auto decimals = parseDecimals(++it, base);
		if (negative)
			decimals = -decimals;

		numDouble += decimals;

		if (!it || isNumberEnd(*it, base))
			return numDouble;

		if (!isBaseSeparator(*it, base))
			throw runtime_error("invalid number");
	}

	if (!skipLineJunk(++it) || !isNumberStart(*it))
		throw runtime_error(ERROR_EXPECTED_NUMBER);
	return addExponent(it, numDouble, base);
}