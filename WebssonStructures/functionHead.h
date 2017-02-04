//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "entity.h"
#include "parameters.h"

namespace webss
{
#define This BasicFunctionHead
	template <class Parameter, class Webss>
	class This
	{
	public:
		using Param = Parameter;
		using Parameters = BasicParameters<Param>;
		using Pointer = std::shared_ptr<Webss>;
		using Entity = BasicEntity<Webss>;
		using size_type = typename Parameters::size_type;

		This() : type(Type::PARAMS), params(new Parameters()) {}
		This(Parameters&& params) : type(Type::PARAMS), params(new Parameters(std::move(params))) {}
		This(const Parameters& params) : This(Parameters(params)) {}
		This(const Pointer& pointer) : type(Type::POINTER), pointer(pointer) {}
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
			switch (type)
			{
			case Type::NONE:
				assert(false); throw std::domain_error("");
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
		const Param& back() const { return getParameters().back(); }

		const Parameters& getParameters() const
		{
			switch (type)
			{
			case Type::NONE: default:
				assert(false); throw std::domain_error("");
			case Type::PARAMS:
				return *params;
			case Type::POINTER:
				return pointer->getElement<This>().getParameters();
			case Type::ENTITY:
				return ent.getContent().getElement<This>().getParameters();
			}
		}

		const Entity& getEntity() const
		{
			assert(hasEntity());
			return ent;
		}

		void attach(Param&& value)
		{
			switch (type)
			{
			case Type::NONE:
				params = new Parameters();
				type = Type::PARAMS;
			case Type::PARAMS:
				break;
			case Type::ENTITY:
				removeEntity();
				break;
			case Type::POINTER:
				removePointer();
				break;
			}

			params->add(std::move(value));
		}

		void attachEmpty(std::string&& key)
		{
			attach(std::move(key), Param());
		}

		void attach(std::string&& key, Param&& value)
		{
			switch (type)
			{
			case Type::NONE:
				params = new Parameters();
				type = Type::PARAMS;
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
			if (ent2.getContent().getElement<This>().empty())
				return;

			switch (type)
			{
			case Type::NONE:
				new (&ent) Entity(ent2);
				type = Type::ENTITY;
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

			params->merge(ent2.getContent().getElement<This>().getParameters());
		}

		void attach(const This& value)
		{
			if (value.empty())
				return;

			const auto& valueTuple = value.getParameters();
			switch (type)
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

		Type type = Type::NONE;
		union
		{
			Parameters* params;
			Pointer pointer;
			Entity ent;
		};

		void removeEntity()
		{
			auto newParameters = ent.getContent().getElement<This>().getParameters().makeCompleteCopy();
			ent.~BasicEntity();
			type = Type::NONE;
			setParameters(std::move(newParameters));
		}

		void removePointer()
		{
			auto newParameters = pointer->getElement<This>().getParameters().makeCompleteCopy();
			pointer.~shared_ptr();
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
			switch (type)
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
			type = Type::NONE;
		}

		void copyUnion(This&& o)
		{
			switch (o.type)
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
			type = o.type;
			o.type = Type::NONE;
		}
		void copyUnion(const This& o)
		{
			switch (o.type)
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
			type = o.type;
		}
	};
#undef This
}