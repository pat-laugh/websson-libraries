//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

using namespace std;
using namespace webss;

//HOW IT WORKS: starting whitespace is skipped, then stuff is read until line end
//everytime a \s or \e is met, the minimum length of the output is set; this is because
//\s output a space and \e nothing; if followed by only whitespace, the space and whitespace
//before should not be removed
//whitespace is then removed from the output until a non-whitespace charater is found or
//the output has reached minimum length
string Parser::parseLineString(It& it, ConType con)
{
	skipWhitespace(it);
	if (!it)
		return "";

	string line;
	auto minLength = line.length();
	do
	{
	switchStart:
		if (isLineEnd(*it, con, language))
			break;

		switch (*it)
		{
		case CHAR_COMMENT:
			if (!(++it))
				return line + CHAR_COMMENT;
			if (*it == CHAR_COMMENT) //is comment
			{
				skipLine(++it);
				goto loopEnd;
			}
			else if (*it == '*')
			{
				skipMultilineComment(++it);
				continue;
			}
			line += CHAR_COMMENT;
			goto switchStart;
		case CHAR_CONCRETE_ENTITY:
			if (!(++it))
				return line + CHAR_CONCRETE_ENTITY;
			if (!checkVariableString(it, line)) //if variable, iterator has to be checked
				goto switchStart;
			continue;
		case '\\':
			checkEscapedChar(it, line, [&]() { minLength = line.length() + 1; });
			continue;
		default:
			break;
		}
		if (!isWhitespace(*it) || *it == ' ')
			line += *it;
		++it;
	} while (it);
loopEnd:
	if (!line.empty())
		while (line.back() == ' ' && line.length() > minLength)
			line.pop_back();

	return line;
}

string Parser::parseDictionaryText(It& it)
{
	if (*skipJunkToValid(it) == CLOSE_DICTIONARY)
		return "";

	string text;
	int countStartEnd = 1; //count of dictionary start - dictionary end
	bool addSpace = false; //== false if last char was \e or \s
loopStart:
	text += parseLineStringTextDictionary(it, countStartEnd, addSpace); //function throws errors if it is end or dictionary not closed
	if (countStartEnd == 0)
	{
		++it;
		return text;
	}
	if (addSpace)
		text += ' ';
	goto loopStart;
}

//RQUIREMENT: FOR USE BY TEXT DICTIONARY ONLY
//HOW IT WORKS: exactly like parseLineString with the addition that addSpace is set
//to false if the last character (excluding trailing whitespace) is \s or \e, else false
//addSpace tells the calling function whether or not it should add a space after the line
//countStartEnd maintains the count of dictionary start - dictionary end chars
//if the end of the line is reached, then junk is skipped and function returns
//ready to start reading at the new line
//it checks if last dictionary end lies on an empty line, and if so returns for a final time
string Parser::parseLineStringTextDictionary(It& it, int& countStartEnd, bool& addSpace)
{
	static const char NOT_CLOSED[] = "dictionary is not closed";
	string line;
	string::size_type minLength = 0;
	do
	{
	switchStart:
		switch (*it) //it is initially checked by caller
		{
		case CHAR_COMMENT:
			if (!(++it))
				throw runtime_error(NOT_CLOSED);
			if (*it == CHAR_COMMENT)
			{
				skipLine(++it);
				++it;
				goto endLoop;
			}
			else if (*it == '*')
			{
				skipMultilineComment(++it);
				continue;
			}
			line += CHAR_COMMENT;
			addSpace = true;
			goto switchStart;
		case CHAR_CONCRETE_ENTITY:
		{
			if (!(++it))
				throw runtime_error(NOT_CLOSED);
			bool isVar = checkVariableString(it, line); //if variable, iterator has to be checked
			minLength = line.length();
			addSpace = true;
			if (!isVar)
				goto switchStart;
			continue;
		}
		case '\\':
			addSpace = true;
			checkEscapedChar(it, line, [&]() { minLength = line.length() + 1; addSpace = false; });
			continue;
		case '\n':
			++it;
			goto endLoop;
		case OPEN_DICTIONARY:
			++countStartEnd;
			break;
		case CLOSE_DICTIONARY:
			if (countStartEnd == 1)
				goto endLoop;
			--countStartEnd;
		default:
			break;
		}
		if (!isWhitespace(*it))
		{
			line += *it;
			addSpace = true;
		}
		else if (*it == ' ')
			line += *it;
		++it;
	} while (it);
endLoop:
	bool addNewLine = skipWhitespace(it) == '\n';
	if (addNewLine)
		skipJunkToValid(++it);

	while (line.back() == ' ' && line.length() > minLength)
		line.pop_back();

	if (*it == CLOSE_DICTIONARY && countStartEnd == 1)
	{
		countStartEnd = 0;
		return line;
	}

	if (addNewLine)
	{
		addSpace = false;
		line += '\n';
	}

	return line;
}

string Parser::parseCString(It& it)
{
	static const char NOT_CLOSED[] = "string is not closed";
	if (!it)
		throw runtime_error(NOT_CLOSED);

	string line;
	do
	{
	switchStart:
		switch (*it)
		{
		case CHAR_CONCRETE_ENTITY:
			if (!(++it))
				throw runtime_error(NOT_CLOSED);
			if (!checkVariableString(it, line))
				goto switchStart;
			continue;
		case '\\':
			checkEscapedChar(it, line, []() {});
			continue;
		case CHAR_CSTRING:
			++it;
			return line;
		default:
			break;
		}
		if (isJunk(*it))
		{
			if (*it == '\n')
				throw runtime_error("can't have line break in cstring");
			else if (*it == ' ') //other junk is ignored
				line += ' ';
		}
		else
			line += *it;
		++it;
	} while (it);
	throw runtime_error(NOT_CLOSED);
}