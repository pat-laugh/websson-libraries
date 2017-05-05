//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <vector>

#include "base.h"
#include "entity.h"
#include "namespace.h"
#include "templateHead.h"

namespace webss
{
	class ImportedDocument
	{
	public:
		ImportedDocument(Webss&& name) : name(std::move(name)) { assert(this->name.isString() && "import must reference a string"); }

		const Webss& getName() const { return name; }
		const std::string& getLink() const { return name.getString(); }
	private:
		Webss name;
	};
	
	struct ScopedDocument
	{
		BasicTemplateHead<ParamDocument> head;
		DocumentHead body;
	};
	
	class ParamDocument
	{
	public:
		enum class Type
		{
			NONE, ENTITY_ABSTRACT, ENTITY_CONCRETE, SCOPED_DOCUMENT,
			USING_ALL, USING_ONE, IMPORT
		};

		ParamDocument() {}
		static ParamDocument makeEntityAbstract(Entity ent) { return ParamDocument(ent, Type::ENTITY_ABSTRACT); }
		static ParamDocument makeEntityConcrete(Entity ent) { return ParamDocument(ent, Type::ENTITY_CONCRETE); }
		static ParamDocument makeUsingAll(Entity ent) { assert(ent.getContent().isNamespace()); return ParamDocument(ent, Type::USING_ALL); }
		static ParamDocument makeUsingOne(Entity ent, ImportedDocument import) { return ParamDocument(ent, Type::USING_ONE, std::move(import)); }
		ParamDocument(ImportedDocument import) : type(Type::IMPORT), import(new ImportedDocument(std::move(import))) {}
		ParamDocument(ScopedDocument scopedDoc) : type(Type::SCOPED_DOCUMENT), scopedDoc(new ScopedDocument(std::move(scopedDoc))) {}
		~ParamDocument() { destroyUnion(); }

		ParamDocument(ParamDocument&& o) { copyUnion(std::move(o)); }
		ParamDocument(const ParamDocument& o) { copyUnion(o); }

		ParamDocument& operator=(ParamDocument&& o)
		{
			destroyUnion();
			copyUnion(std::move(o));
			return *this;
		}
		ParamDocument& operator=(const ParamDocument& o)
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
		const ImportedDocument& getImport() const { return *import; }
		const ScopedDocument& getScopedDoc() const { return *scopedDoc; }
	private:
		Type type = Type::NONE;
		union
		{
			Entity ent;
			ScopedDocument* scopedDoc;
		};

		ImportedDocument* import;

		ParamDocument(Entity ent, Type type) : type(type), ent(std::move(ent)) {}
		ParamDocument(Entity ent, Type type, ImportedDocument import) : type(type), ent(std::move(ent)), import(new ImportedDocument(std::move(import))) {}

		void destroyUnion()
		{
			switch (type)
			{
			case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::USING_ALL:
				ent.~Entity();
				break;
			case Type::USING_ONE:
				ent.~Entity();
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

		void copyUnion(ParamDocument&& o)
		{
			switch (o.type)
			{
			case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::USING_ALL:
				new (&ent) Entity(std::move(o.ent));
				o.ent.~Entity();
				break;
			case Type::USING_ONE:
				new (&ent) Entity(std::move(o.ent));
				o.ent.~Entity();
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

		void copyUnion(const ParamDocument& o)
		{
			switch (o.type)
			{
			case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::USING_ALL:
				new (&ent) Entity(o.ent);
				break;
			case Type::USING_ONE:
				new (&ent) Entity(o.ent);
				import = new ImportedDocument(*o.import);
				break;
			case Type::IMPORT:
				import = new ImportedDocument(*o.import);
				break;
			case Type::SCOPED_DOCUMENT:
				scopedDoc = new ScopedDocument(*o.scopedDoc);
				break;
			default:
				break;
			}

			type = o.type;
		}
	};
}