//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#ifdef VOID //in Windows
#undef VOID
#endif

#include "various/smartIterator.hpp"

namespace webss
{
	enum class Tag {
		NONE, UNKNOWN,
		NAME_START, DIGIT, MINUS, PLUS, C_STRING, LINE_STRING, EQUAL,
		START_DICTIONARY, START_LIST, START_TUPLE, START_TEMPLATE,
		END_DICTIONARY, END_LIST, END_TUPLE, END_TEMPLATE,
		TEXT_DICTIONARY, TEXT_LIST, TEXT_TUPLE,
		ENTITY_ABSTRACT, ENTITY_CONCRETE, EXPLICIT_NAME, EXPAND, FOREACH,
		SCOPED_IMPORT, IMPORT, OPTION, SELF, VOID, SEPARATOR, SCOPE, SLASH
	};

	class TagIterator
	{
	public:
		TagIterator(various::SmartIterator it);

		TagIterator& operator=(TagIterator&& o);
		TagIterator& operator=(const TagIterator& o) = delete;

		//advances the iterator by one then sets a tag
		TagIterator& operator++(); //prefix
		TagIterator& incSafe();

		Tag operator*() const;
		Tag getSafe();

		operator Tag() const;

		void sofertTag(Tag tag);

		various::SmartIterator& getIt();

		//invalidates the tag iterator
		various::SmartIterator& getItSafe();

		void setIterator(various::SmartIterator it);

		//sets tag to what the iterator currently points to
		Tag update();

		void setTag(Tag tag);

	private:
		various::SmartIterator it;
		Tag tag;
		bool valid;

		//only advances the iterator if it points to junk,
		//then sets tag currently pointed to
		Tag getTag();
	};
}