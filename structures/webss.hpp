//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <string>

#include "base.hpp"
#include "enum.hpp"
#include "entity.hpp"
#include "keywords.hpp"
#include "namespace.hpp"
#include "types.hpp"
#include "typeWebss.hpp"

namespace webss
{
	using Default = std::shared_ptr<Webss>;

	class Webss
	{
	public:
		Webss();
		~Webss();

		Webss(Webss&& o);
		Webss(const Webss& o);

		Webss& operator=(Webss o);

		Webss(const Entity& ent);
		Webss(const Default& tDefault);
		Webss(const Namespace& nspace);
		Webss(const Enum& tEnum);

		explicit Webss(bool b);
		Webss(Keyword keyword);
		Webss(int i);
		Webss(WebssInt i);
		Webss(size_t i);
		Webss(double d);

		Webss(const char* s);
		Webss(std::string s);

		Webss(Document document);
		Webss(Dictionary dict);
		Webss(List list);
		Webss(List list, bool);
		Webss(Tuple tuple);
		Webss(Tuple tuple, bool);
		Webss(TemplateHeadBinary theadBinary);
		Webss(TemplateHeadScoped theadScoped);
		Webss(TemplateHeadStandard theadStandard);
		Webss(TemplateHeadStandard theadStandard, bool);
		Webss(TemplateBinary templBinary);
		Webss(TemplateScoped templScoped);
		Webss(TemplateStandard templStandard);
		Webss(TemplateStandard templStandard, bool);
		Webss(BlockHead bhead);
		Webss(Block block);

		Webss(TemplateHeadSelf);

		Webss(TemplateHeadBinary&& head, Webss&& body);
		Webss(TemplateHeadStandard&& head, Webss&& body);
		Webss(TemplateHeadStandard&& head, Webss&& body, bool);

		const Webss& operator[](int index) const;
		const Webss& operator[](const std::string& key) const;
		const Webss& at(int index) const;
		const Webss& at(const std::string& key) const;

		const Webss& getWebssLast() const;

		WebssType getType() const;
		bool getBool() const;
		WebssInt getInt() const;
		double getDouble() const;
		const std::string& getString() const;
		const Document& getDocument() const;
		const Dictionary& getDictionary() const;
		const List& getList() const;
		const Tuple& getTuple() const;
		const TemplateHeadBinary& getTemplateHeadBinary() const;
		const TemplateHeadScoped& getTemplateHeadScoped() const;
		const TemplateHeadStandard& getTemplateHeadStandard() const;
		const TemplateBinary& getTemplateBinary() const;
		const TemplateScoped& getTemplateScoped() const;
		const TemplateStandard& getTemplateStandard() const;
		const Namespace& getNamespace() const;
		const Enum& getEnum() const;
		const BlockHead& getBlockHead() const;
		const Block& getBlock() const;

		WebssType getTypeRaw() const;

		const Entity& getEntityRaw() const;
		const Default& getDefaultRaw() const;
		const Namespace& getNamespaceRaw() const;
		const Enum& getEnumRaw() const;

		bool getBoolRaw() const;
		WebssInt getIntRaw() const;
		double getDoubleRaw() const;
		const std::string& getStringRaw() const;
		const Document& getDocumentRaw() const;
		const Dictionary& getDictionaryRaw() const;
		const List& getListRaw() const;
		const Tuple& getTupleRaw() const;
		const TemplateHeadBinary& getTemplateHeadBinaryRaw() const;
		const TemplateHeadScoped& getTemplateHeadScopedRaw() const;
		const TemplateHeadStandard& getTemplateHeadStandardRaw() const;
		const TemplateBinary& getTemplateBinaryRaw() const;
		const TemplateScoped& getTemplateScopedRaw() const;
		const TemplateStandard& getTemplateStandardRaw() const;
		const BlockHead& getBlockHeadRaw() const;
		const Block& getBlockRaw() const;

		Entity& getEntityRaw();
		Default& getDefaultRaw();
		Namespace& getNamespaceRaw();
		Enum& getEnumRaw();

		std::string& getStringRaw();
		Document& getDocumentRaw();
		Dictionary& getDictionaryRaw();
		List& getListRaw();
		Tuple& getTupleRaw();
		TemplateHeadBinary& getTemplateHeadBinaryRaw();
		TemplateHeadScoped& getTemplateHeadScopedRaw();
		TemplateHeadStandard& getTemplateHeadStandardRaw();
		TemplateBinary& getTemplateBinaryRaw();
		TemplateScoped& getTemplateScopedRaw();
		TemplateStandard& getTemplateStandardRaw();
		BlockHead& getBlockHeadRaw();
		Block& getBlockRaw();

		explicit operator bool() const { return getBool(); }
		explicit operator WebssInt() const { return getInt(); }
		explicit operator double() const { return getDouble(); }
		explicit operator const std::string&() const { return getString(); }
		explicit operator const Document&() const { return getDocument(); }
		explicit operator const Dictionary&() const { return getDictionary(); }
		explicit operator const List&() const { return getList(); }
		explicit operator const Tuple&() const { return getTuple(); }
		explicit operator const TemplateHeadBinary&() const { return getTemplateHeadBinary(); }
		explicit operator const TemplateHeadScoped&() const { return getTemplateHeadScoped(); }
		explicit operator const TemplateHeadStandard&() const { return getTemplateHeadStandard(); }
		explicit operator const TemplateBinary&() const { return getTemplateBinary(); }
		explicit operator const TemplateScoped&() const { return getTemplateScoped(); }
		explicit operator const TemplateStandard&() const { return getTemplateStandard(); }
		explicit operator const Namespace&() const { return getNamespace(); }
		explicit operator const Enum&() const { return getEnum(); }
		explicit operator const BlockHead&() const { return getBlockHead(); }
		explicit operator const Block&() const { return getBlock(); }

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
			BlockHead* bhead;
			Block* block;
			Namespace nspace;
			Enum tEnum;
			Entity ent;
			Default tDefault;
		};

		void destroyUnion();
		void copyUnion(Webss&& o);
		void copyUnion(const Webss& o);
	};
}