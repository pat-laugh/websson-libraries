//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "block.h"
#include "function.h"
#include "namespace.h"
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
	using FunctionHeadBinary = BasicFunctionHead<ParamBinary>;
	using FunctionHeadScoped = BasicFunctionHead<ParamScoped>;
	using FunctionHeadStandard = BasicFunctionHead<ParamStandard>;
	using FunctionBinary = BasicFunction<FunctionHeadBinary, Webss>;
	using FunctionScoped = BasicFunctionScoped<Webss>;
	using FunctionStandard = BasicFunction<FunctionHeadStandard, Webss>;
	using Entity = BasicEntity<Webss>;
	using Namespace = BasicNamespace<Webss>;
	using Enum = Namespace;
	using BlockHead = BasicBlockHead<Webss>;
	using Block = BasicBlock<Webss>;
	using Default = std::shared_ptr<Webss>;

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
		Webss(FunctionBinary&& funcBinary);
		Webss(FunctionScoped&& funcScoped);
		Webss(FunctionStandard&& funcStandard);
		Webss(Namespace&& nspace);
		Webss(Enum&& tEnum, bool dummy);
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
		Webss(const FunctionBinary& funcBinary);
		Webss(const FunctionScoped& funcScoped);
		Webss(const FunctionStandard& funcStandard);
		Webss(const Namespace& nspace);
		Webss(const Enum& tEnum, bool dummy);
		Webss(const BlockHead& blockHead);
		Webss(const Block& block);

		Webss(FunctionHeadBinary&& head, Webss&& body);
		Webss(FunctionHeadStandard&& head, Webss&& body);

		Webss(const BasicEntity<Webss>& ent) : t(WebssType::ENTITY), ent(ent) {}
		Webss(const Default& tDefault) : t(WebssType::DEFAULT), tDefault(tDefault) {}

		~Webss();

		bool hasEntity() const;

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
		const FunctionBinary& getFunctionBinary() const;
		const FunctionScoped& getFunctionScoped() const;
		const FunctionStandard& getFunctionStandard() const;
		const Namespace& getNamespace() const;
		const Enum& getEnum() const;
		const BlockHead& getBlockHead() const;
		const Block& getBlock() const;

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
		bool isFunctionHead() const;
		bool isFunctionHeadBinary() const;
		bool isFunctionHeadScoped() const;
		bool isFunctionHeadStandard() const;
		bool isNamespace() const;
		bool isEnum() const;
		bool isBlockHead() const;
		bool isBlock() const;

		bool isConcrete() const;

		bool isPrimitive() const;

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
			FunctionBinary* funcBinary;
			FunctionScoped* funcScoped;
			FunctionStandard* funcStandard;
			Namespace* nspace;
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