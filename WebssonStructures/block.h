//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "entity.h"

namespace webss
{
#define This BasicBlockHead
	template <class Webss>
	class This
	{
	public:
		using Entity = BasicEntity<This>;

		This() {}
		This(const Entity& ent) : hasEnt(true), ent(ent) {}

		bool hasEntity() const { return hasEnt; }
		const std::string& getEntName() const { return ent.getName(); }
	private:
		bool hasEnt = false;
		Entity ent;
	};
#undef This

#define This BasicBlockBody
	template <class Webss>
	class This
	{
	public:
		This() {}
		This(Webss&& value) : value(std::move(value)) {}
		This(const Webss& value) : value(value) {}

		~This() {}

		This(This&& o) : value(std::move(o.value)) {}
		This(const This& o) : value(o.value) {}

		This& operator=(This&& o)
		{
			if (this != &o)
				value = std::move(o.value);
			return *this;
		}
		This& operator=(const This& o)
		{
			if (this != &o)
				value = o.value;
			return *this;
		}

		const Webss& getValue() const { return value; }

	private:
		Webss value;
	};
#undef This

#define This BasicBlock
	template <class Webss>
	class This : public BasicBlockHead<Webss>, public BasicBlockBody<Webss>
	{
	private:
		using head = BasicBlockHead<Webss>;
		using body = BasicBlockBody<Webss>;
	public:
		using Entity = BasicEntity<BasicBlockHead<Webss>>;

		This(Webss&& value) : head(), body(std::move(value)) {}
		This(const Webss& value) : head(), body(value) {}
		This(const Entity& ent, Webss&& value) : head(ent), body(std::move(value)) {}
		This(const Entity& ent, const Webss& value) : head(ent), body(value) {}
		This(head&& h, Webss&& value) : head(std::move(h)), body(std::move(value)) {}
		This(const head& h, const Webss& value) : head(h), body(value) {}
	};
#undef This
}