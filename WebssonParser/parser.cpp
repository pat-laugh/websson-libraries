//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include <cassert>

#include "WebssonUtils/constants.h"
#include "errors.h"

using namespace std;
using namespace webss;

TemplateHeadBinary makeTheadBinaryKeyword(Keyword keyword)
{
	using Bhead = ParamBinary::SizeHead;
	using Blist = ParamBinary::SizeList;
	TemplateHeadBinary thead;
	thead.attach("", ParamBinary(Bhead(keyword), Blist(Blist::Type::ONE)));
	return thead;
}

void addTheadBinaryEntityKeywords(BasicEntityManager<Webss> ents, vector<string> names, Keyword keyword)
{
	Entity ent(string(names[0]), makeTheadBinaryKeyword(keyword));
	for (const auto& name : names)
		ents.addGlobalEntity(name, ent);
}

void Parser::initEnts()
{
	addTheadBinaryEntityKeywords(ents, { "B", "Bool", "bool" }, Keyword::BOOL);
	addTheadBinaryEntityKeywords(ents, { "byte", "Byte" }, Keyword::INT8);
	addTheadBinaryEntityKeywords(ents, { "short", "Short" }, Keyword::INT16);
	addTheadBinaryEntityKeywords(ents, { "I", "Int", "int" }, Keyword::INT32);
	addTheadBinaryEntityKeywords(ents, { "L", "Long", "long" }, Keyword::INT64);
	addTheadBinaryEntityKeywords(ents, { "float", "Float" }, Keyword::FLOAT);
	addTheadBinaryEntityKeywords(ents, { "D", "Double", "double" }, Keyword::DOUBLE);
	addTheadBinaryEntityKeywords(ents, { "S", "String", "string" }, Keyword::STRING);
}

Parser::Parser() : it(SmartIterator(string(""))) { initEnts(); }
Parser::Parser(SmartIterator&& it) : it(move(it)) { initEnts(); }
Parser::Parser(const std::istream& in) : it(SmartIterator(in)) { initEnts(); }
Parser::Parser(const std::stringstream& in) : it(SmartIterator(in)) { initEnts(); }
Parser::Parser(const std::string& in) : it(SmartIterator(in)) { initEnts(); }

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

const char* ERROR_VOID = "can't have void element";

bool Parser::containerEmpty()
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

bool Parser::checkNextElement()
{
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