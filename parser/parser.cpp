//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include <cassert>

#include "utils/constants.hpp"
#include "errors.hpp"

using namespace std;
using namespace various;
using namespace webss;

static void addPrivateEntities(EntityManager& ents, vector<string> names, Entity ent)
{
	for (const auto& name : names)
		ents.addPrivateEntity(name, ent);
}

static void addEntityKeywords(EntityManager& ents)
{
	addPrivateEntities(ents, { "N", "Nil", "None", "Null", "nil", "none", "null", "F", "False", "false", "T", "True", "true" }, Entity("", Webss()));
}

static TheadBin makeTheadBinKeyword(Keyword keyword)
{
	using Bhead = ParamBin::SizeHead;
	using Barray = ParamBin::SizeArray;
	TheadBin thead;
	thead.attach("", ParamBin(Bhead(keyword), Barray(Barray::Type::ONE)));
	return thead;
}

static void addTheadBinEntityKeywords(EntityManager& ents, vector<string> names, Keyword keyword)
{
	string entName(names[0]);
	addPrivateEntities(ents, move(names), Entity(move(entName), Thead(makeTheadBinKeyword(keyword))));
}

static void initEnts(EntityManager& ents)
{
	addEntityKeywords(ents);
	addTheadBinEntityKeywords(ents, { "B", "Bool", "bool" }, Keyword::BOOL);
	addTheadBinEntityKeywords(ents, { "byte", "Byte" }, Keyword::INT8);
	addTheadBinEntityKeywords(ents, { "short", "Short" }, Keyword::INT16);
	addTheadBinEntityKeywords(ents, { "I", "Int", "int" }, Keyword::INT32);
	addTheadBinEntityKeywords(ents, { "L", "Long", "long" }, Keyword::INT64);
	addTheadBinEntityKeywords(ents, { "float", "Float" }, Keyword::FLOAT);
	addTheadBinEntityKeywords(ents, { "D", "Double", "double" }, Keyword::DOUBLE);
	addTheadBinEntityKeywords(ents, { "S", "String", "string" }, Keyword::STRING);
	addTheadBinEntityKeywords(ents, { "Varint", "varint" }, Keyword::VARINT);
}

Parser::Parser(string filename) : tagit(SmartIterator("")), filename(move(filename)) {}
Parser::Parser(SmartIterator it, string filename) : tagit(move(it)), filename(move(filename)) { initEnts(ents); }
Parser::Parser(const istream& in, string filename) : tagit(SmartIterator("")), filename(move(filename))
{
	stringstream ss;
	ss << in.rdbuf();
	setIterator(SmartIterator(move(ss)));
	initEnts(ents);
}

Parser& Parser::setIterator(SmartIterator it)
{
	tagit.setIterator(move(it));
	return *this;
}

Parser& Parser::setFilename(string filename)
{
	this->filename = move(filename);
	return *this;
}

Parser& Parser::addEntity(string name, Webss value)
{
	ents.addPrivate(move(name), move(value));
	return *this;
}

bool Parser::containerEmpty()
{
	switch (tagit.getSafe())
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
		if (con.isEnd(*getIt()))
		{
			++tagit;
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
	switch (tagit.getSafe())
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
			if (con.isEnd(*getIt()))
			{
				++tagit;
				return false;
			}
			break;
		default:
			break;
		}
		break;
	case Tag::END_DICTIONARY: case Tag::END_LIST: case Tag::END_TUPLE: case Tag::END_TEMPLATE:
		if (con.isEnd(*getIt()))
		{
			++tagit;
			return false;
		}
		break;
	default:
		break;
	}
	return true;
}