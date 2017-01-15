//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <memory>

#include "entity.h"
#include "types.h"

namespace webss
{
	class BlockId
	{
	public:
		using Entity = BasicEntity<WebssInt>;

		BlockId(std::string&& name, WebssInt index) : name(std::move(name)), hasEnt(false), index(index) {}
		BlockId(const std::string& name, WebssInt index) : name(name), hasEnt(false), index(index) {}
		BlockId(std::string&& name, const Entity& ent) : name(std::move(name)), hasEnt(true), ent(ent) {}
		BlockId(const std::string& name, const Entity& ent) : name(name), hasEnt(true), ent(ent) {}

		BlockId(BlockId&& o) : name(std::move(o.name)) { copyUnion(std::move(o)); }
		BlockId(const BlockId& o) : name(o.name) { copyUnion(o); }

		~BlockId() { destroyUnion(); }

		const std::string& getName() const { return name; }
		WebssInt getIndex() const { return hasEnt ? ent.getContent() : index; }
		bool hasEntity() const { return hasEnt; }
		const std::string& getEntName() const { return ent.getName(); }
	private:
		std::string name;
		bool hasEnt;
		union
		{
			WebssInt index; 
			Entity ent;
		};

		void copyUnion(BlockId&& o)
		{
			if ((hasEnt = o.hasEnt))
			{
				new (&ent) Entity(std::move(o.ent));
				o.hasEnt = false;
			}
			else
				index = o.index;
		}
		void copyUnion(const BlockId& o)
		{
			if ((hasEnt = o.hasEnt))
				new (&ent) Entity(o.ent);
			else
				index = o.index;
		}

		void destroyUnion()
		{
			if (hasEnt)
				ent.~BasicEntity();
		}
	};

	template <class Webss>
	class BasicBlock
	{
	public:
		using Entity = BasicEntity<BlockId>;

		BasicBlock(const Entity& id, Webss&& value) : id(id), value(std::move(value)) {}
		BasicBlock(const Entity& id, const Webss& value) : id(id), value(value) {}

		~BasicBlock() {}

		BasicBlock(BasicBlock&& o) : id(o.id), value(std::move(o.value)) { }
		BasicBlock(const BasicBlock& o) : id(o.id), value(o.value) { }

		BasicBlock& operator=(BasicBlock&& o)
		{
			if (this != &o)
			{
				id = std::move(o.id);
				value = std::move(o.value);
			}
			return *this;
		}
		BasicBlock& operator=(const BasicBlock& o)
		{
			if (this != &o)
			{
				id = o.id;
				value = o.value;
			}
			return *this;
		}

		const std::string& getName() const { return id.getName(); }
		WebssInt getIndex() const { return id.getContent().getIndex(); }
		const Webss& getValue() const { return value; }
	private:
		Entity id;
		Webss value;
	};
}