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

		~BasicScopedDocument() {}

		TheadScoped head;
		DocHead body;
	};

	template <class Webss>
	class BasicImportedDocument
	{
	public:
		BasicImportedDocument(Webss&& name) : name(std::move(name))
		{
			assert(this->name.isString() && "import must reference a string");
			link = const_cast<std::string*>(&this->name.getString());
		}
		~BasicImportedDocument() {}

		const Webss& getName() const { return name; }
		const std::string& getLink() const { return *link; }
	private:
		Webss name;
		std::string* link;
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

		enum class Type { NONE, ENTITY_ABSTRACT, ENTITY_CONCRETE, NAMESPACE, IMPORT, SCOPED_DOCUMENT };

		This() {}
		This(Entity entAbstract) : type(Type::ENTITY_ABSTRACT), entAbstract(std::move(entAbstract)) {}
		This(Entity entConcrete, bool isConcrete) : type(Type::ENTITY_CONCRETE), entAbstract(std::move(entConcrete)) {}
		This(Namespace nspace) : type(Type::NAMESPACE), nspace(new Namespace(std::move(nspace))) {}
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
		bool hasNamespace() const { return type == Type::NAMESPACE; }
		const Entity& getAbstractEntity() const { return entAbstract; }
		const Entity& getConcreteEntity() const { return entConcrete; }
		const Namespace& getNamespace() const { return *nspace; }
		const Import& getImportedDoc() const { return *import; }
		const ScopedDoc& getScopedDoc() const { return *scopedDoc; }
	private:
		Type type = Type::NONE;
		union
		{
			Entity entAbstract;
			Entity entConcrete;
			Namespace* nspace;
			Import* import;
			ScopedDoc* scopedDoc;
		};

		void destroyUnion()
		{
			switch (type)
			{
			case Type::ENTITY_ABSTRACT:
				entAbstract.~BasicEntity();
				break;
			case Type::ENTITY_CONCRETE:
				entConcrete.~BasicEntity();
				break;
			case Type::NAMESPACE:
				delete nspace;
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
			case Type::ENTITY_ABSTRACT:
				new (&entAbstract) Entity(std::move(o.entAbstract));
				o.entAbstract.~BasicEntity();
				break;
			case Type::ENTITY_CONCRETE:
				new (&entConcrete) Entity(std::move(o.entConcrete));
				o.entConcrete.~BasicEntity();
				break;
			case Type::NAMESPACE:
				nspace = o.nspace;
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
			case Type::ENTITY_ABSTRACT:
				new (&entAbstract) Entity(o.entAbstract);
				break;
			case Type::ENTITY_CONCRETE:
				new (&entConcrete) Entity(o.entConcrete);
				break;
			case Type::NAMESPACE:
				nspace = new Namespace(*o.nspace);
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