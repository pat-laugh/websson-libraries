//MIT License
//Copyright 2017 Patrick Laughrea
#include "webssString.hpp"

#include <cassert>
#include <exception>

#include "utils.hpp"
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
	case StringType::NONE:
		break;
#ifdef COMPILE_WEBSS
	case StringType::FUNC_FLUSH: case StringType::FUNC_NEWLINE:
		break;
	case StringType::WEBSS:
		delete webss;
		break;
	case StringType::ENT_DYNAMIC:
#endif
	case StringType::ENT_STATIC:
		ent.~Entity();
		break;
	case StringType::STRING:
		tString.~string();
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
		break;
	case StringType::WEBSS:
		webss = o.webss;
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
		break;
	case StringType::WEBSS:
		webss = new Webss(*o.webss);
		break;
	case StringType::ENT_DYNAMIC:
#endif
	case StringType::ENT_STATIC:
		new (&ent) Entity(o.ent);
		break;
	case StringType::STRING:
		new (&tString) string(o.tString);
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
		return true;
	case StringType::WEBSS:
		return *webss == *o.webss;
	case StringType::ENT_DYNAMIC:
#endif
	case StringType::ENT_STATIC:
		return ent.getContent() == o.ent.getContent();
	case StringType::STRING:
		return tString == o.tString;
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

#ifdef COMPILE_WEBSS
const Webss& StringItem::getWebssRaw() const { assert(type == StringType::WEBSS); return *webss; }
#endif


WebssString::WebssString(WebssString&& o) : items(move(o.items)), ptr(move(o.ptr)) {}
WebssString::WebssString(const WebssString& o) : items(o.items), ptr(o.ptr == nullptr ? nullptr : new string(*o.ptr)) {}

void WebssString::push(StringItem item) { items.push_back(move(item)); }

const string& WebssString::getString() const
{
	string sb;
	for (const auto& item : items)
		switch (item.getTypeRaw())
		{
		default: assert(false);
#ifdef COMPILE_WEBSS
		case StringType::FUNC_NEWLINE: case StringType::FUNC_FLUSH:
			throw runtime_error("can't put function chars into a raw string");
		case StringType::WEBSS:
			sb += item.getWebssRaw().getString();
			break;
		case StringType::ENT_DYNAMIC:
#endif
		case StringType::ENT_STATIC:
			sb += item.getEntityRaw().getContent().getString();
			break;
		case StringType::STRING:
			sb += item.getStringRaw();
			break;
		}
	unique_ptr<string>& refPtr = *const_cast<unique_ptr<string>*>(&ptr);
	refPtr = unique_ptr<string>(new string(sb));
	return *ptr;
}


bool WebssString::operator==(const WebssString& o) const
{
	if (this == &o)
		return true;
	return items == o.items; //don't compare pointers
}
bool WebssString::operator!=(const WebssString& o) const { return !(*this == o); }