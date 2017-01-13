//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "function.h"
#include "block.h"
#include "dictionary.h"
#include "namespace.h"
#include "typeWebss.h"
#include "paramStandard.h"
#include "document.h"

namespace webss
{
	class Webss;

	using Document = BasicDocument<Webss>;
	using Dictionary = BasicDictionary<Webss>;
	using List = BasicList<Webss>;
	using Tuple = BasicTuple<Webss>;
	using ParamStandard = BasicParamStandard<Webss>;
	using ParamBinary = BasicParamBinary<Webss>;
	using FunctionHeadStandard = BasicFunctionHead<ParamStandard>;
	using FunctionHeadBinary = BasicFunctionHead<ParamBinary>;
	using FunctionStandard = BasicFunctionDual<FunctionHeadStandard, Webss>;
	using FunctionBinary = BasicFunctionSingle<FunctionHeadBinary, Webss>;
//	using FunctionScoped = BasicFunctionScoped<Webss>;
	using BlockHead = FunctionHeadStandard;
	using Block = BasicBlock<Webss>;
	using Variable = BasicVariable<Webss>;
	using Namespace = BasicNamespace<Webss>;
	using Enum = Namespace;
	using Default = std::shared_ptr<Webss>;

	class Webss
	{
	public:
		Webss();
		Webss(WebssType t);
		Webss(WebssType t, bool containerText);
		Webss(WebssType::Enum t);
		Webss(WebssType::Enum t, bool containerText);

		Webss(Webss&& o);
		Webss(const Webss& o);

		explicit Webss(bool b);
		Webss(Keyword keyword);
		Webss(int i);
		Webss(type_int i);
		Webss(size_t i);
		Webss(double d);

		Webss(const char* s);

		Webss(std::string&& s);
		Webss(Document&& document);
		Webss(Dictionary&& dictionary);
		Webss(List&& list);
		Webss(Tuple&& tuple);
		Webss(FunctionHeadStandard&& fheadStandard);
		Webss(FunctionHeadBinary&& fheadBinary);
		Webss(FunctionStandard&& funcStandard);
		Webss(FunctionBinary&& funcBinary);
//		Webss(FunctionScoped&& funcScoped);
		Webss(BlockId&& blockId);
		Webss(Block&& block);
		Webss(Namespace&& nspace);
		Webss(Enum&& tEnum, bool dummy);
		Webss(BlockHead&& blockHead, bool dummy);

		Webss(const std::string& s);
		Webss(const Document& document);
		Webss(const Dictionary& dictionary);
		Webss(const List& list);
		Webss(const Tuple& tuple);
		Webss(const FunctionHeadStandard& fheadStandard);
		Webss(const FunctionHeadBinary& fheadBinary);
		Webss(const FunctionStandard& funcStandard);
		Webss(const FunctionBinary& funcBinary);
//		Webss(const FunctionScoped& funcScoped);
		Webss(const BlockId& blockId);
		Webss(const Block& block);
		Webss(const Namespace& nspace);
		Webss(const Enum& tEnum, bool dummy);
		Webss(const BlockHead& blockHead, bool dummy);

		Webss(FunctionHeadStandard&& head, Webss&& body);

		Webss(const BasicVariable<Webss>& var) : t(WebssType::VARIABLE), var(var) {}
		Webss(const Default& tDefault) : t(WebssType::DEFAULT), tDefault(tDefault) {}

		~Webss();

		bool hasVariable() const;

		Webss& operator=(Webss&& o);
		Webss& operator=(const Webss& o);

		void add(Webss&& value);
		void add(const Webss& value);
		void add(std::string&& key, Webss&& value);
		void add(const std::string& key, const Webss& value);
		void addSafe(std::string&& key, Webss&& value);
		void addSafe(const std::string& key, const Webss& value);

		const Webss& operator[](int index) const;
		const Webss& operator[](const std::string& key) const;
		const Webss& at(int index) const;
		const Webss& at(const std::string& key) const;

		WebssType getType() const;

		bool getBool() const;
		type_int getInt() const;
		double getDouble() const;
		const std::string& getString() const;
		const Document& getDocument() const;
		const Dictionary& getDictionary() const;
		const List& getList() const;
		const Tuple& getTuple() const;
		const FunctionHeadStandard& getFunctionHeadStandard() const;
		const FunctionHeadBinary& getFunctionHeadBinary() const;
		const FunctionStandard& getFunctionStandard() const;
		const FunctionBinary& getFunctionBinary() const;
//		const FunctionScoped& getFunctionScoped() const;
		const BlockId& getBlockId() const;
		const Block& getBlock() const;
		const Namespace& getNamespace() const;
		const Enum& getEnum() const;
		const BlockHead& getBlockHead() const;

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
		bool isFunctionHeadStandard() const;
		bool isFunctionHeadBinary() const;
		bool isBlockId() const;
		bool isBlock() const;
		bool isNamespace() const;
		bool isEnum() const;
		bool isBlockHead() const;

		bool isConcrete() const;

		bool isPrimitive() const;

		WebssType t;
		union
		{
			bool tBool;
			type_int tInt;
			double tDouble;
			std::string* tString;
			Document* document;
			Dictionary* dictionary;
			List* list;
			Tuple* tuple;
			FunctionHeadStandard* fheadStandard;
			FunctionHeadBinary* fheadBinary;
			FunctionStandard* funcStandard;
			FunctionBinary* funcBinary;
//			FunctionScoped* funcScoped;
			BlockId* blockId;
			Block* block;
			Namespace* nspace;
			BasicVariable<Webss> var;
			Default tDefault;
		};

		void destroyUnion();
		void copyUnion(Webss&& o);
		void copyUnion(const Webss& o);
	};
}