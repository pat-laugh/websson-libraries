//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "utilsNumbers.h"
#include "WebssonUtils/utilsWebss.h"

using namespace std;
using namespace webss;

Webss GlobalParser::Parser::parseNumber(It& it)
{
	bool negative = checkNumberNegative(it);
	auto base = checkNumberBase(it);
	auto num = parseInt(it, base);
	if (negative)
		num = -num;

	if (!it || isNumberEnd(*it, language, base))
		return num;

	double numDouble = (double)num;

	if (isDecimalSeparator(*it, language))
	{
		auto decimals = parseDecimals(++it, base);
		if (negative)
			decimals = -decimals;

		numDouble += decimals;

		if (!it || isNumberEnd(*it, language, base))
			return numDouble;

		if (!isBaseSeparator(*it, base))
			throw runtime_error("invalid number");
	}

	if (!skipLineJunk(++it) || !isNumberStart(*it))
		throw runtime_error(ERROR_EXPECTED_NUMBER);
	return addExponent(it, numDouble, base);
}