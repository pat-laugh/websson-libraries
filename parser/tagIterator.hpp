//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "utils/smartIterator.hpp"

#ifdef VOID //in Windows
#undef VOID
#endif

namespace webss
{
	enum class Tag {
		NONE, UNKNOWN,
		NAME_START, DIGIT, MINUS, PLUS, C_STRING, LINE_STRING, EQUAL,
		START_DICTIONARY, START_LIST, START_TUPLE, START_TEMPLATE,
		END_DICTIONARY, END_LIST, END_TUPLE, END_TEMPLATE,
		TEXT_DICTIONARY, TEXT_LIST, TEXT_TUPLE, TEXT_TEMPLATE,
		ENTITY_ABSTRACT, ENTITY_CONCRETE, EXPLICIT_NAME, EXPAND,
		SCOPED_IMPORT, IMPORT, OPTION, SELF, VOID, SEPARATOR, SCOPE, SLASH
	};

	class TagIterator
	{
	public:
		TagIterator(SmartIterator it);

		TagIterator& operator=(TagIterator&& o);
		TagIterator& operator=(const TagIterator& o) = delete;

		//advances the iterator by one then sets a tag
		Tag operator++(); //prefix

		Tag operator*();

		Tag incSafe();
		Tag getSafe();

		//only advances the iterator if it points to junk,
		//then sets tag currently pointed to
		Tag getTag();

		TagIterator& getToTag(Tag tag);

		SmartIterator& getIt();

		//invalidates the tag iterator
		SmartIterator& getItSafe();

		void setIterator(SmartIterator it);

		bool isSafe() { return valid; }

	private:
		SmartIterator it;
		Tag tag;
		bool valid;
	};
}