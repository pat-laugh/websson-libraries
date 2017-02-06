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
	using ParamText = BasicParamText<Webss>;
	using TemplateHeadBinary = BasicTemplateHead<ParamBinary, Webss>;
	using TemplateHeadScoped = BasicTemplateHeadScoped<Webss>;
	using TemplateHeadStandard = BasicTemplateHead<ParamStandard, Webss>;
	using TemplateHeadText = BasicTemplateHead<ParamText, Webss>;
	using TemplateBinary = BasicTemplate<TemplateHeadBinary, Webss>;
	using TemplateScoped = BasicTemplateScoped<Webss>;
	using TemplateStandard = BasicTemplate<TemplateHeadStandard, Webss>;
	using TemplateText = BasicTemplate<TemplateHeadText, Webss>;
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
		Webss(TemplateHeadBinary&& fheadBinary);
		Webss(TemplateHeadScoped&& fheadScoped);
		Webss(TemplateHeadStandard&& fheadStandard);
		Webss(TemplateHeadText&& fheadText);
		Webss(TemplateBinary&& templBinary);
		Webss(TemplateScoped&& templScoped);
		Webss(TemplateStandard&& templStandard);
		Webss(TemplateText&& templText);
		Webss(Namespace&& nspace);
		Webss(Enum&& tEnum);
		Webss(BlockHead&& blockHead);
		Webss(Block&& block);

		Webss(const std::string& s);
		Webss(const Document& document);
		Webss(const Dictionary& dict);
		Webss(const List& list);
		Webss(const Tuple& tuple);
		Webss(const TemplateHeadBinary& fheadBinary);
		Webss(const TemplateHeadScoped& fheadScoped);
		Webss(const TemplateHeadStandard& fheadStandard);
		Webss(const TemplateHeadText& fheadText);
		Webss(const TemplateBinary& templBinary);
		Webss(const TemplateScoped& templScoped);
		Webss(const TemplateStandard& templStandard);
		Webss(const TemplateText& templText);
		Webss(const Namespace& nspace);
		Webss(const Enum& tEnum);
		Webss(const BlockHead& blockHead);
		Webss(const Block& block);

		Webss(TemplateHeadSelf);

		Webss(TemplateHeadBinary&& head, Webss&& body);
		Webss(TemplateHeadStandard&& head, Webss&& body);
		Webss(TemplateHeadText&& head, Webss&& body);

		Webss(const BasicEntity<Webss>& ent) : type(WebssType::ENTITY), ent(ent) {}
		Webss(const Default& tDefault) : type(WebssType::DEFAULT), tDefault(tDefault) {}

		~Webss();

		Webss& operator=(Webss&& o);
		Webss& operator=(const Webss& o);

		const Webss& operator[](int index) const;
		const Webss& operator[](const std::string& key) const;
		const Webss& at(int index) const;
		const Webss& at(const std::string& key) const;

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
		const TemplateHeadText& getTemplateHeadText() const;
		const TemplateBinary& getTemplateBinary() const;
		const TemplateScoped& getTemplateScoped() const;
		const TemplateStandard& getTemplateStandard() const;
		const TemplateText& getTemplateText() const;
		const Namespace& getNamespace() const;
		const Enum& getEnum() const;
		const BlockHead& getBlockHead() const;
		const Block& getBlock() const;

		explicit operator bool() const { return getBool(); }
		explicit operator WebssInt() const { return getInt(); }
		explicit operator WebssBinarySize() const { return static_cast<WebssBinarySize>(getInt()); }
		explicit operator double() const { return getDouble(); }
		explicit operator const std::string&() const { return getString(); }
		explicit operator const Document&() const { return getDocument(); }
		explicit operator const Dictionary&() const { return getDictionary(); }
		explicit operator const List&() const { return getList(); }
		explicit operator const Tuple&() const { return getTuple(); }
		explicit operator const TemplateHeadBinary&() const { return getTemplateHeadBinary(); }
		explicit operator const TemplateHeadScoped&() const { return getTemplateHeadScoped(); }
		explicit operator const TemplateHeadStandard&() const { return getTemplateHeadStandard(); }
		explicit operator const TemplateHeadText&() const { return getTemplateHeadText(); }
		explicit operator const TemplateBinary&() const { return getTemplateBinary(); }
		explicit operator const TemplateScoped&() const { return getTemplateScoped(); }
		explicit operator const TemplateStandard&() const { return getTemplateStandard(); }
		explicit operator const TemplateText&() const { return getTemplateText(); }
		explicit operator const Namespace&() const { return getNamespace(); }
		explicit operator const Enum&() const { return getEnum(); }
		explicit operator const BlockHead&() const { return getBlockHead(); }
		explicit operator const Block&() const { return getBlock(); }

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
			TemplateHeadBinary* fheadBinary;
			TemplateHeadScoped* fheadScoped;
			TemplateHeadStandard* fheadStandard;
			TemplateHeadText* fheadText;
			TemplateBinary* templBinary;
			TemplateScoped* templScoped;
			TemplateStandard* templStandard;
			TemplateText* templText;
			Namespace* nspace;
			Enum* tEnum;
			BlockHead* blockHead;
			Block* block;
			BasicEntity<Webss> ent;
			Default tDefault;
		};

		void destroyUnion();
		void copyUnion(Webss&& o);
		void copyUnion(const Webss& o);
	};
}