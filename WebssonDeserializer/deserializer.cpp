//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"
#include <type_traits>

using namespace std;
using namespace webss;

template <ConType::Enum CON> void putContainerStart(StringBuilder& out) = delete;
template <> void putContainerStart<ConType::DOCUMENT>(StringBuilder& out) {}
template <> void putContainerStart<ConType::DICTIONARY>(StringBuilder& out) { out += OPEN_DICTIONARY; }
template <> void putContainerStart<ConType::LIST>(StringBuilder& out) { out += OPEN_LIST; }
template <> void putContainerStart<ConType::TUPLE>(StringBuilder& out) { out += OPEN_TUPLE; }
template <> void putContainerStart<ConType::TEMPLATE_HEAD>(StringBuilder& out) { out += OPEN_TEMPLATE; }

template <ConType::Enum CON> void putContainerEnd(StringBuilder& out) = delete;
template <> void putContainerEnd<ConType::DOCUMENT>(StringBuilder& out) {}
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

class DeserializerTemplate : public Deserializer
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
				putAbstractEntity(out, it->getAbstractEntity(), CON);
				break;
			case Type::ENTITY_CONCRETE:
				putConcreteEntity(out, it->getConcreteEntity(), CON);
				break;
			case Type::IMPORT:
				putImportedDocument(out, it->getImportedDoc(), CON);
				break;
			case Type::NAMESPACE:
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

	void putFheadBinary(StringBuilder& out, const TemplateHeadBinary& thead)
	{
		putFhead<TemplateHeadBinary, TemplateHeadBinary::Param>(out, thead, [&](StringBuilder& out, const string& key, const TemplateHeadBinary::Param& param) { putParamBinary(out, key, param); });
	}
	void putFheadStandard(StringBuilder& out, const TemplateHeadStandard& thead)
	{
		putFhead<TemplateHeadStandard, TemplateHeadStandard::Param>(out, thead, [&](StringBuilder& out, const string& key, const TemplateHeadStandard::Param& param) { putParamStandard(out, key, param); });
	}
	void putFheadText(StringBuilder& out, const TemplateHeadText& thead)
	{
		out += ASSIGN_CONTAINER_STRING;
		putFhead<TemplateHeadText, TemplateHeadText::Param>(out, thead, [&](StringBuilder& out, const string& key, const TemplateHeadText::Param& param) { putParamText(out, key, param); });
	}
private:
	template <class TemplateHead, class Param>
	void putFhead(StringBuilder& out, const TemplateHead& thead, function<void(StringBuilder& out, const string& key, const Param& param)>&& putParam)
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
				assert(it->first != nullptr && ERROR_ANONYMOUS_KEY);
				putParam(out, *it->first, *it->second);
			});
		}
	}

	void putParamBinary(StringBuilder& out, const string& key, const ParamBinary& param)
	{
		auto&& bhead = param.sizeHead;
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
				putFheadBinary(out, bhead.getTemplateHead());
				break;
			case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_TEMPLATE_HEAD:
				putEntityName(out, bhead.getEntity());
				break;
			case Type::SELF:
				putFheadSelf(out);
				break;
			default:
				assert(false); throw domain_error("");
			}
			putBinarySizeList(out, param.sizeList);
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
			switch (param.getTypeFhead())
			{
			case WebssType::TEMPLATE_HEAD_BINARY:
				putFheadBinary(out, param.getTemplateHeadBinary());
				break;
			case WebssType::TEMPLATE_HEAD_SCOPED:
				putFheadScoped(out, param.getTemplateHeadScoped());
				break;
			case WebssType::TEMPLATE_HEAD_SELF:
				putFheadSelf(out);
				break;
			case WebssType::TEMPLATE_HEAD_STANDARD:
				putFheadStandard(out, param.getTemplateHeadStandard());
				break;
			case WebssType::TEMPLATE_HEAD_TEXT:
				putFheadText(out, param.getTemplateHeadText());
				break;
			default:
				break;
			}

		out += key;
		if (param.hasDefaultValue())
			putCharValue(out, param.getDefaultValue(), ConType::TEMPLATE_HEAD);
		else
			assert(param.getTypeFhead() != WebssType::TEMPLATE_HEAD_SELF);
	}
	void putParamText(StringBuilder& out, const string& key, const ParamText& param)
	{
		out += key;
		if (param.hasDefaultValue())
		{
			auto&& webss = param.getDefaultValue();
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

void Deserializer::putNamespaceName(StringBuilder& out, const Namespace& nspace)
{
	auto&& nspaces = nspace.getNamespaces();
	if (!nspaces.empty())
		for (auto it = nspaces.begin(); it != nspaces.end(); ++it)
			if (currentNamespaces.find(it->get()) == currentNamespaces.end())
			{
				do
					out += (*it)->getName() + CHAR_SCOPE;
				while (++it != nspaces.end());
				break;
			}
	out += nspace.getName();
}

void Deserializer::putEntityName(StringBuilder& out, const Entity& ent)
{
	if (ent.hasNamespace())
	{
		putNamespaceName(out, ent.getNamespace());
		out += CHAR_SCOPE;
	}
	out += ent.getName();
}

void Deserializer::putAbstractValue(StringBuilder& out, const Webss& webss, ConType con)
{
	switch (webss.type)
	{
	case WebssType::TEMPLATE_HEAD_BINARY:
		putFheadBinary(out, *webss.theadBinary);
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		putFheadScoped(out, *webss.theadScoped);
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		putFheadStandard(out, *webss.theadStandard);
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		putFheadText(out, *webss.theadText);
		break;
	case WebssType::ENTITY:
		assert(webss.ent.getContent().isAbstract());
		putEntityName(out, webss.ent);
		break;
	case WebssType::NAMESPACE:
		putNamespace(out, *webss.nspace);
		break;
	case WebssType::ENUM:
		putEnum(out, *webss.tEnum);
		break;
	case WebssType::BLOCK_HEAD:
		putBlockHead(out, *webss.blockHead);
		break;
	default:
		assert(false && "type is not an abstract value"); throw domain_error("");
	}
}

void Deserializer::putConcreteValue(StringBuilder& out, const Webss& webss, ConType con)
{
	switch (webss.type)
	{
	case WebssType::PRIMITIVE_NULL:
		out += 'N';
		break;
	case WebssType::PRIMITIVE_BOOL:
		out += webss.tBool ? 'T' : 'F';
		break;
	case WebssType::PRIMITIVE_INT:
		out += to_string(webss.tInt);
		break;
	case WebssType::PRIMITIVE_DOUBLE:
		out += to_string(webss.tDouble);
		break;
	case WebssType::PRIMITIVE_STRING:
		out += CHAR_COLON;
		putLineString(out, *webss.tString, con);
		break;
	case WebssType::DICTIONARY:
		putDictionary(out, *webss.dict);
		break;
	case WebssType::LIST:
		putList(out, *webss.list);
		break;
	case WebssType::TUPLE:
		putTuple(out, *webss.tuple);
		break;
	case WebssType::TEMPLATE_BINARY:
		putFuncBinary(out, *webss.templBinary);
		break;
	case WebssType::TEMPLATE_SCOPED:
		putFuncScoped(out, *webss.templScoped, con);
		break;
	case WebssType::TEMPLATE_STANDARD:
		putFuncStandard(out, *webss.templStandard);
		break;
	case WebssType::TEMPLATE_TEXT:
		putFuncText(out, *webss.templText);
		break;
	case WebssType::ENTITY:
		assert(webss.ent.getContent().isConcrete());
		putEntityName(out, webss.ent);
		break;
	case WebssType::BLOCK:
		putBlock(out, *webss.block, con);
		break;
	default:
		assert(false && "type is not a concrete value"); throw domain_error("");
	}
}

void Deserializer::putCharValue(StringBuilder& out, const Webss& value, ConType con)
{
	switch (value.type)
	{
	case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT:
	case WebssType::PRIMITIVE_DOUBLE: case WebssType::ENTITY:
		out += CHAR_EQUAL;
	default:
		putConcreteValue(out, value, con);
		break;
	}
}

void Deserializer::putKeyValue(StringBuilder& out, const string& key, const Webss& value, ConType con)
{
	assert(value.type != WebssType::DEFAULT && "can't deserialize this type with key");

	out += key;
	putCharValue(out, value, con);
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

void Deserializer::putLineString(StringBuilder& out, const string& str, ConType con)
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

void Deserializer::putCstring(StringBuilder& out, const string& str)
{
	out += CHAR_CSTRING;
	if (!str.empty())
		for (auto it = str.begin(); it != str.end(); ++it)
			if (isMustEscapeChar(*it) || *it == CHAR_CSTRING)
				addCharEscape(out, *it);
			else
				out += *it;
	out += CHAR_CSTRING;
}

void Deserializer::putDocument(StringBuilder& out, const Document& doc)
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

void Deserializer::putDocumentHead(StringBuilder& out, const DocumentHead& docHead)
{
	static_cast<DeserializerTemplate*>(this)->putDocumentHead<ConType::DOCUMENT>(out, docHead);
}

void Deserializer::putAbstractEntity(StringBuilder& out, const Entity& ent, ConType con)
{
	auto&& content = ent.getContent();
	assert(content.isAbstract());
	out += CHAR_ABSTRACT_ENTITY;
	putEntityName(out, ent);
	putAbstractValue(out, content, con);
}

void Deserializer::putConcreteEntity(StringBuilder& out, const Entity& ent, ConType con)
{
	auto&& content = ent.getContent();
	assert(content.isConcrete());
	out += CHAR_CONCRETE_ENTITY;
	putEntityName(out, ent);
	putCharValue(out, content, con);
}

void Deserializer::putImportedDocument(StringBuilder& out, const ImportedDocument& importDoc, ConType con)
{
	auto&& name = importDoc.getName();
	assert(name.isString());
	out += CHAR_IMPORT;
	if (name.type == WebssType::PRIMITIVE_STRING)
		putCstring(out, *name.tString);
	else
		putConcreteValue(out, name, con);
}

void Deserializer::putScopedDocument(StringBuilder& out, const ScopedDocument& scopedDoc)
{
	out += CHAR_SCOPED_DOCUMENT;
	putFheadScoped(out, scopedDoc.head);
	NamespaceIncluder includer(currentNamespaces, scopedDoc.head.getParameters());
	static_cast<DeserializerTemplate*>(this)->putDocumentHead<ConType::DICTIONARY>(out, scopedDoc.body);
}

void Deserializer::putUsingNamespace(StringBuilder& out, const Namespace& nspace)
{
	out += CHAR_USING_NAMESPACE;
	putNamespaceName(out, nspace);
}

void Deserializer::putNamespace(StringBuilder& out, const Namespace& nspace)
{
	static const auto CON = ConType::DICTIONARY;
	NamespaceIncluder includer(currentNamespaces, nspace);
	putSeparatedValues<Namespace, CON>(out, nspace, [&](Namespace::const_iterator it)
	{
		if (it->getContent().isAbstract())
			putAbstractEntity(out, *it, CON);
		else
			putConcreteEntity(out, *it, CON);
	});
}

void Deserializer::putEnum(StringBuilder& out, const Enum& tEnum)
{
	static const auto CON = ConType::LIST;
	putSeparatedValues<Enum, CON>(out, tEnum, [&](Enum::const_iterator it) { putEntityName(out, *it); });
}

void Deserializer::putBlockHead(StringBuilder& out, const BlockHead& blockHead)
{
	out += OPEN_TEMPLATE;
	if (blockHead.hasEntity())
		putEntityName(out, blockHead.getEntity());
	out += CLOSE_TEMPLATE;
}

void Deserializer::putDictionary(StringBuilder& out, const Dictionary& dict)
{
	static const auto CON = ConType::DICTIONARY;
	putSeparatedValues<Dictionary, CON>(out, dict, [&](Dictionary::const_iterator it) { putKeyValue(out, it->first, it->second, CON); });
}

void Deserializer::putList(StringBuilder& out, const List& list)
{
	static const auto CON = ConType::LIST;
	if (list.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it) { putLineString(out, it->getString(), CON); });
	}
	else
		putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it) { putConcreteValue(out, *it, CON); });
}

void Deserializer::putTuple(StringBuilder& out, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	if (tuple.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it) { putLineString(out, it->getString(), CON); });
	}
	else
	{
		using Type = decltype(tuple.getOrderedKeyValues());
		putSeparatedValues<Type, CON>(out, tuple.getOrderedKeyValues(), [&](Type::const_iterator it)
		{
			if (it->first == nullptr)
				putConcreteValue(out, *it->second, CON);
			else
				putKeyValue(out, *it->first, *it->second, CON);
		});
	}
}

void Deserializer::putFheadBinary(StringBuilder& out, const TemplateHeadBinary& thead)
{
	static_cast<DeserializerTemplate*>(this)->putFheadBinary(out, thead);
}
void Deserializer::putFheadScoped(StringBuilder& out, const TemplateHeadScoped& thead)
{
	out += OPEN_TEMPLATE;
	if (thead.hasEntity())
		putEntityName(out, thead.getEntity());
	else
	{
		static const auto CON = ConType::TEMPLATE_HEAD;
		using Type = remove_reference<decltype(thead.getParameters())>::type;
		putSeparatedValues<Type, CON>(out, thead.getParameters(), [&](Type::const_iterator it)
		{
			using ParamType = decltype(it->getType());
			switch (it->getType())
			{
			case ParamType::ENTITY_ABSTRACT:
				putAbstractEntity(out, it->getAbstractEntity(), CON);
				break;
			case ParamType::ENTITY_CONCRETE:
				putConcreteEntity(out, it->getConcreteEntity(), CON);
				break;
			case ParamType::NAMESPACE:
				putUsingNamespace(out, it->getNamespace());
				break;
			default:
				assert(false); throw domain_error("");
			}
		});
	}
	out += CLOSE_TEMPLATE;
}
void Deserializer::putFheadSelf(StringBuilder& out)
{
	ContainerIncluder<ConType::TEMPLATE_HEAD> includer(out);
	out += CHAR_SELF;
}
void Deserializer::putFheadStandard(StringBuilder& out, const TemplateHeadStandard& thead)
{
	static_cast<DeserializerTemplate*>(this)->putFheadStandard(out, thead);
}
void Deserializer::putFheadText(StringBuilder& out, const TemplateHeadText& thead)
{
	static_cast<DeserializerTemplate*>(this)->putFheadText(out, thead);
}

void Deserializer::putFuncBinaryDictionary(StringBuilder& out, const TemplateHeadBinary::Parameters& params, const Dictionary& dict)
{
	static const auto CON = ConType::DICTIONARY;
	putSeparatedValues<Dictionary, CON>(out, dict, [&](Dictionary::const_iterator it)
	{
		out += it->first;
		if (it->second.isList())
			putFuncBinaryList(out, params, it->second.getList());
		else
		{
			assert(it->second.isTuple());
			putFuncBinaryTuple(out, params, it->second.getTuple());
		}
	});
}
void Deserializer::putFuncBinaryList(StringBuilder& out, const TemplateHeadBinary::Parameters& params, const List& list)
{
	static const auto CON = ConType::LIST;
	putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it)
	{
		assert(it->isTuple());
		putFuncBinaryTuple(out, params, it->getTuple());
	});
}
void Deserializer::putFuncBinaryTuple(StringBuilder& out, const TemplateHeadBinary::Parameters& params, const Tuple& tuple)
{
	ContainerIncluder<ConType::TUPLE> includer(out);
	putFuncBodyBinary(out, params, tuple);
}

void Deserializer::putFuncStandardDictionary(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Dictionary& dict)
{
	static const auto CON = ConType::DICTIONARY;
	putSeparatedValues<Dictionary, CON>(out, dict, [&](Dictionary::const_iterator it)
	{
		out += it->first;
		if (it->second.isList())
			putFuncStandardList(out, params, it->second.getList());
		else
		{
			assert(it->second.isTuple());
			putFuncStandardTuple(out, params, it->second.getTuple());
		}
	});
}

void Deserializer::putFuncStandardList(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const List& list)
{
	static const auto CON = ConType::LIST;
	if (list.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it)
		{
			assert(it->isTuple());
			putFuncStandardTupleText(out, params, it->getTuple());
		});
	}
	else
	{
		putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it)
		{
			assert(it->isTuple());
			putFuncStandardTuple(out, params, it->getTuple());
		});
	}
}

void Deserializer::putFuncStandardTuple(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple)
{
	if (tuple.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putFuncStandardTupleText(out, params, tuple);
		return;
	}

	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");

	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		auto&& param = params[i++];
		if (!param.hasTemplateHead())
		{
			if (it->type == WebssType::NONE || it->type == WebssType::DEFAULT)
				assert(param.hasDefaultValue());
			else
				putConcreteValue(out, *it, CON);
		}
		else
		{
			auto&& params2 = param.getTemplateHeadStandard().getParameters();
			switch (it->getType())
			{
			case WebssType::DICTIONARY:
				putFuncStandardDictionary(out, params2, it->getDictionary());
				break;
			case WebssType::LIST:
				putFuncStandardList(out, params2, it->getList());
				break;
			case WebssType::TUPLE:
				putFuncStandardTuple(out, params2, it->getTuple());
				break;
			default:
				assert(false && "template body must be dictionary, list or tuple"); throw domain_error("");
			}
		}
	});
}

void Deserializer::putFuncStandardTupleText(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");

	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		auto&& param = params[i++];
		assert(!param.hasTemplateHead());
		switch (it->type)
		{
		case WebssType::NONE: case WebssType::DEFAULT:
			assert(param.hasDefaultValue());
			break;
		case WebssType::PRIMITIVE_STRING:
			putLineString(out, *it->tString, CON);
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

void Deserializer::putFuncTextDictionary(StringBuilder& out, const TemplateHeadText::Parameters& params, const Dictionary& dict)
{
	static const auto CON = ConType::DICTIONARY;
	putSeparatedValues<Dictionary, CON>(out, dict, [&](Dictionary::const_iterator it)
	{
		out += it->first;
		if (it->second.isList())
			putFuncTextList(out, params, it->second.getList());
		else
		{
			assert(it->second.isTuple());
			putFuncTextTuple(out, params, it->second.getTuple());
		}
	});
}

void Deserializer::putFuncTextList(StringBuilder& out, const TemplateHeadText::Parameters& params, const List& list)
{
	static const auto CON = ConType::LIST;
	putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it)
	{
		assert(it->isTuple());
		putFuncTextTuple(out, params, it->getTuple());
	});
}

void Deserializer::putFuncTextTuple(StringBuilder& out, const TemplateHeadText::Parameters& params, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");

	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		auto&& param = params[i++];
		switch (it->type)
		{
		case WebssType::NONE: case WebssType::DEFAULT:
			assert(param.hasDefaultValue());
			break;
		case WebssType::PRIMITIVE_STRING:
			putLineString(out, *it->tString, CON);
			break;
		default:
			assert(false); throw domain_error("");
		}
	});
}

void Deserializer::putBlock(StringBuilder& out, const Block& block, ConType con)
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

void Deserializer::putFuncScoped(StringBuilder& out, const TemplateScoped& templ, ConType con)
{
	NamespaceIncluder includer(currentNamespaces, templ.getParameters());
	if (templ.hasEntity())
	{
		putEntityName(out, templ.getEntity());
		putCharValue(out, templ.getValue(), con);
	}
	else
	{
		putFheadScoped(out, templ);
		putConcreteValue(out, templ.getValue(), con);
	}
}