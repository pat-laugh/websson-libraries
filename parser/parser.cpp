//MIT License
//Copyright 2017-2019 Patrick Laughrea
#include "parser.hpp"

#include <cassert>

#include "errors.hpp"
#include "utils/constants.hpp"

using namespace std;
using namespace various;
using namespace webss;

static void addEntities(EntityManager& ents, vector<const char*> names, Entity ent)
{
	for (const char* name : names)
		ents.addAlias(name, ent);
}

static void addEntityKeywords(EntityManager& ents)
{
	addEntities(ents, { "N", "Nil", "None", "Null", "nil", "none", "null", "F", "False", "false", "T", "True", "true" }, Entity("", Webss()));
}

static void addEntitySubstitutes(EntityManager& ents)
{
	ents.add(string() + CHAR_SUBSTITUTION + CHAR_FOREACH_SUBST_PARAM, Webss(nullptr, WebssType::FOREACH_PARAM));
}

static TheadBin makeTheadBinKeyword(Keyword keyword)
{
	using Bhead = ParamBin::SizeHead;
	using Barray = ParamBin::SizeArray;
	TheadBin thead;
	thead.attach("", ParamBin(Bhead(keyword), Barray(Barray::Type::ONE)));
	return thead;
}

static void addTheadBinEntityKeywords(EntityManager& ents, vector<const char*> names, Keyword keyword)
{
	string entName(names[0]);
	addEntities(ents, move(names), Entity(move(entName), Thead(makeTheadBinKeyword(keyword))));
}

static void initEnts(EntityManager& ents)
{
	addEntityKeywords(ents);
	addEntitySubstitutes(ents);
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

Parser::Parser(string filename) : tagit(SmartIterator("")), filename(filename), docId(move(filename)) {}
Parser::Parser(SmartIterator it, string filename) : tagit(move(it)), filename(filename), docId(move(filename)) { initEnts(ents); }
Parser::Parser(const istream& in, string filename) : tagit(SmartIterator("")), filename(filename), docId(move(filename))
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
	this->filename = filename;
	docId = move(filename);
	return *this;
}

Parser& Parser::addEntity(string name, Webss value)
{
	ents.add(move(name), move(value));
	return *this;
}

bool Parser::containerEmpty()
{
	switch (tagit.getSafe())
	{
	case Tag::NONE:
		if (con.hasEndChar())
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_EXPECTED));
		return true;
	case Tag::UNKNOWN:
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
	case Tag::SEPARATOR:
		if (!allowVoid)
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_VOID));
		break;
	case Tag::END_DICTIONARY: case Tag::END_LIST: case Tag::END_TUPLE: case Tag::END_TEMPLATE:
		if (!con.isEnd(*getIt()))
			break;
		++tagit;
		return true;
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
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_EXPECTED));
		return false;
	case Tag::UNKNOWN:
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
	case Tag::SEPARATOR:
		switch (++tagit)
		{
		case Tag::NONE:
			if (con.hasEndChar())
				throw runtime_error(WEBSSON_EXCEPTION(ERROR_EXPECTED));
			return false;
		case Tag::UNKNOWN:
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
		case Tag::SEPARATOR:
			if (!allowVoid)
				throw runtime_error(WEBSSON_EXCEPTION(ERROR_VOID));
			break;
		case Tag::END_DICTIONARY: case Tag::END_LIST: case Tag::END_TUPLE: case Tag::END_TEMPLATE:
			if (!con.isEnd(*getIt()))
				break;
			++tagit;
			return false;
		default:
			break;
		}
		break;
	case Tag::END_DICTIONARY: case Tag::END_LIST: case Tag::END_TUPLE: case Tag::END_TEMPLATE:
		if (!con.isEnd(*getIt()))
			break;
		++tagit;
		return false;
	default:
		break;
	}
	return true;
}