//MIT License
//Copyright 2018 Patrick Laughrea
#pragma once

#include <string>
#include <vector>

#include "entity.hpp"
#ifdef COMPILE_WEBSS
#include "base.hpp"
#endif

namespace webss
{
	enum class StringType
	{
		NONE, STRING, ENT_STATIC,
#ifdef COMPILE_WEBSS
		WEBSS, ENT_DYNAMIC, FUNC_FLUSH, FUNC_NEWLINE, FUNC_CANCEL_FLUSH, FUNC_CANCEL_NEWLINE,
#endif
	};

	class StringItem
	{
	public:
		StringItem();
		~StringItem();

		StringItem(StringItem&& o);
		StringItem(const StringItem& o);

		StringItem& operator=(StringItem o);

		bool operator==(const StringItem& o) const;
		bool operator!=(const StringItem& o) const;

		StringItem(std::string&& s);
		StringItem(const Entity& ent);

#ifdef COMPILE_WEBSS
		StringItem(StringType t);
		StringItem(Webss webss);
		static StringItem makeEntDynamic(const Entity& ent) { return StringItem(ent, true); }
		const Webss& getWebssRaw() const;
#endif

		StringType getTypeRaw() const;
		const std::string& getStringRaw() const;
		const Entity& getEntityRaw() const;

	private:
		StringType type = StringType::NONE;
		union
		{
			std::string tString;
			Entity ent;
#ifdef COMPILE_WEBSS
			Webss* webss;
#endif
		};

#ifdef COMPILE_WEBSS
		StringItem(const Entity& ent, bool);
#endif

		void destroyUnion();
		void copyUnion(StringItem&& o);
		void copyUnion(const StringItem& o);
	};

	class StringList
	{
	private:
		std::vector<StringItem> items;

	public:
		StringList();
		StringList(StringList&& o);
		StringList(const StringList& o);

		bool operator==(const StringList& o) const;
		bool operator!=(const StringList& o) const;

		void push(StringItem item);
		void push(const StringList& sl);
		std::string concat() const;
	};
}