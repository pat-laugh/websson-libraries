#pragma once

#include "base.h"
#include "entity.h"
#include "functionHead.h"
#include "namespace.h"

namespace webss
{
	template <class Webss>
	using BasicParamScoped = BasicParamDocument<Webss>;
/*
#define This BasicParamScoped
	template <class Webss>
	class This
	{
	public:
		using Entity = BasicEntity<Webss>;
		using Namespace = BasicNamespace<Webss>;
		using Import = BasicImportedDocument<Webss>;

		enum class Type { NONE, ENTITY, NAMESPACE, IMPORT };

		This() {}
		This(Entity&& ent) : type(Type::ENTITY), ent(std::move(ent)) {}
		This(Namespace&& nspace) : type(Type::NAMESPACE), nspace(std::move(nspace)) {}
		This(Import&& import) : type(Type::IMPORT), import(std::move(import)) {}
		This(const Entity& ent) : type(Type::ENTITY), ent(ent) {}
		This(const Namespace& nspace) : type(Type::NAMESPACE), nspace(nspace) {}
		This(const Import& import) : type(Type::IMPORT), import(import) {}

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

		Type getType() const { return type; }
		bool hasEntity() const { return type == Type::ENTITY; }
		bool hasNamespace() const { return type == Type::NAMESPACE; }
		bool hasImport() const { return type == Type::IMPORT; }

		const Entity& getEntity() const { return ent; }
		const Namespace& getNamespace() const { return nspace; }
		const Import& getImport() const { return import; }
	private:
		Type type = Type::NONE;
		union
		{
			Entity ent;
			Namespace nspace;
			Import import;
		};

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
			case Type::IMPORT:
				import.~BasicImportedDocument();
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
			case Type::ENTITY:
				new (&ent) Entity(std::move(o.ent));
				o.ent.~BasicEntity();
				break;
			case Type::NAMESPACE:
				new (&nspace) Namespace(std::move(o.nspace));
				o.nspace.~BasicNamespace();
				break;
			case Type::IMPORT:
				new (&import) Import(std::move(o.import));
				o.import.~BasicImportedDocument();
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
			case Type::ENTITY:
				new (&ent) Entity(o.ent);
				break;
			case Type::NAMESPACE:
				new (&nspace) Namespace(o.nspace);
				break;
			case Type::IMPORT:
				new (&import) Import(o.import);
				break;
			default:
				break;
			}
			type = o.type;
		}
	};
#undef This*/

	template <class Webss>
	using BasicFunctionHeadScoped = BasicFunctionHead<BasicParamDocument<Webss>>;

#define This BasicFunctionScoped
	template <class Webss>
	class This : public BasicFunctionHeadScoped<Webss>
	{
	private:
		Webss value;
	public:
		using Head = BasicFunctionHeadScoped<Webss>;
		using HeadParameters = typename Head::Parameters;
		using HeadPointer = typename Head::Pointer;
		using HeadEntity = typename Head::Entity;

		This(Head&& head, Webss&& value) : Head(std::move(head)), value(std::move(value)) {}
		This(const Head& head, const Webss& value) : Head(head), value(value) {}
		This(HeadParameters&& head, Webss&& value) : Head(std::move(head)), value(std::move(value)) {}
		This(const HeadParameters& head, const Webss& value) : Head(head), value(value) {}
		This(const HeadPointer& head, Webss&& value) : Head(head), value(std::move(value)) {}
		This(const HeadPointer& head, const Webss& value) : Head(head), value(value) {}
		This(const HeadEntity& head, Webss&& value) : Head(head), value(std::move(value)) {}
		This(const HeadEntity& head, const Webss& value) : Head(head), value(value) {}

		const Webss& getValue() const { return value; }
	};
#undef This
}