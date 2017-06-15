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
		TagIterator& operator++(); //prefix
		TagIterator& incSafe();

		Tag operator*() const;
		Tag getSafe();

		operator Tag() const;

		TagIterator& getToTag(Tag tag);

		SmartIterator& getIt();

		//invalidates the tag iterator
		SmartIterator& getItSafe();

		void setIterator(SmartIterator it);

		bool isSafe() { return valid; }

		//sets tag to what the iterator currently points to
		Tag update();

	private:
		SmartIterator it;
		Tag tag;
		bool valid;

		//only advances the iterator if it points to junk,
		//then sets tag currently pointed to
		Tag getTag();
	};
}