//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "entity.hpp"
#include "parameters.hpp"
#include "webss.hpp"

namespace webss
{
#define This BasicThead
	template <class Param>
	class This
	{
	public:
		using Params = BasicParams<Param>;
		using size_type = typename Params::size_type;

		This() : type(Type::PARAMS), params(new Params()) {}
		This(Params&& params) : type(Type::PARAMS), params(new Params(std::move(params))) {}
		This(const Params& params) : This(Params(params)) {}
		This(Entity&& ent) : type(Type::ENTITY), ent(std::move(ent)) {}
		This(const Entity& ent) : type(Type::ENTITY), ent(ent) {}
		~This() { destroyUnion(); }

		This(This&& o) { copyUnion(std::move(o)); }
		This(const This& o) { copyUnion(o); }

		This& operator=(This&& o)
		{
			destroyUnion();
			copyUnion(std::move(o));
			return *this;
		}
		This& operator=(const This& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(o);
			}
			return *this;
		}

		bool operator==(const This& o) const
		{
			if (this == &o)
				return true;
			if (type == Type::NONE || o.type == Type::NONE)
				return type == Type::NONE && o.type == Type::NONE;
			return getParams() == o.getParams();
		}
		bool operator!=(const This& o) const { return !(*this == o); }

		bool hasEntity() const { return type == Type::ENTITY; }
		bool empty() const { return getParams().empty(); }
		size_type size() const { return getParams().size(); }

		Param& back()
		{
			assert(type != Type::NONE);
			if (type == Type::ENTITY)
				removeEntity();
			return params->back();
		}
		const Param& back() const { return getParams().back(); }
		Param& last() { return back(); }
		const Param& last() const { return back(); }

		const Params& getParams() const
		{
			assert(type != Type::NONE);
			return type == Type::ENTITY ? ent.getContent(). template getElement<This>().getParams() : *params;
		}

		const Entity& getEntity() const
		{
			assert(hasEntity());
			return ent;
		}

		void attach(Param&& value)
		{
			if (type == Type::NONE)
			{
				params = new Params();
				type = Type::PARAMS;
			}
			else if (type == Type::ENTITY)
				removeEntity();

			params->add(std::move(value));
		}

		void attachEmpty(std::string&& key)
		{
			attach(std::move(key), Param());
		}

		void attach(std::string&& key, Param&& value)
		{
			if (type == Type::NONE)
			{
				params = new Params();
				type = Type::PARAMS;
			}
			else if (type == Type::ENTITY)
				removeEntity();

			params->addSafe(std::move(key), std::move(value));
		}

		void attach(const Entity& ent2)
		{
			if (ent2.getContent(). template getElement<This>().empty())
				return;

			if (type == Type::NONE)
			{
				new (&ent) Entity(ent2);
				type = Type::ENTITY;
				return;
			}
			else if (type == Type::ENTITY)
				removeEntity();

			params->merge(ent2.getContent(). template getElement<This>().getParams());
		}

		void attach(const This& value)
		{
			if (value.empty())
				return;

			const auto& valueTuple = value.getParams();
			if (type == Type::NONE)
			{
				setParams(valueTuple.makeCompleteCopy());
				return;
			}
			else if (type == Type::ENTITY)
				removeEntity();
			else
			{
				if (params->empty())
				{
					*params = valueTuple.makeCompleteCopy();
					return;
				}
			}

			params->merge(valueTuple);
		}

	private:
		enum class Type { NONE, PARAMS, ENTITY };

		Type type = Type::NONE;
		union
		{
			Params* params;
			Entity ent;
		};

		This(Params* params) : type(Type::PARAMS), params(params) {}

		void removeEntity()
		{
			auto newParams = ent.getContent(). template getElement<This>().getParams().makeCompleteCopy();
			ent.~Entity();
			type = Type::NONE;
			setParams(std::move(newParams));
		}

		void setParams(Params&& newParams)
		{
			params = new Params(std::move(newParams));
			type = Type::PARAMS;
		}

		void destroyUnion()
		{
			if (type == Type::PARAMS)
				delete params;
			else if (type == Type::ENTITY)
				ent.~Entity();
			type = Type::NONE;
		}

		void copyUnion(This&& o)
		{
			if (o.type == Type::PARAMS)
				params = o.params;
			else if (o.type == Type::ENTITY)
			{
				new (&ent) Entity(std::move(o.ent));
				o.ent.~Entity();
			}
			type = o.type;
			o.type = Type::NONE;
		}
		void copyUnion(const This& o)
		{
			if (o.type == Type::PARAMS)
				params = new Params(*o.params);
			else if (o.type == Type::ENTITY)
				new (&ent) Entity(o.ent);
			type = o.type;
		}
	};
#undef This
}