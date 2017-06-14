//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include <cassert>

#include "utils/constants.hpp"
#include "errors.hpp"

using namespace std;
using namespace webss;

void addPrivateEntities(EntityManager& ents, vector<string> names, Entity ent)
{
	for (const auto& name : names)
		ents.addPrivateEntity(name, ent);
}

void addEntityKeywords(EntityManager& ents)
{
	addPrivateEntities(ents, { "N", "Nil", "None", "Null", "nil", "none", "null", "F", "False", "false", "T", "True", "true" }, Entity("", Webss()));
}

TemplateHeadBinary makeTheadBinaryKeyword(Keyword keyword)
{
	using Bhead = ParamBinary::SizeHead;
	using Blist = ParamBinary::SizeList;
	TemplateHeadBinary thead;
	thead.attach("", ParamBinary(Bhead(keyword), Blist(Blist::Type::ONE)));
	return thead;
}

void addTheadBinaryEntityKeywords(EntityManager& ents, vector<string> names, Keyword keyword)
{
	string entName(names[0]);
	addPrivateEntities(ents, move(names), Entity(move(entName), makeTheadBinaryKeyword(keyword)));
}

void initEnts(EntityManager& ents)
{
	addEntityKeywords(ents);
	addTheadBinaryEntityKeywords(ents, { "B", "Bool", "bool" }, Keyword::BOOL);
	addTheadBinaryEntityKeywords(ents, { "byte", "Byte" }, Keyword::INT8);
	addTheadBinaryEntityKeywords(ents, { "short", "Short" }, Keyword::INT16);
	addTheadBinaryEntityKeywords(ents, { "I", "Int", "int" }, Keyword::INT32);
	addTheadBinaryEntityKeywords(ents, { "L", "Long", "long" }, Keyword::INT64);
	addTheadBinaryEntityKeywords(ents, { "float", "Float" }, Keyword::FLOAT);
	addTheadBinaryEntityKeywords(ents, { "D", "Double", "double" }, Keyword::DOUBLE);
	addTheadBinaryEntityKeywords(ents, { "S", "String", "string" }, Keyword::STRING);
}

void initAliases(map<string, vector<string>>& aliases)
{
	aliases.insert({ "alias", { "-", "a" } });
}

Parser::Parser(SmartIterator&& it) : it(move(it)), tagit(it) { initEnts(ents); initAliases(aliases); }
Parser::Parser(const istream& in) : it(SmartIterator(in)), tagit(it) { initEnts(ents); initAliases(aliases); }
Parser::Parser(const stringstream& in) : it(SmartIterator(in)), tagit(it) { initEnts(ents); initAliases(aliases); }
Parser::Parser(const string& in) : it(SmartIterator(in)), tagit(it) { initEnts(ents); initAliases(aliases); }

Parser& Parser::setIterator(SmartIterator&& it)
{
	this->it = move(it);
	tagit.setIterator(it);
	tagit.getTag();
	return *this;
}

Parser& Parser::addEntity(string&& name, Webss&& value)
{
	ents.addPrivate(move(name), move(value));
	return *this;
}

const char* ERROR_VOID = "can't have void element";

bool Parser::containerEmpty()
{
	switch (tagit.getTag())
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
			++tagit.getItSafe();
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
	switch (tagit.getTag())
	{
	case Tag::NONE:
		if (con.hasEndChar())
			throw runtime_error(ERROR_EXPECTED);
		return false;
	case Tag::UNKNOWN:
		throw runtime_error(ERROR_UNEXPECTED);
	case Tag::SEPARATOR:
		switch (++tagit)
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
				++tagit.getItSafe();
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
			++tagit.getItSafe();
			return false;
		}
		break;
	default:
		break;
	}
	return true;
}

bool Parser::hasAlias(const std::string& name) { return aliases.find(name) != aliases.end(); }