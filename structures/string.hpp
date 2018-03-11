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
		NONE, STRING, ENT_STATIC, FUNC_NEWLINE,
#ifdef COMPILE_WEBSS
		WEBSS, ENT_DYNAMIC, FUNC_FLUSH,
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

		StringItem(StringType t);
		StringItem(std::string&& s);
		StringItem(const Entity& ent);

#ifdef COMPILE_WEBSS
		static StringItem makeEntDynamic(const Entity& ent) { return StringItem(ent, true); }
		StringItem(Webss webss);
		const Webss& getWebssRaw();
#endif

		StringType getTypeRaw();
		const std::string& getStringRaw();
		const Entity& getEntityRaw();

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

	using String = std::vector<StringItem>;
}