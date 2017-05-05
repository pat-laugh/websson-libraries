//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "base.h"
#include "entity.h"
#include "webss.h"

namespace webss
{
	class BlockHead
	{
	public:
		BlockHead();
		BlockHead(const Entity& ent);

		bool hasEntity() const;
		const Entity& getEntity() const;
	private:
		bool hasEnt = false;
		Entity ent;
	};

	class Block : public BlockHead
	{
	private:
		using Head = BlockHead;
		Webss value;
	public:
		Block(Webss&& value);
		Block(const Webss& value);
		Block(const Entity& ent, Webss&& value);
		Block(const Entity& ent, const Webss& value);
		Block(Head&& head, Webss&& value);
		Block(const Head& head, const Webss& value);

		const Webss& getValue() const;
	};
}