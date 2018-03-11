//MIT License
//Copyright 2017 Patrick Laughrea
#include "webssString.hpp"

#include <cassert>

#include "webss.hpp"

using namespace std;
using namespace webss;

StringItem::StringItem() {}
StringItem::~StringItem() { destroyUnion(); }

StringItem::StringItem(StringItem&& o) { copyUnion(move(o)); }
StringItem::StringItem(const StringItem& o) { copyUnion(o); }

StringItem& StringItem::operator=(StringItem o)
{
	destroyUnion();
	copyUnion(move(o));
	return *this;
}

StringItem::StringItem(StringType t) : type(t) {}
StringItem::StringItem(string&& s) : type(StringType::STRING), tString(move(s)) {}

#ifndef COMPILE_WEBSS
StringItem::StringItem(const Entity& ent) : type(StringType::ENT_STATIC), ent(ent) {}
#else
StringItem::StringItem(const Entity& ent) : type(StringType::ENT_STATIC), ent(Entity(ent.getName(), ent.getContent().getWebssLast())) {}
StringItem::StringItem(const Entity& ent, bool) : type(StringType::ENT_DYNAMIC), ent(ent) {}
StringItem::StringItem(Webss webss) : type(StringType::WEBSS), webss(new Webss(move(webss))) {}
#endif

void StringItem::destroyUnion()
{
	switch (type)
	{
	default: assert(false);
	case StringType::NONE: case StringType::FUNC_NEWLINE:
#ifdef COMPILE_WEBSS
	case StringType::FUNC_FLUSH:
#endif
		break;
	case StringType::ENT_STATIC:
#ifdef COMPILE_WEBSS
	case StringType::ENT_DYNAMIC:
#endif
		ent.~Entity();
		break;
	case StringType::STRING:
		tString.~string();
		break;
#ifdef COMPILE_WEBSS
	case StringType::WEBSS:
		delete webss;
		break;
#endif
	}
	type = StringType::NONE;
}

void StringItem::copyUnion(StringItem&& o)
{
	switch (o.type)
	{
	default: assert(false);
	case StringType::NONE: case StringType::FUNC_NEWLINE:
#ifdef COMPILE_WEBSS
	case StringType::FUNC_FLUSH:
#endif
		break;
	case StringType::ENT_STATIC:
#ifdef COMPILE_WEBSS
	case StringType::ENT_DYNAMIC:
#endif
		new (&ent) Entity(move(o.ent));
		o.ent.~Entity();
		break;
	case StringType::STRING:
		new (&tString) string(move(o.tString));
		o.tString.~string();
		break;
#ifdef COMPILE_WEBSS
	case StringType::WEBSS:
		webss = o.webss;
		break;
#endif
	}
	type = o.type;
	o.type = StringType::NONE;
}

void StringItem::copyUnion(const StringItem& o)
{
	switch (o.type)
	{
	default: assert(false);
	case StringType::NONE: case StringType::FUNC_NEWLINE:
#ifdef COMPILE_WEBSS
	case StringType::FUNC_FLUSH:
#endif
		break;
	case StringType::ENT_STATIC:
#ifdef COMPILE_WEBSS
	case StringType::ENT_DYNAMIC:
#endif
		new (&ent) Entity(o.ent);
		break;
	case StringType::STRING:
		new (&tString) string(o.tString);
		break;
#ifdef COMPILE_WEBSS
	case StringType::WEBSS:
		webss = new Webss(*o.webss);
		break;
#endif
	}
	type = o.type;
}

bool StringItem::operator==(const StringItem& o) const
{
	if (this == &o)
		return true;
	if (type != o.type)
		return false;
	switch (type)
	{
	default: assert(false);
	case StringType::NONE: case StringType::FUNC_NEWLINE:
#ifdef COMPILE_WEBSS
	case StringType::FUNC_FLUSH:
#endif
		return true;
	case StringType::ENT_STATIC:
#ifdef COMPILE_WEBSS
	case StringType::ENT_DYNAMIC:
#endif
		return ent.getContent() == o.ent.getContent();
	case StringType::STRING:
		return tString == o.tString;
#ifdef COMPILE_WEBSS
	case StringType::WEBSS:
		return *webss == *o.webss;
#endif
	}
}
bool StringItem::operator!=(const StringItem& o) const { return !(*this == o); }

StringType StringItem::getTypeRaw() { return type; }

const std::string& StringItem::getStringRaw() { assert(type == StringType::STRING); return tString; }

const Entity& StringItem::getEntityRaw()
{
	assert(type == StringType::ENT_STATIC
#ifdef COMPILE_WEBSS
		|| type == StringType::ENT_DYNAMIC
#endif
	);
	return ent;
}

#ifdef COMPILE_WEBSS
const Webss& StringItem::getWebssRaw() { assert(type == StringType::WEBSS); return *webss; }
#endif