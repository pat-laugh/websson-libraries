//MIT License
//Copyright 2017 Patrick Laughrea
#include "documentHead.hpp"

#include "tuple.hpp"

using namespace std;
using namespace webss;

ImportedDocument::ImportedDocument(Webss&& data) : data(move(data))
{
	assert(this->data.isTuple() && "import must reference a tuple");
	assert(this->data.getTuple().size() == 3);
#ifdef assert
	for (const auto& item : this->data.getTuple())
		assert(item.isString());
#endif	
}

bool ImportedDocument::operator==(const ImportedDocument& o) const { return data == o.data; }
bool ImportedDocument::operator!=(const ImportedDocument& o) const { return !(*this == o); }

const Webss& ImportedDocument::getData() const { return data; }
const string& ImportedDocument::getLink() const { return data.getTuple()[0].getString(); }

ParamDocument::ParamDocument() {}
ParamDocument::ParamDocument(ImportedDocument import) : type(Type::IMPORT), import(new ImportedDocument(move(import))) {}
ParamDocument::~ParamDocument() { destroyUnion(); }

ParamDocument::ParamDocument(ParamDocument&& o) { copyUnion(move(o)); }
ParamDocument::ParamDocument(const ParamDocument& o) { copyUnion(o); }

ParamDocument& ParamDocument::operator=(ParamDocument&& o)
{
	destroyUnion();
	copyUnion(move(o));
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

bool ParamDocument::operator==(const ParamDocument& o) const
{
	if (this == &o)
		return true;
	if (type != o.type || import != o.import)
		return false;
	switch (type)
	{
	case Type::NONE: case Type::IMPORT:
		return true;
	case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::EXPAND: case Type::SCOPED_IMPORT:
		return ent == o.ent;
	case Type::SCOPED_IMPORT_LIST:
		return entList == o.entList;
	default:
		assert(false); throw domain_error("");
	}
}
bool ParamDocument::operator!=(const ParamDocument& o) const
{
	return !(*this == o);
}

ParamDocument::Type ParamDocument::getType() const { return type; }
bool ParamDocument::hasNamespace() const { return type == Type::EXPAND; }
const Entity& ParamDocument::getEntity() const { assert(type != Type::SCOPED_IMPORT_LIST); return ent; }
const std::vector<Entity>& ParamDocument::getEntityList() const { assert(type == Type::SCOPED_IMPORT_LIST); return *entList; }
const Namespace& ParamDocument::getNamespace() const { return ent.getContent().getNamespace(); }
const ImportedDocument& ParamDocument::getImport() const { return *import; }

ParamDocument::ParamDocument(Entity ent, Type type) : type(type), ent(move(ent)) {}
ParamDocument::ParamDocument(Entity ent, Type type, ImportedDocument import) : type(type), ent(move(ent)), import(new ImportedDocument(move(import))) {}
ParamDocument::ParamDocument(vector<Entity> entList, ImportedDocument import) : type(Type::SCOPED_IMPORT_LIST), entList(new vector<Entity>(move(entList))), import(new ImportedDocument(move(import))) {}

void ParamDocument::destroyUnion()
{
	switch (type)
	{
	case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::EXPAND: case Type::SCOPED_IMPORT:
		ent.~Entity();
		break;
	case Type::SCOPED_IMPORT_LIST:
		delete entList;
		break;
	default:
		break;
	}

	type = Type::NONE;
	if (import != nullptr)
	{
		delete import;
		import = nullptr;
	}
}

void ParamDocument::copyUnion(ParamDocument&& o)
{
	switch (o.type)
	{
	case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::EXPAND: case Type::SCOPED_IMPORT:
		new (&ent) Entity(move(o.ent));
		o.ent.~Entity();
		break;
	case Type::SCOPED_IMPORT_LIST:
		entList = o.entList;
		break;
	default:
		break;
	}

	type = o.type;
	import = o.import;
	o.import = nullptr;
	o.type = Type::NONE;
}

void ParamDocument::copyUnion(const ParamDocument& o)
{
	switch (o.type)
	{
	case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::EXPAND: case Type::SCOPED_IMPORT:
		new (&ent) Entity(o.ent);
		break;
	case Type::SCOPED_IMPORT_LIST:
		entList = new vector<Entity>(*o.entList);
		break;
	default:
		break;
	}

	type = o.type;
	if (o.import != nullptr)
		import = new ImportedDocument(*o.import);
}