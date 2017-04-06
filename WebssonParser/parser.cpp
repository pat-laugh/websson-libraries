//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include <cassert>

#include "WebssonUtils/constants.h"
#include "errors.h"

using namespace std;
using namespace webss;

Parser::Parser() : it(SmartIterator(string(""))) {}
Parser::Parser(SmartIterator&& it) : it(move(it)) {}
Parser::Parser(const std::istream& in) : it(SmartIterator(in)) {}
Parser::Parser(const std::stringstream& in) : it(SmartIterator(in)) {}
Parser::Parser(const std::string& in) : it(SmartIterator(in)) {}

Parser& Parser::setIterator(SmartIterator&& it)
{
	this->it = move(it);
	return *this;
}

Parser& Parser::addEntity(string&& name, Webss&& value)
{
	ents.addGlobal(move(name), move(value));
	return *this;
}

const char* ERROR_VOID = "can't have void element"; //to avoid linker error

bool Parser::parserContainerEmpty()
{
	switch (nextTag = getTag(it))
	{
	case Tag::NONE:
		if (con.hasEndChar())
			throw runtime_error(ERROR_EXPECTED);
		return true;
	case Tag::UNKNOWN:
		throw runtime_error(ERROR_UNEXPECTED);
	case Tag::SEPARATOR:
		if (!allowVoid)
			throw runtime_error(ERROR_VOID);
		break;
	case Tag::END_DICTIONARY: case Tag::END_LIST: case Tag::END_TUPLE: case Tag::END_TEMPLATE:
		if (con.isEnd(*it))
		{
			++it;
			return true;
		}
		break;
	default:
		break;
	}
	return false;
}

bool Parser::parserCheckNextElement()
{
	if (!lineGreed)
		cleanLine(it, con);
	else
		lineGreed = false;

	switch (nextTag = getTag(it))
	{
	case Tag::NONE:
		if (con.hasEndChar())
			throw runtime_error(ERROR_EXPECTED);
		return false;
	case Tag::UNKNOWN:
		throw runtime_error(ERROR_UNEXPECTED);
	case Tag::SEPARATOR:
		switch (nextTag = getTag(++it))
		{
		case Tag::NONE:
			if (con.hasEndChar())
				throw runtime_error(ERROR_EXPECTED);
			return false;
		case Tag::UNKNOWN:
			throw runtime_error(ERROR_UNEXPECTED);
		case Tag::SEPARATOR:
			if (!allowVoid)
				throw runtime_error(ERROR_VOID);
			break;
		case Tag::END_DICTIONARY: case Tag::END_LIST: case Tag::END_TUPLE: case Tag::END_TEMPLATE:
			if (con.isEnd(*it))
			{
				++it;
				return false;
			}
			break;
		default:
			break;
		}
		break;
	case Tag::END_DICTIONARY: case Tag::END_LIST: case Tag::END_TUPLE: case Tag::END_TEMPLATE:
		if (con.isEnd(*it))
		{
			++it;
			return false;
		}
		break;
	default:
		break;
	}
	return true;
}