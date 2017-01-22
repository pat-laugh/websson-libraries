//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "entity.h"
#include "functionHead.h"
#include "paramScoped.h"

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

#define This BasicFunctionScoped
	template <class Webss>
	class This : public BasicFunctionHead<BasicParamScoped<Webss>>
	{
	private:
		Webss value;
	public:
		using Head = BasicFunctionHead<BasicParamScoped<Webss>>;
		using HeadTuple = typename Head::Tuple;
		using HeadPointer = typename Head::Pointer;
		using HeadEntity = typename Head::Entity;

		This(Head&& head, Webss&& value) : Head(std::move(head)), value(std::move(value)) {}
		This(const Head& head, const Webss& value) : Head(head), value(value) {}
		This(HeadTuple&& head, Webss&& value) : Head(std::move(head)), value(std::move(value)) {}
		This(const HeadTuple& head, const Webss& value) : Head(head), value(value) {}
		This(const HeadPointer& head, Webss&& value) : Head(head), value(std::move(value)) {}
		This(const HeadPointer& head, const Webss& value) : Head(head), value(value) {}
		This(const HeadEntity& head, Webss&& value) : Head(head), value(std::move(value)) {}
		This(const HeadEntity& head, const Webss& value) : Head(head), value(value) {}

		const Webss& getValue() const { return value; }
	};
#undef This
}