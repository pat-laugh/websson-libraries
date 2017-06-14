//MIT License
//Copyright 2017 Patrick Laughrea
#include "tagIterator.hpp"

#include "errors.hpp"
#include "utilsSweepers.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

TagIterator::TagIterator(SmartIterator it) : it(move(it)) { getTag(); }

TagIterator& TagIterator::operator=(TagIterator&& o)
{
	it = move(o.it);
	tag = o.tag;
	valid = o.valid;
	return *this;
}

Tag TagIterator::operator++()
{
	++it;
	return getTag();
}

Tag TagIterator::operator*()
{
	return tag;
}

Tag TagIterator::incSafe()
{
	if (valid)
		++it;
	return getTag();
}

Tag TagIterator::getSafe()
{
	if (!valid || !it || isJunk(*it) || checkJunkOperators(it))
		return getTag();
	return tag;
}

Tag getTagColon(SmartIterator& it)
{
	if (it.peekEnd() || it.peek() != CHAR_COLON)
		return Tag::LINE_STRING;
	switch (*skipJunkToValid(it.incTwo()))
	{
	case OPEN_LIST:
		return Tag::TEXT_LIST;
	case OPEN_TUPLE:
		return Tag::TEXT_TUPLE;
	case OPEN_TEMPLATE:
		return Tag::TEXT_TEMPLATE;
	default:
		return Tag::TEXT_DICTIONARY;
	}
}

Tag TagIterator::getTag()
{
	valid = true;
	if (!it)
		return (tag = Tag::NONE);
	switch (*it)
	{
	case CHAR_CSTRING: return (tag = Tag::C_STRING);
	case CHAR_COLON: return (tag = getTagColon(it));
	case CHAR_EQUAL: return (tag = Tag::EQUAL);
	case OPEN_DICTIONARY: return (tag = Tag::START_DICTIONARY);
	case OPEN_LIST: return (tag = Tag::START_LIST);
	case OPEN_TUPLE: return (tag = Tag::START_TUPLE);
	case OPEN_TEMPLATE: return (tag = Tag::START_TEMPLATE);
	case CLOSE_DICTIONARY: return (tag = Tag::END_DICTIONARY);
	case CLOSE_LIST: return (tag = Tag::END_LIST);
	case CLOSE_TUPLE: return (tag = Tag::END_TUPLE);
	case CLOSE_TEMPLATE: return (tag = Tag::END_TEMPLATE);
	case CHAR_ABSTRACT_ENTITY: return (tag = Tag::ENTITY_ABSTRACT);
	case CHAR_CONCRETE_ENTITY: return (tag = Tag::ENTITY_CONCRETE);
	case CHAR_EXPAND: return (tag = Tag::EXPAND);
	case CHAR_SCOPED_IMPORT: return (tag = Tag::SCOPED_IMPORT);
	case CHAR_IMPORT: return (tag = Tag::IMPORT);
	case CHAR_OPTION: return (tag = Tag::OPTION);
	case CHAR_SELF: return (tag = Tag::SELF);
	case CHAR_SEPARATOR: return (tag = Tag::SEPARATOR);
	case CHAR_EXPLICIT_NAME: return (tag = Tag::EXPLICIT_NAME);
	case CHAR_SCOPE: return (tag = Tag::SCOPE);

		//junk
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
	case 0x20: case 0x7f:
		skipJunk(++it);
		return getTag();

		//junk operators
	case '/':
		return checkJunkOperators(it) ? getTag() : (tag = Tag::SLASH);

		//name start
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M':
	case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
	case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		return (tag = Tag::NAME_START);
	default:
		return (tag = (unsigned char)*it > 127 ? Tag::NAME_START : Tag::UNKNOWN);

		//number start
	case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		return (tag = Tag::DIGIT);
	case '-':
		return (tag = Tag::MINUS);
	case '+':
		return (tag = Tag::PLUS);
	}
}

TagIterator& TagIterator::getToTag(Tag tag)
{
	if (getSafe() == tag)
		return *this;
	throw runtime_error(it ? ERROR_UNEXPECTED : ERROR_EXPECTED);
}

SmartIterator& TagIterator::getIt()
{
	return it;
}

SmartIterator& TagIterator::getItSafe()
{
	valid = false;
	return it;
}

void TagIterator::setIterator(SmartIterator it)
{
	it = move(it);
	getTag();
}