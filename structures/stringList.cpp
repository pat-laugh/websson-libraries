//MIT License
//Copyright 2018 Patrick Laughrea
#include "stringList.hpp"

#include <cassert>
#include <exception>

#include "utils.hpp"
#include "webss.hpp"
#include "various/stringBuilder.hpp"

using namespace std;
using namespace various;
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

StringItem::StringItem(string&& s) : type(StringType::STRING), tString(move(s)) {}
StringItem::StringItem(Webss webss) : type(StringType::WEBSS), webss(new Webss(move(webss))) {}

#ifndef COMPILE_WEBSS
StringItem::StringItem(const Entity& ent) : type(StringType::ENT_STATIC), ent(ent) {}
#else
StringItem::StringItem(StringType t) : type(t) {}
StringItem::StringItem(const Entity& ent) : type(StringType::ENT_STATIC), ent(Entity(ent.getName(), ent.getContent().getWebssLast())) {}
StringItem::StringItem(const Entity& ent, bool) : type(StringType::ENT_DYNAMIC), ent(ent) {}
#endif

void StringItem::destroyUnion()
{
	switch (type)
	{
	default: assert(false);
	case StringType::NONE:
		break;
#ifdef COMPILE_WEBSS
	case StringType::FUNC_FLUSH: case StringType::FUNC_NEWLINE:
	case StringType::FUNC_NEWLINE_FLUSH:
		break;
	case StringType::ENT_DYNAMIC:
#endif
	case StringType::ENT_STATIC:
		ent.~Entity();
		break;
	case StringType::STRING:
		tString.~string();
		break;
	case StringType::WEBSS:
		delete webss;
		break;
	}
	type = StringType::NONE;
}

void StringItem::copyUnion(StringItem&& o)
{
	switch (o.type)
	{
	default: assert(false);
	case StringType::NONE:
		break;
#ifdef COMPILE_WEBSS
	case StringType::FUNC_FLUSH: case StringType::FUNC_NEWLINE:
	case StringType::FUNC_NEWLINE_FLUSH:
		break;
	case StringType::ENT_DYNAMIC:
#endif
	case StringType::ENT_STATIC:
		new (&ent) Entity(move(o.ent));
		o.ent.~Entity();
		break;
	case StringType::STRING:
		new (&tString) string(move(o.tString));
		o.tString.~string();
		break;
	case StringType::WEBSS:
		webss = o.webss;
		break;
	}
	type = o.type;
	o.type = StringType::NONE;
}

void StringItem::copyUnion(const StringItem& o)
{
	switch (o.type)
	{
	default: assert(false);
	case StringType::NONE:
		break;
#ifdef COMPILE_WEBSS
	case StringType::FUNC_FLUSH: case StringType::FUNC_NEWLINE:
	case StringType::FUNC_NEWLINE_FLUSH:
		break;
	case StringType::ENT_DYNAMIC:
#endif
	case StringType::ENT_STATIC:
		new (&ent) Entity(o.ent);
		break;
	case StringType::STRING:
		new (&tString) string(o.tString);
		break;
	case StringType::WEBSS:
		webss = new Webss(*o.webss);
		break;
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
	case StringType::NONE:
		return true;
#ifdef COMPILE_WEBSS
	case StringType::FUNC_FLUSH: case StringType::FUNC_NEWLINE:
	case StringType::FUNC_NEWLINE_FLUSH:
		return true;
	case StringType::ENT_DYNAMIC:
#endif
	case StringType::ENT_STATIC:
		return ent.getContent() == o.ent.getContent();
	case StringType::STRING:
		return tString == o.tString;
	case StringType::WEBSS:
		return *webss == *o.webss;
	}
}
bool StringItem::operator!=(const StringItem& o) const { return !(*this == o); }

StringType StringItem::getTypeRaw() const { return type; }

const std::string& StringItem::getStringRaw() const { assert(type == StringType::STRING); return tString; }

const Entity& StringItem::getEntityRaw() const
{
	assert(type == StringType::ENT_STATIC
#ifdef COMPILE_WEBSS
		|| type == StringType::ENT_DYNAMIC
#endif
	);
	return ent;
}

const Webss& StringItem::getWebssRaw() const { assert(type == StringType::WEBSS); return *webss; }

StringList::StringList() {}
StringList::StringList(StringList&& o) : items(move(o.items)) {}
StringList::StringList(const StringList& o) : items(o.items) {}

void StringList::push(StringItem item) { items.push_back(move(item)); }
void StringList::push(const StringList& sl)
{
	for (const auto& item : sl.items)
		push(item);
}

static void substituteString(StringBuilder& sb, const Webss& webss)
{
	switch (webss.getType())
	{
	case WebssType::PRIMITIVE_STRING:
		sb += webss.getString();
		break;
	case WebssType::STRING_LIST:
		sb += webss.getStringList().concat();
		break;
#ifdef COMPILE_WEBSS
		//allow other stuff...
#endif
	default:
		throw runtime_error("string substitution must evaluate to string");
	}
}

string StringList::concat() const
{
	StringBuilder sb;
	for (const auto& item : items)
		switch (item.getTypeRaw())
		{
		default: assert(false);
#ifdef COMPILE_WEBSS
		case StringType::FUNC_NEWLINE: case StringType::FUNC_FLUSH:
		case StringType::FUNC_NEWLINE_FLUSH:
			throw runtime_error("can't put function chars into a raw string");
		case StringType::ENT_DYNAMIC:
#endif
		case StringType::ENT_STATIC:
			substituteString(sb, item.getEntityRaw().getContent());
			break;
		case StringType::STRING:
			sb += item.getStringRaw();
			break;
		case StringType::WEBSS:
			substituteString(sb, item.getWebssRaw());
			break;
		}
	return sb;
}

const std::vector<StringItem>& StringList::getItems() const { return items; }

bool StringList::operator==(const StringList& o) const
{
	if (this == &o)
		return true;
	return items == o.items;
}
bool StringList::operator!=(const StringList& o) const { return !(*this == o); }