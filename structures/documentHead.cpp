//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "documentHead.hpp"

using namespace std;
using namespace webss;

ImportedDocument::ImportedDocument(Webss&& name) : name(std::move(name)) { assert(this->name.isString() && "import must reference a string"); }

const Webss& ImportedDocument::getName() const { return name; }
const std::string& ImportedDocument::getLink() const { return name.getString(); }


ParamDocument::ParamDocument() {}
ParamDocument::ParamDocument(ImportedDocument import) : type(Type::IMPORT), import(new ImportedDocument(std::move(import))) {}
ParamDocument::ParamDocument(ScopedDocument scopedDoc) : type(Type::SCOPED_DOCUMENT), scopedDoc(new ScopedDocument(std::move(scopedDoc))) {}
ParamDocument::~ParamDocument() { destroyUnion(); }

ParamDocument::ParamDocument(ParamDocument&& o) { copyUnion(std::move(o)); }
ParamDocument::ParamDocument(const ParamDocument& o) { copyUnion(o); }

ParamDocument& ParamDocument::operator=(ParamDocument&& o)
{
	destroyUnion();
	copyUnion(std::move(o));
	return *this;
}
ParamDocument& ParamDocument::operator=(const ParamDocument& o)
{
	if (this != &o)
	{
		destroyUnion();
		copyUnion(o);
	}
	return *this;
}

ParamDocument::Type ParamDocument::getType() const { return type; }
bool ParamDocument::hasNamespace() const { return type == Type::USING_ALL; }
const Entity& ParamDocument::getEntity() const { return ent; }
const Namespace& ParamDocument::getNamespace() const { return ent.getContent().getNamespace(); }
const ImportedDocument& ParamDocument::getImport() const { return *import; }
const ScopedDocument& ParamDocument::getScopedDoc() const { return *scopedDoc; }

ParamDocument::ParamDocument(Entity ent, Type type) : type(type), ent(std::move(ent)) {}
ParamDocument::ParamDocument(Entity ent, Type type, ImportedDocument import) : type(type), ent(std::move(ent)), import(new ImportedDocument(std::move(import))) {}

void ParamDocument::destroyUnion()
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

void ParamDocument::copyUnion(ParamDocument&& o)
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

void ParamDocument::copyUnion(const ParamDocument& o)
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