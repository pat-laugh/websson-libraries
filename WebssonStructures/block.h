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
		const Entity& getEntity() const { return ent; }
	private:
		bool hasEnt = false;
		Entity ent;
	};
#undef This

#define This BasicBlock
	template <class Webss>
	class This : public BasicBlockHead<Webss>
	{
	private:
		using Head = BasicBlockHead<Webss>;
		Webss value;
	public:
		using Entity = BasicEntity<BasicBlockHead<Webss>>;

		This(Webss&& value) : Head(), value(std::move(value)) {}
		This(const Webss& value) : Head(), value(value) {}
		This(const Entity& ent, Webss&& value) : Head(ent), value(std::move(value)) {}
		This(const Entity& ent, const Webss& value) : Head(ent), value(value) {}
		This(Head&& head, Webss&& value) : Head(std::move(head)), value(std::move(value)) {}
		This(const Head& head, const Webss& value) : Head(head), value(value) {}

		const Webss& getValue() const { return value; }
	};
#undef This
}