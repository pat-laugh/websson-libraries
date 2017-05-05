//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "serializer.h"

#include <cmath>
#include <cstdio>
#include <limits>
#include <type_traits>

#include "utils/constants.h"
#include "utils/utils.h"
#include "utils/utilsWebss.h"

using namespace std;
using namespace webss;

template <ConType::Enum CON> void putContainerStart(StringBuilder&) { assert(false); }
template <> void putContainerStart<ConType::DOCUMENT>(StringBuilder&) {}
template <> void putContainerStart<ConType::DICTIONARY>(StringBuilder& out) { out += OPEN_DICTIONARY; }
template <> void putContainerStart<ConType::LIST>(StringBuilder& out) { out += OPEN_LIST; }
template <> void putContainerStart<ConType::TUPLE>(StringBuilder& out) { out += OPEN_TUPLE; }
template <> void putContainerStart<ConType::TEMPLATE_HEAD>(StringBuilder& out) { out += OPEN_TEMPLATE; }

template <ConType::Enum CON> void putContainerEnd(StringBuilder&) { assert(false); }
template <> void putContainerEnd<ConType::DOCUMENT>(StringBuilder&) {}
template <> void putContainerEnd<ConType::DICTIONARY>(StringBuilder& out) { out += CLOSE_DICTIONARY; }
template <> void putContainerEnd<ConType::LIST>(StringBuilder& out) { out += CLOSE_LIST; }
template <> void putContainerEnd<ConType::TUPLE>(StringBuilder& out) { out += CLOSE_TUPLE; }
template <> void putContainerEnd<ConType::TEMPLATE_HEAD>(StringBuilder& out) { out += CLOSE_TEMPLATE; }

template <ConType::Enum CON>
class ContainerIncluder
{
private:
	StringBuilder& out;
public:
	ContainerIncluder(StringBuilder& out) : out(out) { putContainerStart<CON>(out); }
	~ContainerIncluder() { putContainerEnd<CON>(out); }
};

template <class Container, ConType::Enum CON>
void putSeparatedValues(StringBuilder& out, const Container& cont, function<void(typename Container::const_iterator it)> output)
{
	ContainerIncluder<CON> includer(out);
	if (!cont.empty())
	{
		typename Container::const_iterator it = cont.begin();
		output(it);
		while (++it != cont.end())
		{
			out += CHAR_SEPARATOR;
			output(it);
		}
	}
}

class SerializerTemplate : public Serializer
{
public:
	template <ConType::Enum CON>
	void putDocumentHead(StringBuilder& out, const DocumentHead& docHead)
	{
		putSeparatedValues<DocumentHead, CON>(out, docHead, [&](DocumentHead::const_iterator it)
		{
			using Type = decltype(it->getType());
			switch (it->getType())
			{
			case Type::ENTITY_ABSTRACT:
				putAbstractEntity(out, it->getEntity());
				break;
			case Type::ENTITY_CONCRETE:
				putConcreteEntity(out, it->getEntity(), CON);
				break;
			case Type::IMPORT:
				putImportedDocument(out, it->getImport(), CON);
				break;
			case Type::USING_ONE:
				//TODO
			//	putImportedDocument(out, it->getImportedDoc(), CON);
			//	break;
			case Type::USING_ALL:
				putUsingNamespace(out, it->getNamespace());
				break;
			case Type::SCOPED_DOCUMENT:
				putScopedDocument(out, it->getScopedDoc());
				break;
			default:
				assert(false); throw domain_error("");
			}
		});
	}

	void putTheadBinary(StringBuilder& out, const TemplateHeadBinary& thead)
	{
		putThead<TemplateHeadBinary, ParamBinary>(out, thead, [&](StringBuilder& out, const string& key, const ParamBinary& param) { putParamBinary(out, key, param); });
	}
	void putTheadStandard(StringBuilder& out, const TemplateHeadStandard& thead)
	{
		putThead<TemplateHeadStandard, ParamStandard>(out, thead, [&](StringBuilder& out, const string& key, const ParamStandard& param) { putParamStandard(out, key, param); });
	}
	void putTheadText(StringBuilder& out, const TemplateHeadStandard& thead)
	{
		out += ASSIGN_CONTAINER_STRING;
		putThead<TemplateHeadStandard, ParamStandard>(out, thead, [&](StringBuilder& out, const string& key, const ParamStandard& param) { putParamText(out, key, param); });
	}

	template <class Parameters>
	void putTemplateDictionary(StringBuilder& out, const Parameters& params, const Dictionary& dict, function<void(StringBuilder& out, const Parameters& params, const Tuple& tuple)>&& putTupleRegular, function<void(StringBuilder& out, const Parameters& params, const Tuple& tuple)>&& putTupleText)
	{
		static const auto CON = ConType::DICTIONARY;
		putSeparatedValues<Dictionary, CON>(out, dict, [&](typename Dictionary::const_iterator it)
		{
			out += it->first;
			const auto type = it->second.getTypeRaw();
			if (type == WebssType::LIST)
				putTemplateList<Parameters>(out, params, it->second.getListRaw(), move(putTupleRegular), move(putTupleText));
			else if (type == WebssType::TUPLE)
				putTupleRegular(out, params, it->second.getTupleRaw());
			else
			{
				assert(type == WebssType::TUPLE_TEXT);
				putTupleText(out, params, it->second.getTupleRaw());
			}
		});
	}

	template <class Parameters>
	void putTemplateList(StringBuilder& out, const Parameters& params, const List& list, function<void(StringBuilder& out, const Parameters& params, const Tuple& tuple)>&& putTupleRegular, function<void(StringBuilder& out, const Parameters& params, const Tuple& tuple)>&& putTupleText)
	{
		static const auto CON = ConType::DICTIONARY;
		putSeparatedValues<List, CON>(out, list, [&](typename List::const_iterator it)
		{
			const auto type = it->getTypeRaw();
			if (type == WebssType::TUPLE)
				putTupleRegular(out, params, it->getTupleRaw());
			else
			{
				assert(type == WebssType::TUPLE_TEXT);
				putTupleText(out, params, it->getTupleRaw());
			}
		});
	}
private:
	template <class TemplateHead, class Param>
	void putThead(StringBuilder& out, const TemplateHead& thead, function<void(StringBuilder& out, const string& key, const Param& param)>&& putParam)
	{
		static const auto CON = ConType::TEMPLATE_HEAD;
		assert(!thead.empty() && "template head can't be empty");
		if (thead.hasEntity())
		{
			ContainerIncluder<CON> includer(out);
			putEntityName(out, thead.getEntity());
		}
		else
		{
			auto&& keyValues = thead.getParameters().getOrderedKeyValues();
			using Type = typename remove_reference<decltype(keyValues)>::type;
			putSeparatedValues<Type, CON>(out, keyValues, [&](typename Type::const_iterator it)
			{
				assert(it->first != nullptr && "can't have anonymous key in dictionary, template head or enum");
				putParam(out, *it->first, *it->second);
			});
		}
	}

	void putParamBinary(StringBuilder& out, const string& key, const ParamBinary& param)
	{
		const auto& bhead = param.getSizeHead();
		{
			ContainerIncluder<ConType::TUPLE> includer(out);
			using Type = ParamBinary::SizeHead::Type;
			switch (bhead.getType())
			{
			case Type::EMPTY:
				break;
			case Type::KEYWORD:
				out += bhead.getKeyword().toString();
				break;
			case Type::NUMBER:
				out += to_string(bhead.size());
				break;
			case Type::TEMPLATE_HEAD:
				putTheadBinary(out, bhead.getTemplateHead());
				break;
			case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_TEMPLATE_HEAD:
				putEntityName(out, bhead.getEntity());
				break;
			case Type::SELF:
				putTheadSelf(out);
				break;
			default:
				assert(false); throw domain_error("");
			}
			putBinarySizeList(out, param.getSizeList());
		}

		out += key;
		if (bhead.hasDefaultValue())
			putCharValue(out, bhead.getDefaultValue(), ConType::TEMPLATE_HEAD);
		else
			assert(!bhead.isSelf());
	}
	void putParamStandard(StringBuilder& out, const string& key, const ParamStandard& param)
	{
		if (param.hasTemplateHead())
			switch (param.getTypeThead())
			{
			case WebssType::TEMPLATE_HEAD_BINARY:
				putTheadBinary(out, param.getTemplateHeadBinary());
				break;
			case WebssType::TEMPLATE_HEAD_SCOPED:
				putTheadScoped(out, param.getTemplateHeadScoped());
				break;
			case WebssType::TEMPLATE_HEAD_SELF:
				putTheadSelf(out);
				break;
			case WebssType::TEMPLATE_HEAD_STANDARD:
				putTheadStandard(out, param.getTemplateHeadStandard());
				break;
			case WebssType::TEMPLATE_HEAD_TEXT:
				putTheadText(out, param.getTemplateHeadStandard());
				break;
			default:
				break;
			}

		out += key;
		if (param.hasDefaultValue())
			putCharValue(out, param.getDefaultValue(), ConType::TEMPLATE_HEAD);
		else
			assert(param.getTypeThead() != WebssType::TEMPLATE_HEAD_SELF);
	}
	void putParamText(StringBuilder& out, const string& key, const ParamStandard& param)
	{
		out += key;
		if (param.hasDefaultValue())
		{
			const auto& webss = param.getDefaultValue();
			assert(webss.getType() == WebssType::PRIMITIVE_STRING && "template head text parameters' values can only be of type string");
			putCharValue(out, webss, ConType::TEMPLATE_HEAD);
		}
	}

	void putBinarySizeList(StringBuilder& out, const ParamBinary::SizeList& blist)
	{
		using Type = ParamBinary::SizeList::Type;
		if (blist.isOne())
			return;

		ContainerIncluder<ConType::LIST> includer(out);
		switch (blist.getType())
		{
		case Type::EMPTY:
			break;
		case Type::NUMBER:
			out += to_string(blist.size());
			break;
		case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
			putEntityName(out, blist.getEntity());
			break;
		default:
			assert(false); throw domain_error("");
		}
	}
};

void Serializer::putPreviousNamespaceNames(StringBuilder& out, const Namespace& nspace)
{
	const auto& nspaces = nspace.getNamespaces();
	if (!nspaces.empty())
	{
		//check the list of namespaces that the entity has in a reverse order until
		//there is an namespace that is in the current scope
		//then precede the entity's name with all the namespaces that were not in the current scope

		auto i = nspaces.size();
		while (i > 0 && !namespaceCurrentScope(*nspaces[i - 1]))
			--i;

		while (i < nspaces.size())
			out += nspaces[i++]->getName() + CHAR_SCOPE;
	}
}

bool Serializer::namespaceCurrentScope(const Namespace& nspace)
{
	return currentNamespaces.find(nspace.getPointer().get()) != currentNamespaces.end();
}

void Serializer::putEntityName(StringBuilder& out, const Entity& ent)
{
	if (ent.hasNamespace())
	{
		const auto& nspace = ent.getNamespace();
		if (!namespaceCurrentScope(nspace))
		{
			putPreviousNamespaceNames(out, nspace);
			out += nspace.getName();
			out += CHAR_SCOPE;
		}
	}
	out += ent.getName();
}

void Serializer::putAbstractValue(StringBuilder& out, const Webss& webss)
{
	switch (webss.getTypeRaw())
	{
	case WebssType::TEMPLATE_HEAD_BINARY:
		putTheadBinary(out, webss.getTemplateHeadBinaryRaw());
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		putTheadScoped(out, webss.getTemplateHeadScopedRaw());
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		putTheadStandard(out, webss.getTemplateHeadStandardRaw());
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		putTheadText(out, webss.getTemplateHeadStandardRaw());
		break;
	case WebssType::ENTITY:
		assert(webss.getEntityRaw().getContent().isAbstract());
		putEntityName(out, webss.getEntityRaw());
		break;
	case WebssType::NAMESPACE:
		putNamespace(out, webss.getNamespaceRaw());
		break;
	case WebssType::ENUM:
		putEnum(out, webss.getEnumRaw());
		break;
	case WebssType::BLOCK_HEAD:
		putBlockHead(out, webss.getBlockHeadRaw());
		break;
	default:
		assert(false && "type is not an abstract value"); throw domain_error("");
	}
}

void Serializer::putConcreteValue(StringBuilder& out, const Webss& webss, ConType con)
{
	switch (webss.getTypeRaw())
	{
	case WebssType::PRIMITIVE_NULL:
		out += 'N';
		break;
	case WebssType::PRIMITIVE_BOOL:
		out += webss.getBoolRaw() ? 'T' : 'F';
		break;
	case WebssType::PRIMITIVE_INT:
		putInt(out, webss.getIntRaw());
		break;
	case WebssType::PRIMITIVE_DOUBLE:
		putDouble(out, webss.getDoubleRaw());
		break;
	case WebssType::PRIMITIVE_STRING:
		out += CHAR_COLON;
		putLineString(out, webss.getStringRaw(), con);
		break;
	case WebssType::DICTIONARY:
		putDictionary(out, webss.getDictionaryRaw());
		break;
	case WebssType::LIST:
		putList(out, webss.getListRaw());
		break;
	case WebssType::LIST_TEXT:
		putListText(out, webss.getListRaw());
		break;
	case WebssType::TUPLE:
		putTuple(out, webss.getTupleRaw());
		break;
	case WebssType::TUPLE_TEXT:
		putTupleText(out, webss.getTupleRaw());
		break;
	case WebssType::TEMPLATE_BINARY:
		putFuncBinary(out, webss.getTemplateBinaryRaw());
		break;
	case WebssType::TEMPLATE_SCOPED:
		putFuncScoped(out, webss.getTemplateScopedRaw(), con);
		break;
	case WebssType::TEMPLATE_STANDARD:
		putFuncStandard(out, webss.getTemplateStandardRaw());
		break;
	case WebssType::TEMPLATE_TEXT:
		putFuncText(out, webss.getTemplateStandardRaw());
		break;
	case WebssType::ENTITY:
		assert(webss.getEntityRaw().getContent().isConcrete());
		putEntityName(out, webss.getEntityRaw());
		break;
	case WebssType::BLOCK:
		putBlock(out, webss.getBlockRaw(), con);
		break;
	default:
		assert(false && "type is not a concrete value"); throw domain_error("");
	}
}

void Serializer::putCharValue(StringBuilder& out, const Webss& value, ConType con)
{
	switch (value.getTypeRaw())
	{
	case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT:
	case WebssType::PRIMITIVE_DOUBLE: case WebssType::ENTITY:
		out += CHAR_EQUAL;
	default:
		putConcreteValue(out, value, con);
		break;
	}
}

void Serializer::putKeyValue(StringBuilder& out, const string& key, const Webss& value, ConType con)
{
	assert(value.getTypeRaw() != WebssType::DEFAULT && "can't serialize this type with key");

	out += key;
	putCharValue(out, value, con);
}

void Serializer::putInt(StringBuilder& out, WebssInt i)
{
	assert(i != numeric_limits<WebssInt>::min());
	out += to_string(i);
}

void Serializer::putDouble(StringBuilder& out, double d)
{
	assert(std::isfinite(d));
	char buffer[32];
	int num = snprintf(buffer, 32, "%.17g", d);
	assert(num > 0 && num < 32);
	out += buffer;
}

void addCharEscape(StringBuilder& out, char c)
{
	out += '\\';
	switch (c)
	{
	case '\0': out += '0'; break;
	case '\a': out += 'a'; break;
	case '\b': out += 'b'; break;
	case '\f': out += 'f'; break;
	case '\n': out += 'n'; break;
	case '\r': out += 'r'; break;
	case '\t': out += 't'; break;
	case '\v': out += 'v'; break;

	case ' ': out += 's'; break;
		//no empty char ('\e')!

	default:
		if (!isControlAscii(c))
		{
			assert(isSpecialAscii(c));
			out += c;
		}
		else
		{
			out += 'x';
			out += hexToChar(c >> 4);
			out += hexToChar(c & 0x0F);
		}
	}
}

bool isLineEnd(char c, ConType con)
{
	return c == '\n' || c == CHAR_SEPARATOR || con.isEnd(c);
}

//whether or not the char should always be escaped in a char
//includes Ascii characers, '\\' and '?'
bool isMustEscapeChar(char c)
{
	return c == '?' || c == '\\' || isControlAscii(c);
}

void Serializer::putLineString(StringBuilder& out, const string& str, ConType con)
{
	if (str.empty())
		return;

	auto it = str.begin();
	//first char can't be ':'; first and last can't be ' '
	if (*it == ' ' || *it == ':')
	{
		addCharEscape(out, *it);
		if (++it == str.end())
			return;
	}

	do
	{
		if (isMustEscapeChar(*it) || isLineEnd(*it, con))
			addCharEscape(out, *it);
		else if (*it == '/') //escape comment
		{
			out += '/';
			if (++it == str.end())
				return;

			if (*it == '/' || *it == '*' || isMustEscapeChar(*it) || isLineEnd(*it, con))
				addCharEscape(out, *it);
			else
				out += *it;
		}
		else if (*it == ' ' && it + 1 == str.end())
			out += "\\s";
		else
			out += *it;
	} while (++it != str.end());
}

void Serializer::putCstring(StringBuilder& out, const string& str)
{
	out += CHAR_CSTRING;
	for (auto it = str.begin(); it != str.end(); ++it)
		if (isMustEscapeChar(*it) || *it == CHAR_CSTRING)
			addCharEscape(out, *it);
		else
			out += *it;
	out += CHAR_CSTRING;
}

void Serializer::putDocument(StringBuilder& out, const Document& doc)
{
	static const auto CON = ConType::DOCUMENT;

	if (doc.getHead().size() > 0)
	{
		putDocumentHead(out, doc.getHead());
		out += '\n';
	}

	using Type = decltype(doc.getOrderedKeyValues());
	putSeparatedValues<Type, CON>(out, doc.getOrderedKeyValues(), [&](Type::const_iterator it)
	{
		if (it->first == nullptr)
			putConcreteValue(out, *it->second, CON);
		else
			putKeyValue(out, *it->first, *it->second, CON);
	});
}

void Serializer::putDocumentHead(StringBuilder& out, const DocumentHead& docHead)
{
	static_cast<SerializerTemplate*>(this)->putDocumentHead<ConType::DOCUMENT>(out, docHead);
}

void Serializer::putAbstractEntity(StringBuilder& out, const Entity& ent)
{
	const auto& content = ent.getContent();
	assert(content.isAbstract());
	out += CHAR_ABSTRACT_ENTITY;
	putEntityName(out, ent);
	putAbstractValue(out, content);
}

void Serializer::putConcreteEntity(StringBuilder& out, const Entity& ent, ConType con)
{
	const auto& content = ent.getContent();
	assert(content.isConcrete());
	out += CHAR_CONCRETE_ENTITY;
	putEntityName(out, ent);
	putCharValue(out, content, con);
}

void Serializer::putImportedDocument(StringBuilder& out, const ImportedDocument& importDoc, ConType con)
{
	const auto& name = importDoc.getName();
	assert(name.isString());
	out += CHAR_IMPORT;
	if (name.getTypeRaw() == WebssType::PRIMITIVE_STRING)
		putCstring(out, name.getStringRaw());
	else
		putConcreteValue(out, name, con);
}

void Serializer::putScopedDocument(StringBuilder& out, const ScopedDocument& scopedDoc)
{
	out += CHAR_USING_ONE;
	putTheadScoped(out, scopedDoc.head);
	NamespaceIncluder includer(currentNamespaces, scopedDoc.head.getParameters());
	static_cast<SerializerTemplate*>(this)->putDocumentHead<ConType::DICTIONARY>(out, scopedDoc.body);
}

void Serializer::putUsingNamespace(StringBuilder& out, const Namespace& nspace)
{
	out += CHAR_USING_ONE;
	putPreviousNamespaceNames(out, nspace);
	out += nspace.getName();
}

void Serializer::putNamespace(StringBuilder& out, const Namespace& nspace)
{
	static const auto CON = ConType::DICTIONARY;
	NamespaceIncluder includer(currentNamespaces, nspace);
	putSeparatedValues<Namespace, CON>(out, nspace, [&](Namespace::const_iterator it)
	{
		if (it->getContent().isAbstract())
			putAbstractEntity(out, *it);
		else
			putConcreteEntity(out, *it, CON);
	});
}

void Serializer::putEnum(StringBuilder& out, const Enum& tEnum)
{
	static const auto CON = ConType::LIST;
	NamespaceIncluder includer(currentNamespaces, tEnum);
	putSeparatedValues<Enum, CON>(out, tEnum, [&](Enum::const_iterator it) { putEntityName(out, *it); });
}

void Serializer::putBlockHead(StringBuilder& out, const BlockHead& blockHead)
{
	out += OPEN_TEMPLATE;
	if (blockHead.hasEntity())
		putEntityName(out, blockHead.getEntity());
	out += CLOSE_TEMPLATE;
}

void Serializer::putDictionary(StringBuilder& out, const Dictionary& dict)
{
	static const auto CON = ConType::DICTIONARY;
	putSeparatedValues<Dictionary, CON>(out, dict, [&](Dictionary::const_iterator it) { putKeyValue(out, it->first, it->second, CON); });
}

void Serializer::putList(StringBuilder& out, const List& list)
{
	static const auto CON = ConType::LIST;
	putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it) { putConcreteValue(out, *it, CON); });
}

void Serializer::putListText(StringBuilder& out, const List& list)
{
	static const auto CON = ConType::LIST;
	out += ASSIGN_CONTAINER_STRING;
	putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it) { putLineString(out, it->getString(), CON); });
}

void Serializer::putTuple(StringBuilder& out, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	using Type = decltype(tuple.getOrderedKeyValues());
	putSeparatedValues<Type, CON>(out, tuple.getOrderedKeyValues(), [&](Type::const_iterator it)
	{
		if (it->first == nullptr)
			putConcreteValue(out, *it->second, CON);
		else
			putKeyValue(out, *it->first, *it->second, CON);
	});
}

void Serializer::putTupleText(StringBuilder& out, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	out += ASSIGN_CONTAINER_STRING;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it) { putLineString(out, it->getString(), CON); });
}

void Serializer::putTheadScoped(StringBuilder& out, const TemplateHeadScoped& thead)
{
	static const auto CON = ConType::TEMPLATE_HEAD;
	ContainerIncluder<CON> includer(out);
	if (thead.hasEntity())
		putEntityName(out, thead.getEntity());
	else
	{
		using Type = remove_reference<decltype(thead.getParameters())>::type;
		putSeparatedValues<Type, CON>(out, thead.getParameters(), [&](Type::const_iterator it)
		{
			using ParamType = decltype(it->getType());
			switch (it->getType())
			{
			case ParamType::ENTITY_ABSTRACT:
				putAbstractEntity(out, it->getEntity());
				break;
			case ParamType::ENTITY_CONCRETE:
				putConcreteEntity(out, it->getEntity(), CON);
				break;
			case ParamType::USING_ONE:
				//TODO
			//	putConcreteEntity(out, it->getConcreteEntity(), CON);
			//	break;
			case ParamType::USING_ALL:
				putUsingNamespace(out, it->getNamespace());
				break;
			default:
				assert(false); throw domain_error("");
			}
		});
	}
}
void Serializer::putTheadSelf(StringBuilder& out)
{
	ContainerIncluder<ConType::TEMPLATE_HEAD> includer(out);
	out += CHAR_SELF;
}
void Serializer::putTheadBinary(StringBuilder& out, const TemplateHeadBinary& thead)
{
	static_cast<SerializerTemplate*>(this)->putTheadBinary(out, thead);
}
void Serializer::putTheadStandard(StringBuilder& out, const TemplateHeadStandard& thead)
{
	static_cast<SerializerTemplate*>(this)->putTheadStandard(out, thead);
}
void Serializer::putTheadText(StringBuilder& out, const TemplateHeadStandard& thead)
{
	static_cast<SerializerTemplate*>(this)->putTheadText(out, thead);
}

void Serializer::putFuncBinary(StringBuilder& out, const TemplateBinary& templ)
{
	auto putTupleRegular = [&](StringBuilder& out, const TemplateHeadBinary::Parameters& params, const Tuple& tuple) { putFuncBinaryTuple(out, params, tuple); };
	auto putTupleText = [&](StringBuilder& out, const TemplateHeadBinary::Parameters& params, const Tuple& tuple) { putFuncBinaryTuple(out, params, tuple); };

	putTheadBinary(out, templ);
	const auto& params = templ.getParameters();
	switch (templ.getType())
	{
	case WebssType::DICTIONARY:
		static_cast<SerializerTemplate*>(this)->putTemplateDictionary<TemplateHeadBinary::Parameters>(out, params, templ.getDictionary(), move(putTupleRegular), move(putTupleText));
		break;
	case WebssType::LIST:
		static_cast<SerializerTemplate*>(this)->putTemplateList<TemplateHeadBinary::Parameters>(out, params, templ.getList(), move(putTupleRegular), move(putTupleText));
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		putFuncBinaryTuple(out, params, templ.getTuple());
		break;
	default:
		assert(false); throw domain_error("");
	}
}

void Serializer::putFuncStandard(StringBuilder& out, const TemplateStandard& templ)
{
	putTheadStandard(out, templ);
	putFuncStandardBody(out, templ.getParameters(), templ.getWebss());
}

void Serializer::putFuncStandardBody(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Webss& body)
{
	auto putTupleRegular = [&](StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple) { putFuncStandardTuple(out, params, tuple); };
	auto putTupleText = [&](StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple) { putFuncStandardTupleText(out, params, tuple); };
	switch (body.getTypeRaw())
	{
	case WebssType::DICTIONARY:
		static_cast<SerializerTemplate*>(this)->putTemplateDictionary<TemplateHeadStandard::Parameters>(out, params, body.getDictionaryRaw(), move(putTupleRegular), move(putTupleText));
		break;
	case WebssType::LIST:
		static_cast<SerializerTemplate*>(this)->putTemplateList<TemplateHeadStandard::Parameters>(out, params, body.getListRaw(), move(putTupleRegular), move(putTupleText));
		break;
	case WebssType::TUPLE:
		putFuncStandardTuple(out, params, body.getTupleRaw());
		break;
	case WebssType::TUPLE_TEXT:
		putFuncStandardTupleText(out, params, body.getTupleRaw());
		break;
	default:
		assert(false); throw domain_error("");
	}
}

void Serializer::putFuncText(StringBuilder& out, const TemplateStandard& templ)
{
	auto putTupleRegular = [&](StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple) { putFuncTextTuple(out, params, tuple); };
	auto putTupleText = [&](StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple) { putFuncTextTuple(out, params, tuple); };

	putTheadText(out, templ);
	const auto& params = templ.getParameters();
	switch (templ.getType())
	{
	case WebssType::DICTIONARY:
		static_cast<SerializerTemplate*>(this)->putTemplateDictionary<TemplateHeadStandard::Parameters>(out, params, templ.getDictionary(), move(putTupleRegular), move(putTupleText));
		break;
	case WebssType::LIST:
		static_cast<SerializerTemplate*>(this)->putTemplateList<TemplateHeadStandard::Parameters>(out, params, templ.getList(), move(putTupleRegular), move(putTupleText));
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		putFuncTextTuple(out, params, templ.getTuple());
		break;
	default:
		assert(false); throw domain_error("");
	}
}

extern void putFuncBodyBinary(StringBuilder&, const TemplateHeadBinary::Parameters&, const Tuple&);

void Serializer::putFuncBinaryTuple(StringBuilder& out, const TemplateHeadBinary::Parameters& params, const Tuple& tuple)
{
	ContainerIncluder<ConType::TUPLE> includer(out);
	putFuncBodyBinary(out, params, tuple);
}
void Serializer::putFuncStandardTuple(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");

	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		const auto& param = params[i++];
		if (param.hasTemplateHead())
			putFuncStandardBody(out, param.getTemplateHeadStandard().getParameters(), *it);
		else
		{
			if (it->getTypeRaw() == WebssType::NONE || it->getTypeRaw() == WebssType::DEFAULT)
				assert(param.hasDefaultValue());
			else
				putConcreteValue(out, *it, CON);
		}
	});
}
void Serializer::putFuncStandardTupleText(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");

	out += ASSIGN_CONTAINER_STRING;
	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		const auto& param = params[i++];
		assert(!param.hasTemplateHead());
		switch (it->getTypeRaw())
		{
		case WebssType::NONE: case WebssType::DEFAULT:
			assert(param.hasDefaultValue());
			break;
		case WebssType::PRIMITIVE_STRING:
			putLineString(out, it->getStringRaw(), CON);
			break;
		default:
			assert(false); throw domain_error("");
		}
	});
#ifdef assert
	while (i < params.size())
		assert(params[i++].hasDefaultValue());
#endif
}
void Serializer::putFuncTextTuple(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");

	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		const auto& param = params[i++];
		switch (it->getTypeRaw())
		{
		case WebssType::NONE: case WebssType::DEFAULT:
			assert(param.hasDefaultValue());
			break;
		case WebssType::PRIMITIVE_STRING:
			putLineString(out, it->getStringRaw(), CON);
			break;
		default:
			assert(false); throw domain_error("");
		}
	});
}

void Serializer::putBlock(StringBuilder& out, const Block& block, ConType con)
{
	if (block.hasEntity())
	{
		putEntityName(out, block.getEntity());
		putCharValue(out, block.getValue(), con);
	}
	else
	{
		out += OPEN_TEMPLATE;
		out += CLOSE_TEMPLATE;
		putConcreteValue(out, block.getValue(), con);
	}
}

void Serializer::putFuncScoped(StringBuilder& out, const TemplateScoped& templ, ConType con)
{
	NamespaceIncluder includer(currentNamespaces, templ.getParameters());
	if (templ.hasEntity())
	{
		putEntityName(out, templ.getEntity());
		putCharValue(out, templ.getValue(), con);
	}
	else
	{
		putTheadScoped(out, templ);
		putConcreteValue(out, templ.getValue(), con);
	}
}
