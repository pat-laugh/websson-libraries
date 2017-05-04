//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <vector>

#include "base.h"
#include "entity.h"
#include "templateScoped.h"

namespace webss
{
	template <class Webss>
	class BasicParamDocument;

	template <class Webss>
	using BasicDocumentHead = std::vector<BasicParamDocument<Webss>>;

	template <class Webss>
	class BasicScopedDocument
	{
	public:
		using TheadScoped = BasicTemplateHeadScoped<Webss>;
		using DocHead = BasicDocumentHead<Webss>;

		TheadScoped head;
		DocHead body;
	};

	template <class Webss>
	class BasicImportedDocument
	{
	public:
		BasicImportedDocument(Webss&& name) : name(std::move(name)) { assert(this->name.isString() && "import must reference a string"); }

		const Webss& getName() const { return name; }
		const std::string& getLink() const { return name.getString(); }
	private:
		Webss name;
	};

#define This BasicParamDocument
	template <class Webss>
	class This
	{
	public:
		using Entity = BasicEntity<Webss>;
		using Namespace = BasicNamespace<Webss>;
		using ScopedDoc = BasicScopedDocument<Webss>;
		using Import = BasicImportedDocument<Webss>;

		enum class Type
		{
			NONE, ENTITY_ABSTRACT, ENTITY_CONCRETE, SCOPED_DOCUMENT,
			USING_ALL, USING_ONE, IMPORT
		};

		This() {}
		static This makeEntityAbstract(Entity ent) { return This(ent, Type::ENTITY_ABSTRACT); }
		static This makeEntityConcrete(Entity ent) { return This(ent, Type::ENTITY_CONCRETE); }
		static This makeUsingAll(Entity ent) { assert(ent.getContent().isNamespace()); return This(ent, Type::USING_ALL); }
		static This makeUsingOne(Entity ent, Import import) { return This(ent, Type::USING_ONE, std::move(import)); }
		This(Import import) : type(Type::IMPORT), import(new Import(std::move(import))) {}
		This(ScopedDoc scopedDoc) : type(Type::SCOPED_DOCUMENT), scopedDoc(new ScopedDoc(std::move(scopedDoc))) {}
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
		bool hasNamespace() const { return type == Type::USING_ALL; }
		const Entity& getEntity() const { return ent; }
		const Namespace& getNamespace() const { return ent.getContent().getNamespace(); }
		const Import& getImport() const { return *import; }
		const ScopedDoc& getScopedDoc() const { return *scopedDoc; }
	private:
		Type type = Type::NONE;
		union
		{
			Entity ent;
			ScopedDoc* scopedDoc;
		};

		Import* import;

		This(Entity ent, Type type) : type(type), ent(std::move(ent)) {}
		This(Entity ent, Type type, Import import) : type(type), ent(std::move(ent)), import(new Import(std::move(import))) {}

		void destroyUnion()
		{
			switch (type)
			{
			case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::USING_ALL:
				ent.~BasicEntity();
				break;
			case Type::USING_ONE:
				ent.~BasicEntity();
				delete import;
				break;
			case Type::IMPORT:
				delete import;
				break;
			case Type::SCOPED_DOCUMENT:
				delete scopedDoc;
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
			case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::USING_ALL:
				new (&ent) Entity(std::move(o.ent));
				o.ent.~BasicEntity();
				break;
			case Type::USING_ONE:
				new (&ent) Entity(std::move(o.ent));
				o.ent.~BasicEntity();
				import = o.import;
				break;
			case Type::IMPORT:
				import = o.import;
				break;
			case Type::SCOPED_DOCUMENT:
				scopedDoc = o.scopedDoc;
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
			case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::USING_ALL:
				new (&ent) Entity(o.ent);
				break;
			case Type::USING_ONE:
				new (&ent) Entity(o.ent);
				import = new Import(*o.import);
				break;
			case Type::IMPORT:
				import = new Import(*o.import);
				break;
			case Type::SCOPED_DOCUMENT:
				scopedDoc = new ScopedDoc(*o.scopedDoc);
				break;
			default:
				break;
			}

			type = o.type;
		}
	};
#undef This
}
