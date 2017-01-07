//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <assert.h>

#include "keywords.h"
#include "types.h"
#include "functionHead.h"
#include <memory>

namespace webss
{
	template <class Webss>
	class BasicParamBinary
	{
	public:
		template <class Webss>
		class BasicSizeHead
		{
		public:
			using FunctionHead = BasicFunctionHead<BasicParamBinary>;
			using VariableFunctionHead = BasicVariable<FunctionHead>;
			using VariableNumber = BasicVariable<type_binary_size>;
			using Default = std::shared_ptr<Webss>;
			enum class Type { NONE, EMPTY, EMPTY_VARIABLE_NUMBER, KEYWORD, NUMBER, FUNCTION_HEAD, FUNCTION_HEAD_POINTER, VARIABLE_NUMBER, VARIABLE_FUNCTION_HEAD };
			enum class Flag { NONE, DEFAULT, SELF };

			Type t;
			union
			{
				Keyword keyword;
				type_binary_size number;
				VariableFunctionHead varFunctionHead;
				VariableNumber varNumber;
				FunctionHead* fhead;
			};

			Flag flag = Flag::NONE;
			Default defaultValue;


			BasicSizeHead() : t(Type::NONE) {}
			BasicSizeHead(Keyword keyword) : t(Type::KEYWORD), keyword(keyword) {}
			BasicSizeHead(const VariableFunctionHead& newVar) : t(Type::VARIABLE_FUNCTION_HEAD), varFunctionHead(newVar) {}
			BasicSizeHead(const VariableNumber& newVar) : varNumber(newVar)
			{
				auto num = newVar.getContent();
				if (num > 0)
					t = Type::VARIABLE_NUMBER;
				else if (num == 0)
					t = Type::EMPTY_VARIABLE_NUMBER;
				else
					throw std::runtime_error(ERROR_BINARY_SIZE_HEAD);
			}
			BasicSizeHead(type_binary_size number) : number(number)
			{
				if (number > 0)
					t = Type::NUMBER;
				else if (number == 0)
					t = Type::EMPTY;
				else
					throw std::runtime_error(ERROR_BINARY_SIZE_HEAD);
			}
			BasicSizeHead(FunctionHead&& o) : t(Type::FUNCTION_HEAD) { fhead = new FunctionHead(std::move(o)); }
			BasicSizeHead(const FunctionHead& o) : t(Type::FUNCTION_HEAD) { fhead = new FunctionHead(o); }
			BasicSizeHead(FunctionHead* o) : t(Type::FUNCTION_HEAD_POINTER) { fhead = o; }
			~BasicSizeHead() { destroyUnion(); }

			BasicSizeHead(Type t) : t(t)
			{
				switch (t)
				{
				case Type::NONE: case Type::EMPTY:
					break;
				default:
					throw std::domain_error(ERROR_UNDEFINED);
				}
			}

			BasicSizeHead(BasicSizeHead&& o) { copyUnion(std::move(o)); }
			BasicSizeHead(const BasicSizeHead& o) { copyUnion(o); }

			BasicSizeHead& operator=(BasicSizeHead&& o)
			{
				if (this != &o)
				{
					destroyUnion();
					copyUnion(std::move(o));
				}
				return *this;
			}
			BasicSizeHead& operator=(const BasicSizeHead& o)
			{
				if (this != &o)
				{
					destroyUnion();
					copyUnion(o);
				}
				return *this;
			}

			Flag getFlag() { return flag; }
			void setFlag(Flag f) { flag = f; }

			bool isEmpty() const { return t == Type::EMPTY || t == Type::EMPTY_VARIABLE_NUMBER; }
			bool isKeyword() const { return t == Type::KEYWORD; }
			bool isBool() const { return isKeyword() && keyword == Keyword::BOOL; }
			bool isFunctionHead() const { return t == Type::VARIABLE_FUNCTION_HEAD || t == Type::FUNCTION_HEAD || t == Type::FUNCTION_HEAD_POINTER; }

			void setDefaultValue(Webss&& value) { new (&defaultValue) Default(new Webss(std::move(value))); }

			const std::string& getVarName() const
			{
				switch (t)
				{
				case Type::EMPTY_VARIABLE_NUMBER: case Type::VARIABLE_NUMBER:
					return varNumber.getName();
				case Type::VARIABLE_FUNCTION_HEAD:
					return varFunctionHead.getName();
				default:
					throw std::logic_error("");
				}
			}

			const FunctionHead& getFunctionHead() const
			{
				switch (t)
				{
				case Type::FUNCTION_HEAD: case Type::FUNCTION_HEAD_POINTER:
					return *fhead;
				case Type::VARIABLE_FUNCTION_HEAD:
					return varFunctionHead.getContent();
				default:
					throw std::domain_error("binary size head does not contain a function head");
				}
			}

			type_binary_size size() const
			{
				switch (t)
				{
				case Type::KEYWORD:
					return keyword.getSize();
				case Type::NUMBER:
					return number;
				case Type::VARIABLE_NUMBER:
					return varNumber.getContent();
				default:
					throw std::domain_error(ERROR_UNDEFINED);
				}
			}

			bool hasVariable() const { return t == Type::VARIABLE_FUNCTION_HEAD || t == Type::VARIABLE_NUMBER || t == Type::EMPTY_VARIABLE_NUMBER; }
		private:
			static constexpr char* ERROR_BINARY_SIZE_HEAD = "size of binary head must be a positive integer, binary function head or equivalent variable";

			void destroyUnion()
			{
				switch (t)
				{
				case Type::FUNCTION_HEAD:
					delete fhead;
					break;
				case Type::EMPTY_VARIABLE_NUMBER: case Type::VARIABLE_NUMBER:
					varNumber.~BasicVariable();
					break;
				case Type::VARIABLE_FUNCTION_HEAD:
					varFunctionHead.~BasicVariable();
					break;
				default:
					break;
				}
			}

			void copyUnion(BasicSizeHead&& o)
			{
				switch (t = o.t)
				{
				case Type::NONE: case Type::EMPTY:
					break;
				case Type::KEYWORD:
					keyword = o.keyword;
					break;
				case Type::NUMBER:
					number = o.number;
					break;
				case Type::FUNCTION_HEAD: case Type::FUNCTION_HEAD_POINTER:
					fhead = o.fhead;
					break;
				case Type::EMPTY_VARIABLE_NUMBER: case Type::VARIABLE_NUMBER:
					new (&varNumber) VariableNumber(std::move(o.varNumber));
					break;
				case Type::VARIABLE_FUNCTION_HEAD:
					new (&varFunctionHead) VariableFunctionHead(std::move(o.varFunctionHead));
					break;
				default:
					throw std::domain_error(ERROR_UNDEFINED);
				}
				o.t = Type::NONE;

				if ((flag = o.flag) != Flag::NONE)
				{
					defaultValue = std::move(o.defaultValue);
					o.flag = Flag::NONE;
				}
			}
			void copyUnion(const BasicSizeHead& o)
			{
				switch (t = o.t)
				{
				case Type::NONE: case Type::EMPTY:
					break;
				case Type::KEYWORD:
					keyword = o.keyword;
					break;
				case Type::NUMBER:
					number = o.number;
					break;
				case Type::FUNCTION_HEAD:
					fhead = new FunctionHead(*o.fhead);
					break;
				case Type::FUNCTION_HEAD_POINTER:
					fhead = o.fhead;
					break;
				case Type::EMPTY_VARIABLE_NUMBER: case Type::VARIABLE_NUMBER:
					new (&varNumber) VariableNumber(o.varNumber);
					break;
				case Type::VARIABLE_FUNCTION_HEAD:
					new (&varFunctionHead) VariableFunctionHead(o.varFunctionHead);
					break;
				default:
					throw std::domain_error(ERROR_UNDEFINED);
				}

				if ((flag = o.flag) != Flag::NONE)
					defaultValue = o.defaultValue;
			}
		};

		class BasicSizeList
		{
		public:
			using Variable = BasicVariable<type_binary_size>;
			enum class Type { NONE, EMPTY, EMPTY_VARIABLE_NUMBER, ONE, NUMBER, VARIABLE_NUMBER };

			Type t;
			union
			{
				type_binary_size number;
				Variable var;
			};

			BasicSizeList() : t(Type::NONE) {}
			BasicSizeList(const Variable& newVar) : var(newVar)
			{
				auto num = newVar.getContent();
				if (num > 0)
					t = Type::VARIABLE_NUMBER;
				else if (num == 0)
					t = Type::EMPTY_VARIABLE_NUMBER;
				else
					throw std::runtime_error(ERROR_BINARY_SIZE_LIST);
			}
			BasicSizeList(type_binary_size number) : number(number)
			{
				if (number > 0)
					t = Type::NUMBER;
				else if (number == 0)
					t = Type::EMPTY;
				else
					throw std::runtime_error(ERROR_BINARY_SIZE_LIST);
			}

			~BasicSizeList() { destroyUnion(); }

			BasicSizeList(Type t) : t(t)
			{
				switch (t)
				{
				case Type::NONE: case Type::EMPTY: case Type::ONE:
					break;
				default:
					throw std::domain_error(ERROR_UNDEFINED);
				}
			}

			BasicSizeList(BasicSizeList&& o) { copyUnion(std::move(o)); }
			BasicSizeList(const BasicSizeList& o) { copyUnion(o); }

			BasicSizeList& operator=(BasicSizeList&& o)
			{
				if (this != &o)
				{
					destroyUnion();
					copyUnion(std::move(o));
				}
				return *this;
			}
			BasicSizeList& operator=(const BasicSizeList& o)
			{
				if (this != &o)
				{
					destroyUnion();
					copyUnion(o);
				}
				return *this;
			}

			bool isEmpty() const { return t == Type::EMPTY || t == Type::EMPTY_VARIABLE_NUMBER; }
			bool isOne() const { return t == Type::ONE; }

			type_binary_size size() const
			{
				switch (t)
				{
				case Type::NUMBER:
					return number;
				case Type::VARIABLE_NUMBER:
					return var.getContent();
				default:
					throw std::domain_error(ERROR_UNDEFINED);
				}
			}

			bool hasVariable() const { return t == Type::VARIABLE_NUMBER || t == Type::EMPTY_VARIABLE_NUMBER; }

			const std::string& getVarName() const
			{
				assert(hasVariable());
				return var.getName();
			}
		private:
			static constexpr char* ERROR_BINARY_SIZE_LIST = "size of binary list must be a positive integer or equivalent variable";

			void destroyUnion()
			{
				if (hasVariable())
					var.~BasicVariable();
			}

			void copyUnion(BasicSizeList&& o)
			{
				switch (t = o.t)
				{
				case Type::NONE: case Type::EMPTY: case Type::ONE:
					break;
				case Type::NUMBER:
					number = o.number;
					break;
				case Type::EMPTY_VARIABLE_NUMBER: case Type::VARIABLE_NUMBER:
					new (&var) Variable(std::move(o.var));
					break;
				default:
					throw std::domain_error(ERROR_UNDEFINED);
				}
				o.t = Type::NONE;
			}
			void copyUnion(const BasicSizeList& o)
			{
				switch (t = o.t)
				{
				case Type::NONE: case Type::EMPTY: case Type::ONE:
					break;
				case Type::NUMBER:
					number = o.number;
					break;
				case Type::EMPTY_VARIABLE_NUMBER: case Type::VARIABLE_NUMBER:
					new (&var) Variable(o.var);
					break;
				default:
					throw std::domain_error(ERROR_UNDEFINED);
				}
			}
		};

		using SizeHead = BasicSizeHead<Webss>;
		using SizeList = BasicSizeList;

		BasicParamBinary() {}
		BasicParamBinary(const SizeHead& sizeHead, const SizeList& sizeList) : sizeHead(sizeHead), sizeList(sizeList) {}
		BasicParamBinary(SizeHead&& sizeHead, SizeList&& sizeList) : sizeHead(std::move(sizeHead)), sizeList(std::move(sizeList)) {}
		~BasicParamBinary() {}

		BasicParamBinary(const BasicParamBinary& o) : sizeHead(o.sizeHead), sizeList(o.sizeList) {}
		BasicParamBinary(BasicParamBinary&& o) : sizeHead(std::move(o.sizeHead)), sizeList(std::move(o.sizeList)) {}

		BasicParamBinary& operator=(const BasicParamBinary& o)
		{
			if (this != &o)
			{
				sizeHead = o.sizeHead;
				sizeList = o.sizeList;
			}
			return *this;
		}
		BasicParamBinary& operator=(BasicParamBinary&& o)
		{
			if (this != &o)
			{
				sizeHead = std::move(o.sizeHead);
				sizeList = std::move(o.sizeList);
			}
			return *this;
		}

		SizeHead sizeHead;
		SizeList sizeList;
	};
}