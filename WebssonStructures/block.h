//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <memory>

#include "variable.h"
#include "types.h"

namespace webss
{
	class BlockId
	{
	public:
		using Variable = BasicVariable<type_int>;

		BlockId(std::string&& name, type_int index) : name(std::move(name)), hasVar(false), index(index) {}
		BlockId(const std::string& name, type_int index) : name(name), hasVar(false), index(index) {}
		BlockId(std::string&& name, const Variable& var) : name(std::move(name)), hasVar(true), var(var) {}
		BlockId(const std::string& name, const Variable& var) : name(name), hasVar(true), var(var) {}

		BlockId(BlockId&& o) : name(std::move(o.name)) { copyUnion(std::move(o)); }
		BlockId(const BlockId& o) : name(o.name) { copyUnion(o); }

		~BlockId() { destroyUnion(); }

		const std::string& getName() const { return name; }
		type_int getIndex() const { return hasVar ? var.getContent() : index; }
		bool hasVariable() const { return hasVar; }
		const std::string& getVarName() const { return var.getName(); }
	private:
		std::string name;
		bool hasVar;
		union
		{
			type_int index; 
			Variable var;
		};

		void copyUnion(BlockId&& o)
		{
			if ((hasVar = o.hasVar))
			{
				new (&var) Variable(std::move(o.var));
				o.hasVar = false;
			}
			else
				index = o.index;
		}
		void copyUnion(const BlockId& o)
		{
			if ((hasVar = o.hasVar))
				new (&var) Variable(o.var);
			else
				index = o.index;
		}

		void destroyUnion()
		{
			if (hasVar)
				var.~BasicVariable();
		}
	};

	template <class Webss>
	class BasicBlock
	{
	public:
		using Variable = BasicVariable<BlockId>;

		BasicBlock(const Variable& id, Webss&& value) : id(id), value(std::move(value)) {}
		BasicBlock(const Variable& id, const Webss& value) : id(id), value(value) {}

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
		type_int getIndex() const { return id.getContent().getIndex(); }
		const Webss& getValue() const { return value; }
	private:
		Variable id;
		Webss value;
	};
}