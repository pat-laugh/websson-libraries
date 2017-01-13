//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "utilsSweepers.h"

using namespace std;
using namespace webss;

/* skip junk must have its own language where
the chars '/' and '\' are recognized as operators
-- There are multiple languages within one language
1) There is the main language
2) Language of strings
3) Language of junk

There are actually 3 kinds of operators, both consisting of two chars:
Line escape: '\' immediately followed by '\n' or whitespace or eof
Line comment: '/' immediately followed by '/'
Multiline comment: '/' immediately followed by '*'

If '\' or '/' is met, but not an operator, then it is left
where it was.

What line escape does:
	1) Skip current line. All chars after '\' must be whitespace, '\n', or eof
		If not, there is an error.
	2) Skip whitespace on line after it.
*/

bool checkOperators(SmartIterator& it);

SmartIterator& webss::skipJunk(SmartIterator& it)
{
	while (it)
		if (isJunk(*it))
			++it;
		else if (!checkOperators(it))
			break;
	return it;
}

SmartIterator& webss::skipWhitespace(SmartIterator& it)
{
	while (it)
		if (isWhitespace(*it))
			++it;
		else
		{
			checkOperators(it);
			break;
		}
	return it;
}

SmartIterator& webss::skipLine(SmartIterator& it)
{
	while (it && *it != '\n')
		++it;
	return it;
}

SmartIterator& webss::skipMultilineComment(SmartIterator& it)
{
	if (!it)
		throw runtime_error("comment is not closed");
	
	for (int num = 1, char c = *it; ++it; c = *it)
		if (c == '*')
		{
			if (*it == '/')
			{
				if (--num == 0)
					return skipWhitespace(++it);
				if (!++it)
					break;
			}
		}
		else if (c == '/' && *it == '*')
		{
			++num;
			if (!++it)
				break;
		}
	
	throw runtime_error("comment is not closed");
}

bool checkOperators(SmartIterator& it)
{
	if (*it == CHAR_ESCAPE)
		return checkLineEscape(it);
	else if (*it == CHAR_COMMENT)
		return checkComment(it);
	return false;
}

bool webss::checkLineEscape(SmartIterator& it)
{
	//'\\' + eof
	if (it.peekEnd())
	{
		++it;
		return true;
	}
	
	char c = it.peek();
	if (c != '\n' && !isWhitespace(c))
		return false;
	
	//'\\' + '\n' or whitespace; whitespace must be skipped until '\n' or end of buffer
	while (++it && isWhitespace(*it))
		;
	if (!it)
		return true;
	if (*it != '\n')
		throw runtime_error(ERROR_UNEXPECTED);
	skipWhitespace(++it);
	return true;
}

bool webss::checkComment(SmartIterator& it)
{
	if (it.peekEnd())
		return false;
	
	char c = it.peek();
	if (c == CHAR_COMMENT)
		skipLine(it.readTwo());
	else if (c == '*')
		skipMultilineComment(it.readTwo());
	else
		return false;
	return true;
}

SmartIterator& webss::skipJunkToValid(SmartIterator& it)
{
	if (!skipJunk(it))
		throw runtime_error(ERROR_EXPECTED);
	return it;
}

SmartIterator& webss::skipJunkToValidCondition(SmartIterator& it, function<bool()> condition)
{
	skipJunkToValid(it);
	if (!condition())
		throw runtime_error(ERROR_UNEXPECTED);
	return it;
}

void webss::cleanLine(SmartIterator& it, ConType con, Language lang)
{
	if (!skipWhitespace(it) || isLineEnd(*it, con, lang))
		return;
	throw runtime_error(ERROR_UNEXPECTED);
}