//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "entity.h"
#include "parameters.h"

namespace webss
{
#define This BasicTemplateHead
	template <class Parameter, class Webss>
	class This
	{
	public:
		using Param = Parameter;
		using Parameters = BasicParameters<Param>;
		using Entity = BasicEntity<Webss>;
		using size_type = typename Parameters::size_type;

		This() : type(Type::PARAMS), params(new Parameters()) {}
		This(Parameters&& params) : type(Type::PARAMS), params(new Parameters(std::move(params))) {}
		This(const Parameters& params) : This(Parameters(params)) {}
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

		bool hasEntity() const { return type == Type::ENTITY; }
		bool empty() const { return getParameters().empty(); }
		size_type size() const { return getParameters().size(); }

		Param& back()
		{
			assert(type != Type::NONE);
			if (type == Type::ENTITY)
				removeEntity();
			return params->back();
		}
		const Param& back() const { return getParameters().back(); }

		const Parameters& getParameters() const
		{
			assert(type != Type::NONE);
			return type == Type::ENTITY ? ent.getContent().getElement<This>().getParameters() : *params;
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
				params = new Parameters();
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
				params = new Parameters();
				type = Type::PARAMS;
			}
			else if (type == Type::ENTITY)
				removeEntity();

			params->addSafe(std::move(key), std::move(value));
		}

		void attach(const Entity& ent2)
		{
			if (ent2.getContent().getElement<This>().empty())
				return;

			if (type == Type::NONE)
			{
				new (&ent) Entity(ent2);
				type = Type::ENTITY;
				return;
			}
			else if (type == Type::ENTITY)
				removeEntity();

			params->merge(ent2.getContent().getElement<This>().getParameters());
		}

		void attach(const This& value)
		{
			if (value.empty())
				return;

			const auto& valueTuple = value.getParameters();
			if (type == Type::NONE)
			{
				setParameters(valueTuple.makeCompleteCopy());
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
			Parameters* params;
			Entity ent;
		};

		void removeEntity()
		{
			auto newParameters = ent.getContent().getElement<This>().getParameters().makeCompleteCopy();
			ent.~BasicEntity();
			type = Type::NONE;
			setParameters(std::move(newParameters));
		}

		void setParameters(Parameters&& newParameters)
		{
			params = new Parameters(std::move(newParameters));
			type = Type::PARAMS;
		}

		void destroyUnion()
		{
			if (type == Type::PARAMS)
				delete params;
			else if (type == Type::ENTITY)
				ent.~BasicEntity();
			type = Type::NONE;
		}

		void copyUnion(This&& o)
		{
			if (o.type == Type::PARAMS)
				params = o.params;
			else if (o.type == Type::ENTITY)
			{
				new (&ent) Entity(std::move(o.ent));
				o.ent.~BasicEntity();
			}
			type = o.type;
			o.type = Type::NONE;
		}
		void copyUnion(const This& o)
		{
			if (o.type == Type::PARAMS)
				params = new Parameters(*o.params);
			else if (o.type == Type::ENTITY)
				new (&ent) Entity(o.ent);
			type = o.type;
		}
	};
#undef This
}