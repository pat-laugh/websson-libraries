//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include <cassert>

#include "WebssonUtils/constants.h"
#include "errors.h"

using namespace std;
using namespace webss;

GlobalParser::GlobalParser() : it(SmartIterator(string(""))) {}
GlobalParser::GlobalParser(SmartIterator&& it) : it(move(it)) {}
GlobalParser::GlobalParser(const std::istream& in) : it(SmartIterator(in)) {}
GlobalParser::GlobalParser(const std::stringstream& in) : it(SmartIterator(in)) {}
GlobalParser::GlobalParser(const std::string& in) : it(SmartIterator(in)) {}

GlobalParser& GlobalParser::setIterator(SmartIterator&& it)
{
	this->it = move(it);
	return *this;
}

GlobalParser& GlobalParser::addEntity(string&& name, Webss&& value)
{
	ents.addGlobal(move(name), move(value));
	return *this;
}

Document GlobalParser::parse() { return GlobalParser::Parser::parseDocument(*this); }

GlobalParser::Parser::Parser(Parser& parser, ConType con, bool allowVoid)
	: ents(parser.ents), importedDocuments(parser.importedDocuments), it(parser.it)
	, con(con), allowVoid(allowVoid)
{
	multiLineContainer = checkLineEmpty(it);
}

const char* ERROR_VOID_2 = "can't have void element"; //to avoid linker error

bool GlobalParser::Parser::parserContainerEmpty()
{
	switch (nextElem = getTag(it))
	{
	case Tag::NONE:
		if (con.hasEndChar())
			throw runtime_error(ERROR_EXPECTED);
		return true;
	case Tag::UNKNOWN:
		if (*it != CHAR_SEPARATOR)
			throw runtime_error(ERROR_UNEXPECTED);
		if (!allowVoid)
			throw runtime_error(ERROR_VOID_2);
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

bool GlobalParser::Parser::parserCheckNextElement()
{
	if (!lineGreed)
		cleanLine(it, con);
	else
		lineGreed = false;

	switch (nextElem = getTag(it))
	{
	case Tag::NONE:
		if (con.hasEndChar())
			throw runtime_error(ERROR_EXPECTED);
		return false;
	case Tag::UNKNOWN:
		if (*it != CHAR_SEPARATOR)
			throw runtime_error(ERROR_UNEXPECTED);
		switch (nextElem = getTag(++it))
		{
		case Tag::NONE:
			if (con.hasEndChar())
				throw runtime_error(ERROR_EXPECTED);
			return false;
		case Tag::UNKNOWN:
			if (*it != CHAR_SEPARATOR)
				throw runtime_error(ERROR_UNEXPECTED);
			if (!allowVoid)
				throw runtime_error(ERROR_VOID_2);
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