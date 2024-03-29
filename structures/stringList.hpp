//MIT License
//Copyright 2018 Patrick Laughrea
#pragma once

#include <string>
#include <vector>

#include "entity.hpp"

namespace webss
{
	enum class StringType
	{
		NONE, STRING, ENT_STATIC, WEBSS,
		ENT_DYNAMIC, FUNC_FLUSH, FUNC_NEWLINE, FUNC_NEWLINE_FLUSH,
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
		StringItem(Webss webss);
		StringItem(StringType t);
		static StringItem makeEntDynamic(const Entity& ent) { return StringItem(ent, true); }

		StringType getTypeRaw() const;
		const std::string& getStringRaw() const;
		const Entity& getEntityRaw() const;
		const Webss& getWebssRaw() const;

	private:
		StringType type = StringType::NONE;
		union
		{
			std::string tString;
			Entity ent;
			Webss* webss;
		};

		StringItem(const Entity& ent, bool);

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
		
		const std::vector<StringItem>& getItems() const;
	};
}