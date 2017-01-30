//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "base.h"
#include "block.h"
#include "function.h"
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
	using FunctionHeadBinary = BasicFunctionHead<ParamBinary, Webss>;
	using FunctionHeadScoped = BasicFunctionHeadScoped<Webss>;
	using FunctionHeadStandard = BasicFunctionHead<ParamStandard, Webss>;
	using FunctionHeadText = BasicFunctionHead<ParamText, Webss>;
	using FunctionBinary = BasicFunction<FunctionHeadBinary, Webss>;
	using FunctionScoped = BasicFunctionScoped<Webss>;
	using FunctionStandard = BasicFunction<FunctionHeadStandard, Webss>;
	using FunctionText = BasicFunction<FunctionHeadText, Webss>;
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
		Webss(FunctionHeadBinary&& fheadBinary);
		Webss(FunctionHeadScoped&& fheadScoped);
		Webss(FunctionHeadStandard&& fheadStandard);
		Webss(FunctionHeadText&& fheadText);
		Webss(FunctionBinary&& funcBinary);
		Webss(FunctionScoped&& funcScoped);
		Webss(FunctionStandard&& funcStandard);
		Webss(FunctionText&& funcText);
		Webss(Namespace&& nspace);
		Webss(Enum&& tEnum);
		Webss(BlockHead&& blockHead);
		Webss(Block&& block);

		Webss(const std::string& s);
		Webss(const Document& document);
		Webss(const Dictionary& dict);
		Webss(const List& list);
		Webss(const Tuple& tuple);
		Webss(const FunctionHeadBinary& fheadBinary);
		Webss(const FunctionHeadScoped& fheadScoped);
		Webss(const FunctionHeadStandard& fheadStandard);
		Webss(const FunctionHeadText& fheadText);
		Webss(const FunctionBinary& funcBinary);
		Webss(const FunctionScoped& funcScoped);
		Webss(const FunctionStandard& funcStandard);
		Webss(const FunctionText& funcText);
		Webss(const Namespace& nspace);
		Webss(const Enum& tEnum);
		Webss(const BlockHead& blockHead);
		Webss(const Block& block);

		Webss(FunctionHeadSelf);

		Webss(FunctionHeadBinary&& head, Webss&& body);
		Webss(FunctionHeadStandard&& head, Webss&& body);
		Webss(FunctionHeadText&& head, Webss&& body);

		Webss(const BasicEntity<Webss>& ent) : t(WebssType::ENTITY), ent(ent) {}
		Webss(const Default& tDefault) : t(WebssType::DEFAULT), tDefault(tDefault) {}

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
		const FunctionHeadBinary& getFunctionHeadBinary() const;
		const FunctionHeadScoped& getFunctionHeadScoped() const;
		const FunctionHeadStandard& getFunctionHeadStandard() const;
		const FunctionHeadText& getFunctionHeadText() const;
		const FunctionBinary& getFunctionBinary() const;
		const FunctionScoped& getFunctionScoped() const;
		const FunctionStandard& getFunctionStandard() const;
		const FunctionText& getFunctionText() const;
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
		explicit operator const FunctionHeadBinary&() const { return getFunctionHeadBinary(); }
		explicit operator const FunctionHeadScoped&() const { return getFunctionHeadScoped(); }
		explicit operator const FunctionHeadStandard&() const { return getFunctionHeadStandard(); }
		explicit operator const FunctionHeadText&() const { return getFunctionHeadText(); }
		explicit operator const FunctionBinary&() const { return getFunctionBinary(); }
		explicit operator const FunctionScoped&() const { return getFunctionScoped(); }
		explicit operator const FunctionStandard&() const { return getFunctionStandard(); }
		explicit operator const FunctionText&() const { return getFunctionText(); }
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
		bool isFunctionHeadBinary() const;
		bool isFunctionHeadScoped() const;
		bool isFunctionHeadStandard() const;
		bool isFunctionHeadText() const;
		bool isNamespace() const;
		bool isEnum() const;
		bool isBlockHead() const;
		bool isBlock() const;

		bool isAbstract() const;
		bool isConcrete() const;

		WebssType t = WebssType::NONE;
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
			FunctionHeadBinary* fheadBinary;
			FunctionHeadScoped* fheadScoped;
			FunctionHeadStandard* fheadStandard;
			FunctionHeadText* fheadText;
			FunctionBinary* funcBinary;
			FunctionScoped* funcScoped;
			FunctionStandard* funcStandard;
			FunctionText* funcText;
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