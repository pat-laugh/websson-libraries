//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "tuple.h"
#include "entity.h"
#include <cassert>

namespace webss
{
	template <class Parameter>
	class BasicFunctionHead
	{
	public:
		using Tuple = BasicTuple<Parameter>;
		using Pointer = std::shared_ptr<Tuple>;
		using Entity = BasicEntity<BasicFunctionHead>;
		using size_type = typename Tuple::size_type;

		BasicFunctionHead() : t(Type::NONE) {}
		explicit BasicFunctionHead(bool containerText) : t(Type::TUPLE), tuple(new Tuple(containerText)) {}
		BasicFunctionHead(const Entity& ent) : t(Type::VAR), ent(ent) {}
		BasicFunctionHead(Tuple&& tuple) : t(Type::TUPLE), tuple(new Tuple(std::move(tuple))) {}
		BasicFunctionHead(const Tuple& tuple) : t(Type::TUPLE), tuple(new Tuple(tuple)) {}
		BasicFunctionHead(const Pointer& pointer) : t(Type::POINTER), pointer(pointer) {}
		~BasicFunctionHead() { destroyUnion(); }

		BasicFunctionHead(BasicFunctionHead&& o) { copyUnion(std::move(o)); }
		BasicFunctionHead(const BasicFunctionHead& o) { copyUnion(o); }

		BasicFunctionHead& operator=(BasicFunctionHead&& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(std::move(o));
			}
			return *this;
		}
		BasicFunctionHead& operator=(const BasicFunctionHead& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(o);
			}
			return *this;
		}

		bool hasEntity() const { return t == Type::VAR; }
		bool empty() const { return getParameters().empty(); }
		bool isText() const { return getParameters().containerText; }
		typename size_type size() const { return getParameters().size(); }

		Parameter& back() { return const_cast<Parameter&>(getParameters().back()); }
		const Parameter& back() const { return getParameters().back(); }

		const Tuple& getParameters() const
		{
			switch (t)
			{
			case Type::TUPLE:
				return *tuple;
			case Type::POINTER:
				return *pointer;
			case Type::VAR:
				return ent.getContent().getParameters();
			default:
				throw std::logic_error("");
			}
		}

		const std::string& getEntName() const
		{
			return ent.getName();
		}

		void attachEmpty(std::string&& key)
		{
			attach(std::move(key), Parameter());
		}

		void attach(std::string&& key, Parameter&& value)
		{
			switch (t)
			{
			case Type::VAR:
				removeEntity();
				break;
			case Type::POINTER:
				removePointer();
				break;
			case Type::NONE:
				tuple = new Tuple();
				t = Type::TUPLE;
				break;
			default:
				break;
			}

			tuple->addSafe(std::move(key), std::move(value));
		}

		void attach(const Entity& ent2)
		{
			assert(!ent2.getContent().empty()); //not problematic, but a function head shouldn't be empty anyway

			switch (t)
			{
			case Type::VAR:
				removeEntity();
				break;
			case Type::POINTER:
				removePointer();
				break;
			case Type::TUPLE:
				if (!tuple->empty())
					break;
				delete tuple;
			case Type::NONE:
				new (&ent) Entity(ent2);
				t = Type::VAR;
				return;
			default:
				break;
			}

			tuple->merge(ent2.getContent().getParameters());
		}

		void attach(const BasicFunctionHead& value)
		{
			assert(!value.empty()); //not problematic, but a function head shouldn't be empty anyway

			const auto& valueTuple = value.getParameters();
			switch (t)
			{
			case Type::VAR:
				removeEntity();
				break;
			case Type::POINTER:
				removePointer();
				break;
			case Type::TUPLE:
				if (!tuple->empty())
					break;
				*tuple = valueTuple.makeCompleteCopy();
				return;
			case Type::NONE:
				setTuple(valueTuple.makeCompleteCopy());
				return;
			default:
				break;
			}

			tuple->merge(valueTuple);
		}
	private:
		enum class Type { NONE, TUPLE, POINTER, VAR };

		Type t;
		union
		{
			Tuple* tuple;
			Pointer pointer;
			Entity ent;
		};

		void removeEntity()
		{
			const auto& newTuple = ent.getContent().getParameters().makeCompleteCopy();
			ent.~BasicEntity();
			setTuple(newTuple);
		}

		void removePointer()
		{
			const auto& newTuple = pointer->makeCompleteCopy();
			pointer.~shared_ptr();
			setTuple(newTuple);
		}

		void setTuple(const Tuple& newTuple)
		{
			tuple = new Tuple(newTuple);
			t = Type::TUPLE;
		}

		void destroyUnion()
		{
			switch (t)
			{
			case Type::TUPLE:
				delete tuple;
				break;
			case Type::POINTER:
				pointer.~shared_ptr();
				break;
			case Type::VAR:
				ent.~BasicEntity();
				break;
			default:
				break;
			}
		}

		void copyUnion(BasicFunctionHead&& o)
		{
			switch (t = o.t)
			{
			case Type::TUPLE:
				tuple = o.tuple;
				break;
			case Type::POINTER:
				new (&pointer) Pointer(std::move(o.pointer));
				break;
			case Type::VAR:
				new (&ent) Entity(std::move(o.ent));
				break;
			default:
				break;
			}
			o.t = Type::NONE;
		}
		void copyUnion(const BasicFunctionHead& o)
		{
			switch (t = o.t)
			{
			case Type::TUPLE:
				tuple = new Tuple(*o.tuple);
				break;
			case Type::POINTER:
				new (&pointer) Pointer(o.pointer);
				break;
			case Type::VAR:
				new (&ent) Entity(o.ent);
				break;
			default:
				break;
			}
		}
	};
}