#pragma once

#include "entity.h"
#include "namespace.h"

namespace webss
{
#define This BasicParamScoped
	template <class Webss>
	class This
	{
	public:
		using Entity = BasicEntity<Webss>;
		using Namespace = BasicNamespace<Webss>;

		enum class Type { NONE, ENTITY, NAMESPACE };

		Type type;
		union
		{
			Entity ent;
			Namespace nspace;
		};
		
		This() : type(Type::NONE) {}
		This(Entity&& ent) : type(Type::ENTITY), ent(std::move(ent)) {}
		This(const Entity& ent) : type(Type::ENTITY), ent(ent) {}
		This(const Namespace& nspace) : type(Type::NAMESPACE), nspace(nspace) {}

		~This() { destroyUnion(); }

		This(This&& o) { copyUnion(std::move(o)); }
		This(const This& o) { copyUnion(o); }

		This& operator=(This&& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(std::move(o));
			}
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
		bool hasNamespace() const { return type == Type::NAMESPACE; }

		const Entity& getEntity() const { return ent; }
		const Namespace& getNamespace() const { return nspace; }
	private:
		void destroyUnion()
		{
			switch (type)
			{
			case Type::ENTITY:
				ent.~BasicEntity();
				break;
			case Type::NAMESPACE:
				nspace.~BasicNamespace();
				break;
			default:
				break;
			}
		}

		void copyUnion(This&& o)
		{
			switch (type = o.type)
			{
			case Type::ENTITY:
				new (&ent) Entity(std::move(o.ent));
				o.type = Type::NONE;
				break;
			case Type::NAMESPACE:
				new (&nspace) Namespace(std::move(o.nspace));
				o.type = Type::NONE;
				break;
			default:
				break;
			}
		}
		void copyUnion(const This& o)
		{
			switch (type = o.type)
			{
			case Type::ENTITY:
				new (&ent) Entity(o.ent);
				break;
			case Type::NAMESPACE:
				new (&nspace) Namespace(o.nspace);
				break;
			default:
				break;
			}
		}
	};
#undef This
}