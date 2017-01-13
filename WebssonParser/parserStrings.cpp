//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "unicode.h"

using namespace std;
using namespace webss;

void addEscapedChar(string& str, char c);

//HOW IT WORKS: starting whitespace is skipped, then stuff is read until line end
//everytime a \s or \e is met, the minimum length of the output is set; this is because
//\s output a space and \e nothing; if followed by only whitespace, the space and whitespace
//before should not be removed
//whitespace is then removed from the output until a non-whitespace charater is found or
//the output has reached minimum length
string Parser::parseLineString(It& it, ConType con)
{
	skipWhitespace(it);

	string line;
	auto minLength = line.length();
	while (it)
	{
	switchStart:
		if (isLineEnd(*it, con, language))
			break;

		switch (*it)
		{
		case CHAR_COMMENT:
			if (checkComment(it))
				continue;
			break;
		case CHAR_CONCRETE_ENTITY: //TO IMPROVE
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
		if (!isControlAscii(*it))
			line += *it;
		++it;
	};
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
			if (checkComment(it))
				continue;
			break;
		case CHAR_CONCRETE_ENTITY: //TO IMPROVE
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
		if (!isControlAscii(*it))
		{
			line += *it;
			if (*it != ' ')
				addSpace = true;
		}
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

const char NOT_CLOSED[] = "string is not closed";
string Parser::parseCString(It& it)
{
	string line;
	while (it)
	{
	switchStart:
		switch (*it)
		{
		case CHAR_CSTRING:
			++it;
			return line;
		case CHAR_CONCRETE_ENTITY: //TO IMPROVE
			if (!(++it))
				throw runtime_error(NOT_CLOSED);
			if (!checkVariableString(it, line))
				goto switchStart;
			continue;
		case '\\':
			checkEscapedChar(it, line, []() {});
			continue;
		case '\n':
			throw runtime_error("can't have line break in cstring");
		default:
			break;
		}
		if (!isControlAscii(*it))
			line += *it;
		++it;
	};
	throw runtime_error(NOT_CLOSED);
}

//adds the char corresponding to an escape; for serialization
//REQUIREMENT: the char must be an escapable char
void addEscapedChar(string& str, char c)
{
	switch (c)
	{
	case '0': str += '\0'; break;
	case 'a': str += '\a'; break;
	case 'b': str += '\b'; break;
	case 'f': str += '\f'; break;
	case 'n': str += '\n'; break;
	case 'r': str += '\r'; break;
	case 't': str += '\t'; break;
	case 'v': str += '\v'; break;

	case 's': str += ' '; //no need for break
	case 'e': break; //empty

	default: //isSpecialAscii, else undefined behavior
		str += c;
	}
}

void Parser::checkEscapedChar(It& it, std::string& line, function<void()> funcIsSENT)
{
	if (checkLineEscape(it))
		return;
	if (!isEscapableChar(*(++it))) //no need to check it is valid since if not valid, would've been a line escape
		throw runtime_error("invalid char escape");

	switch (*(++it))
	{
	case 'x': case 'X': case 'u': case 'U':
		addEscapedHex(it, line, language);
		break;
	case 's': case 'e': case 'n': case 't':
		funcIsSENT();
	default:
		addEscapedChar(line, *it);
		++it;
	}
}

bool Parser::checkVariableString(It& it, string& line)
{
	if (!isNameStart(*it))
	{
		line += CHAR_CONCRETE_ENTITY;
		return false;
	}
	line += parseVariableString(it);
	return true;
}

string Parser::parseVariableString(It& it)
{
	string varName = parseName(it);
	if (!vars.hasVariable(varName))
		throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(varName));

	const Webss* value = &vars[varName].getContent();
	do
	{
		if (it != CHAR_SCOPE)
		{
			if (it == CHAR_COLON && it.peekGood() && isNameBody(it.peek()))
				++it;
			if (!value->isString())
				throw runtime_error(ERROR_VARIABLE_STRING);

			return value->getString();
		}

		if (!(++it) || !isNameStart(*it))
		{
			if (it == CHAR_COLON && it.peekGood() && isNameBody(it.peek()))
				++it;
			if (!value->isString())
				throw runtime_error(ERROR_VARIABLE_STRING);
			return value->getString() + CHAR_SCOPE;
		}
		if (!value->isDictionary())
			throw runtime_error(ERROR_DEREFERENCED_VARIABLE);
		value = &value->getDictionary().at(parseName(it));
	} while (true);
}