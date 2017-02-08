//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "base.h"
#include "block.h"
#include "template.h"
#include "namespace.h"
#include "enum.h"
#include "paramStandard.h"
#include "document.h"

namespace webss
{
	class Webss;

	using Document = BasicDocument<Webss>;
	using Dictionary = BasicDictionary<Webss>;
	using List = BasicList<Webss>;
	using Tuple = BasicTuple<Webss>;
	using ParamBinary = BasicParamBinary<Webss>;
	using ParamScoped = BasicParamScoped<Webss>;
	using ParamStandard = BasicParamStandard<Webss>;
	using TemplateHeadBinary = BasicTemplateHead<ParamBinary, Webss>;
	using TemplateHeadScoped = BasicTemplateHeadScoped<Webss>;
	using TemplateHeadStandard = BasicTemplateHead<ParamStandard, Webss>;
	using TemplateBinary = BasicTemplate<TemplateHeadBinary, Webss>;
	using TemplateScoped = BasicTemplateScoped<Webss>;
	using TemplateStandard = BasicTemplate<TemplateHeadStandard, Webss>;
	using Entity = BasicEntity<Webss>;
	using Namespace = BasicNamespace<Webss>;
	using Enum = BasicEnum<Webss>;
	using BlockHead = BasicBlockHead<Webss>;
	using Block = BasicBlock<Webss>;
	using Default = std::shared_ptr<Webss>;

	using ParamDocument = BasicParamDocument<Webss>;
	using DocumentHead = BasicDocumentHead<Webss>;
	using ScopedDocument = BasicScopedDocument<Webss>;
	using ImportedDocument = BasicImportedDocument<Webss>;

	class Webss
	{
	public:
		Webss();

		Webss(Webss&& o);
		Webss(const Webss& o);

		explicit Webss(bool b);
		Webss(Keyword keyword);
		Webss(int i);
		Webss(WebssInt i);
		Webss(size_t i);
		Webss(double d);

		Webss(const char* s);

		Webss(std::string&& s);
		Webss(Document&& document);
		Webss(Dictionary&& dict);
		Webss(List&& list);
		Webss(Tuple&& tuple);
		Webss(List&& list, bool isText);
		Webss(Tuple&& tuple, bool isText);
		Webss(TemplateHeadBinary&& theadBinary);
		Webss(TemplateHeadScoped&& theadScoped);
		Webss(TemplateHeadStandard&& theadStandard);
		Webss(TemplateHeadStandard&& theadStandard, bool isText);
		Webss(TemplateBinary&& templBinary);
		Webss(TemplateScoped&& templScoped);
		Webss(TemplateStandard&& templStandard);
		Webss(TemplateStandard&& templStandard, bool isText);
		Webss(Namespace&& nspace);
		Webss(Enum&& tEnum);
		Webss(BlockHead&& blockHead);
		Webss(Block&& block);

		Webss(const std::string& s);
		Webss(const Document& document);
		Webss(const Dictionary& dict);
		Webss(const List& list);
		Webss(const Tuple& tuple);
		Webss(const List& list, bool isText);
		Webss(const Tuple& tuple, bool isText);
		Webss(const TemplateHeadBinary& theadBinary);
		Webss(const TemplateHeadScoped& theadScoped);
		Webss(const TemplateHeadStandard& theadStandard);
		Webss(const TemplateHeadStandard& theadStandard, bool isText);
		Webss(const TemplateBinary& templBinary);
		Webss(const TemplateScoped& templScoped);
		Webss(const TemplateStandard& templStandard);
		Webss(const TemplateStandard& templStandard, bool isText);
		Webss(const Namespace& nspace);
		Webss(const Enum& tEnum);
		Webss(const BlockHead& blockHead);
		Webss(const Block& block);

		Webss(TemplateHeadSelf);

		Webss(TemplateHeadBinary&& head, Webss&& body);
		Webss(TemplateHeadStandard&& head, Webss&& body);
		Webss(TemplateHeadStandard&& head, Webss&& body, bool isText);

		Webss(const Entity& ent) : type(WebssType::ENTITY), ent(ent) {}
		Webss(const Default& tDefault) : type(WebssType::DEFAULT), tDefault(tDefault) {}

		~Webss();

		Webss& operator=(Webss&& o);
		Webss& operator=(const Webss& o);

		const Webss& operator[](int index) const;
		const Webss& operator[](const std::string& key) const;
		const Webss& at(int index) const;
		const Webss& at(const std::string& key) const;

		WebssType getType() const { return type; }
		WebssType getTypeSafe() const;

		const Webss& getWebssLast() const;

		bool getBoolSafe() const;
		WebssInt getIntSafe() const;
		double getDoubleSafe() const;
		const std::string& getStringSafe() const;
		const Document& getDocumentSafe() const;
		const Dictionary& getDictionarySafe() const;
		const List& getListSafe() const;
		const Tuple& getTupleSafe() const;
		const TemplateHeadBinary& getTemplateHeadBinarySafe() const;
		const TemplateHeadScoped& getTemplateHeadScopedSafe() const;
		const TemplateHeadStandard& getTemplateHeadStandardSafe() const;
		const TemplateHeadStandard& getTemplateHeadTextSafe() const;
		const TemplateBinary& getTemplateBinarySafe() const;
		const TemplateScoped& getTemplateScopedSafe() const;
		const TemplateStandard& getTemplateStandardSafe() const;
		const Namespace& getNamespaceSafe() const;
		const Enum& getEnumSafe() const;
		const BlockHead& getBlockHeadSafe() const;
		const Block& getBlockSafe() const;

		bool getBool() const { assert(getType() == WebssType::PRIMITIVE_BOOL); return tBool; }
		WebssInt getInt() const { assert(getType() == WebssType::PRIMITIVE_INT); return tBool; }
		double getDouble() const { assert(getType() == WebssType::PRIMITIVE_DOUBLE); return tBool; }

		const std::string& getString() const { assert(getType() == WebssType::PRIMITIVE_STRING); return *tString; }
		const Document& getDocument() const { assert(getType() == WebssType::DOCUMENT); return *document; }
		const Dictionary& getDictionary() const { assert(getType() == WebssType::DICTIONARY); return *dict; }
		const List& getList() const { assert(getType() == WebssType::LIST || getType() == WebssType::LIST_TEXT); return *list; }
		const Tuple& getTuple() const { assert(getType() == WebssType::TUPLE || getType() == WebssType::TUPLE_TEXT); return *tuple; }
		const TemplateHeadBinary& getTemplateHeadBinary() const { assert(getType() == WebssType::TEMPLATE_HEAD_BINARY); return *theadBinary; }
		const TemplateHeadScoped& getTemplateHeadScoped() const { assert(getType() == WebssType::TEMPLATE_HEAD_SCOPED); return *theadScoped; }
		const TemplateHeadStandard& getTemplateHeadStandard() const { assert(getType() == WebssType::TEMPLATE_HEAD_STANDARD); return *theadStandard; }
		const TemplateHeadStandard& getTemplateHeadText() const { assert(getType() == WebssType::TEMPLATE_HEAD_TEXT); return *theadStandard; }
		const TemplateBinary& getTemplateBinary() const { assert(getType() == WebssType::TEMPLATE_BINARY); return *templBinary; }
		const TemplateScoped& getTemplateScoped() const { assert(getType() == WebssType::TEMPLATE_SCOPED); return *templScoped; }
		const TemplateStandard& getTemplateStandard() const { assert(getType() == WebssType::TEMPLATE_STANDARD || getType() == WebssType::TEMPLATE_TEXT); return *templStandard; }
		const Namespace& getNamespace() const { assert(getType() == WebssType::NAMESPACE); return *nspace; }
		const Enum& getEnum() const { assert(getType() == WebssType::ENUM); return *tEnum; }
		const BlockHead& getBlockHead() const { assert(getType() == WebssType::BLOCK_HEAD); return *blockHead; }
		const Block& getBlock() const { assert(getType() == WebssType::BLOCK); return *block; }

		const Entity& getEntity() const { assert(getType() == WebssType::ENTITY); return ent; }
		const Default& getDefault() const { assert(getType() == WebssType::DEFAULT); return tDefault; }

		std::string& getString() { assert(getType() == WebssType::PRIMITIVE_STRING); return *tString; }
		Document& getDocument() { assert(getType() == WebssType::DOCUMENT); return *document; }
		Dictionary& getDictionary() { assert(getType() == WebssType::DICTIONARY); return *dict; }
		List& getList() { assert(getType() == WebssType::LIST || getType() == WebssType::LIST_TEXT); return *list; }
		Tuple& getTuple() { assert(getType() == WebssType::TUPLE || getType() == WebssType::TUPLE_TEXT); return *tuple; }
		TemplateHeadBinary& getTemplateHeadBinary() { assert(getType() == WebssType::TEMPLATE_HEAD_BINARY); return *theadBinary; }
		TemplateHeadScoped& getTemplateHeadScoped() { assert(getType() == WebssType::TEMPLATE_HEAD_SCOPED); return *theadScoped; }
		TemplateHeadStandard& getTemplateHeadStandard() { assert(getType() == WebssType::TEMPLATE_HEAD_STANDARD); return *theadStandard; }
		TemplateHeadStandard& getTemplateHeadText() { assert(getType() == WebssType::TEMPLATE_HEAD_TEXT); return *theadStandard; }
		TemplateBinary& getTemplateBinary() { assert(getType() == WebssType::TEMPLATE_BINARY); return *templBinary; }
		TemplateScoped& getTemplateScoped() { assert(getType() == WebssType::TEMPLATE_SCOPED); return *templScoped; }
		TemplateStandard& getTemplateStandard() { assert(getType() == WebssType::TEMPLATE_STANDARD || getType() == WebssType::TEMPLATE_TEXT); return *templStandard; }
		Namespace& getNamespace() { assert(getType() == WebssType::NAMESPACE); return *nspace; }
		Enum& getEnum() { assert(getType() == WebssType::ENUM); return *tEnum; }
		BlockHead& getBlockHead() { assert(getType() == WebssType::BLOCK_HEAD); return *blockHead; }
		Block& getBlock() { assert(getType() == WebssType::BLOCK); return *block; }

		explicit operator bool() const { return getBoolSafe(); }
		explicit operator WebssInt() const { return getIntSafe(); }
		explicit operator WebssBinarySize() const { return static_cast<WebssBinarySize>(getIntSafe()); }
		explicit operator double() const { return getDoubleSafe(); }
		explicit operator const std::string&() const { return getStringSafe(); }
		explicit operator const Document&() const { return getDocumentSafe(); }
		explicit operator const Dictionary&() const { return getDictionarySafe(); }
		explicit operator const List&() const { return getListSafe(); }
		explicit operator const Tuple&() const { return getTupleSafe(); }
		explicit operator const TemplateHeadBinary&() const { return getTemplateHeadBinarySafe(); }
		explicit operator const TemplateHeadScoped&() const { return getTemplateHeadScopedSafe(); }
		explicit operator const TemplateHeadStandard&() const { return getTemplateHeadStandardSafe(); }
		explicit operator const TemplateBinary&() const { return getTemplateBinarySafe(); }
		explicit operator const TemplateScoped&() const { return getTemplateScopedSafe(); }
		explicit operator const TemplateStandard&() const { return getTemplateStandardSafe(); }
		explicit operator const Namespace&() const { return getNamespaceSafe(); }
		explicit operator const Enum&() const { return getEnumSafe(); }
		explicit operator const BlockHead&() const { return getBlockHeadSafe(); }
		explicit operator const Block&() const { return getBlockSafe(); }

		template <class Element>
		Element getPrimitive() const { return static_cast<Element>(*this); }

		template <class Element>
		const Element& getElement() const { return static_cast<const Element&>(*this); }

		bool isNone() const;
		bool isNull() const;
		bool isBool() const;
		bool isInt() const;
		bool isDouble() const;
		bool isString() const;
		bool isDocument() const;
		bool isDictionary() const;
		bool isList() const;
		bool isTuple() const;
		bool isListText() const;
		bool isTupleText() const;
		bool isTemplateHeadBinary() const;
		bool isTemplateHeadScoped() const;
		bool isTemplateHeadStandard() const;
		bool isTemplateHeadText() const;
		bool isNamespace() const;
		bool isEnum() const;
		bool isBlockHead() const;
		bool isBlock() const;

		bool isAbstract() const;
		bool isConcrete() const;
	private:
		WebssType type = WebssType::NONE;
		union
		{
			bool tBool;
			WebssInt tInt;
			double tDouble;
			std::string* tString;
			Document* document;
			Dictionary* dict;
			List* list;
			Tuple* tuple;
			TemplateHeadBinary* theadBinary;
			TemplateHeadScoped* theadScoped;
			TemplateHeadStandard* theadStandard;
			TemplateBinary* templBinary;
			TemplateScoped* templScoped;
			TemplateStandard* templStandard;
			Namespace* nspace;
			Enum* tEnum;
			BlockHead* blockHead;
			Block* block;
			Entity ent;
			Default tDefault;
		};

		void destroyUnion();
		void copyUnion(Webss&& o);
		void copyUnion(const Webss& o);
	};
}