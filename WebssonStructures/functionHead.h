//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "entity.h"
#include "parameters.h"

namespace webss
{
	template <class Parameter>
	class BasicFunctionHead
	{
	public:
		using Parameters = BasicParameters<Parameter>;
		using Pointer = std::shared_ptr<BasicFunctionHead>;
		using Entity = BasicEntity<BasicFunctionHead>;
		using size_type = typename Parameters::size_type;

		BasicFunctionHead() {}
		BasicFunctionHead(Parameters&& params) : t(Type::PARAMS), params(new Parameters(std::move(params))) {}
		BasicFunctionHead(const Parameters& params) : BasicFunctionHead(Parameters(params)) {}
		BasicFunctionHead(const Pointer& pointer) : t(Type::POINTER), pointer(pointer) {}
		BasicFunctionHead(const Entity& ent) : t(Type::ENTITY), ent(ent) {}
		~BasicFunctionHead() { destroyUnion(); }

		BasicFunctionHead(BasicFunctionHead&& o) { copyUnion(std::move(o)); }
		BasicFunctionHead(const BasicFunctionHead& o) { copyUnion(o); }

		BasicFunctionHead& operator=(BasicFunctionHead&& o)
		{
			destroyUnion();
			copyUnion(std::move(o));
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

		bool hasEntity() const { return t == Type::ENTITY; }
		bool empty() const { return getParameters().empty(); }
		size_type size() const { return getParameters().size(); }

		Parameter& back()
		{
			switch (t)
			{
			case Type::NONE:
				assert(false);
				throw std::logic_error("");
			case Type::PARAMS:
				break;
			case Type::POINTER:
				removePointer();
				break;
			case Type::ENTITY:
				removeEntity();
				break;
			}

			return params->back();
		}
		const Parameter& back() const { return getParameters().back(); }

		const Parameters& getParameters() const
		{
			switch (t)
			{
			case Type::NONE:
				assert(false);
				throw std::logic_error("");
			case Type::PARAMS:
				return *params;
			case Type::POINTER:
				return pointer->getParameters();
			case Type::ENTITY:
				return ent.getContent().getParameters();
			}
		}

		const Entity& getEntity() const
		{
			assert(hasEntity());
			return ent;
		}

		void attachEmpty(std::string&& key)
		{
			attach(std::move(key), Parameter());
		}

		void attach(std::string&& key, Parameter&& value)
		{
			switch (t)
			{
			case Type::NONE:
				params = new Parameters();
				t = Type::PARAMS;
			case Type::PARAMS:
				break;
			case Type::ENTITY:
				removeEntity();
				break;
			case Type::POINTER:
				removePointer();
				break;
			}

			params->addSafe(std::move(key), std::move(value));
		}

		void attach(const Entity& ent2)
		{
			if (ent2.getContent().empty())
				return;

			switch (t)
			{
			case Type::NONE:
				new (&ent) Entity(ent2);
				t = Type::ENTITY;
				return;
			case Type::PARAMS:
				break;
			case Type::POINTER:
				removePointer();
				break;
			case Type::ENTITY:
				removeEntity();
				break;
			}

			params->merge(ent2.getContent().getParameters());
		}

		void attach(const BasicFunctionHead& value)
		{
			if (value.empty())
				return;

			const auto& valueTuple = value.getParameters();
			switch (t)
			{
			case Type::NONE:
				setParameters(valueTuple.makeCompleteCopy());
				return;
			case Type::PARAMS:
				if (!params->empty())
					break;
				*params = valueTuple.makeCompleteCopy();
				return;
			case Type::POINTER:
				removePointer();
				break;
			case Type::ENTITY:
				removeEntity();
				break;
			}

			params->merge(valueTuple);
		}
	private:
		enum class Type { NONE, PARAMS, POINTER, ENTITY };

		Type t = Type::NONE;
		union
		{
			Parameters* params;
			Pointer pointer;
			Entity ent;
		};

		void removeEntity()
		{
			auto newParameters = ent.getContent().getParameters().makeCompleteCopy();
			ent.~BasicEntity();
			t = Type::NONE;
			setParameters(std::move(newParameters));
		}

		void removePointer()
		{
			auto newParameters = pointer->getParameters().makeCompleteCopy();
			pointer.~shared_ptr();
			t = Type::NONE;
			setParameters(std::move(newParameters));
		}

		void setParameters(Parameters&& newParameters)
		{
			params = new Parameters(std::move(newParameters));
			t = Type::PARAMS;
		}

		void destroyUnion()
		{
			switch (t)
			{
			case Type::PARAMS:
				delete params;
				break;
			case Type::POINTER:
				pointer.~shared_ptr();
				break;
			case Type::ENTITY:
				ent.~BasicEntity();
				break;
			default:
				break;
			}
			t = Type::NONE;
		}

		void copyUnion(BasicFunctionHead&& o)
		{
			switch (o.t)
			{
			case Type::PARAMS:
				params = o.params;
				break;
			case Type::POINTER:
				new (&pointer) Pointer(std::move(o.pointer));
				o.pointer.~shared_ptr();
				break;
			case Type::ENTITY:
				new (&ent) Entity(std::move(o.ent));
				o.ent.~BasicEntity();
				break;
			default:
				break;
			}
			t = o.t;
			o.t = Type::NONE;
		}
		void copyUnion(const BasicFunctionHead& o)
		{
			switch (o.t)
			{
			case Type::PARAMS:
				params = new Parameters(*o.params);
				break;
			case Type::POINTER:
				new (&pointer) Pointer(o.pointer);
				break;
			case Type::ENTITY:
				new (&ent) Entity(o.ent);
				break;
			default:
				break;
			}
			t = o.t;
		}
	};
}