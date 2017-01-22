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
			using EntityFunctionHead = BasicEntity<FunctionHead>;
			using EntityNumber = BasicEntity<WebssBinarySize>;
			using Default = std::shared_ptr<Webss>;
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, KEYWORD, NUMBER, FUNCTION_HEAD, FUNCTION_HEAD_POINTER, ENTITY_NUMBER, ENTITY_FUNCTION_HEAD };
			enum class Flag { NONE, DEFAULT, SELF };

			Type t;
			union
			{
				Keyword keyword;
				WebssBinarySize number;
				EntityFunctionHead entFunctionHead;
				EntityNumber entNumber;
				FunctionHead* fhead;
			};

			Flag flag = Flag::NONE;
			Default defaultValue;


			BasicSizeHead() : t(Type::NONE) {}
			BasicSizeHead(Keyword keyword)
			{
				switch (keyword)
				{
				case Keyword::BOOL: case Keyword::INT1: case Keyword::INT2: case Keyword::INT4: case Keyword::INT8: case Keyword::DEC4: case Keyword::DEC8:
					t = Type::KEYWORD;
					this->keyword = keyword;
					break;
				case Keyword::STRING:
					t = Type::EMPTY;
					break;
				default:
					throw runtime_error("invalid binary type: " + keyword.toString());
				}
			}
			BasicSizeHead(const EntityFunctionHead& newEnt) : t(Type::ENTITY_FUNCTION_HEAD), entFunctionHead(newEnt) {}
			BasicSizeHead(const EntityNumber& newEnt) : entNumber(newEnt)
			{
				auto num = newEnt.getContent();
				if (num > 0)
					t = Type::ENTITY_NUMBER;
				else if (num == 0)
					t = Type::EMPTY_ENTITY_NUMBER;
				else
					throw std::runtime_error(ERROR_BINARY_SIZE_HEAD);
			}
			BasicSizeHead(WebssBinarySize number) : number(number)
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

			BasicSizeHead(Type t) : t(t)
			{
				switch (t)
				{
				case Type::NONE: case Type::EMPTY:
					break;
				default:
					throw std::domain_error("");
				}
			}

			~BasicSizeHead() { destroyUnion(); }

			

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

			bool isEmpty() const { return t == Type::EMPTY || t == Type::EMPTY_ENTITY_NUMBER; }
			bool isKeyword() const { return t == Type::KEYWORD; }
			bool isBool() const { return isKeyword() && keyword == Keyword::BOOL; }
			bool isFunctionHead() const { return t == Type::ENTITY_FUNCTION_HEAD || t == Type::FUNCTION_HEAD || t == Type::FUNCTION_HEAD_POINTER; }

			void setDefaultValue(Webss&& value) { new (&defaultValue) Default(new Webss(std::move(value))); }

			const std::string& getEntName() const
			{
				switch (t)
				{
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
					return entNumber.getName();
				case Type::ENTITY_FUNCTION_HEAD:
					return entFunctionHead.getName();
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
				case Type::ENTITY_FUNCTION_HEAD:
					return entFunctionHead.getContent();
				default:
					throw std::domain_error("binary size head does not contain a function head");
				}
			}

			WebssBinarySize size() const
			{
				switch (t)
				{
				case Type::KEYWORD:
					return keyword.getSize();
				case Type::NUMBER:
					return number;
				case Type::ENTITY_NUMBER:
					return entNumber.getContent();
				default:
					throw std::domain_error("");
				}
			}

			bool hasEntity() const { return t == Type::ENTITY_FUNCTION_HEAD || t == Type::ENTITY_NUMBER || t == Type::EMPTY_ENTITY_NUMBER; }
		private:
			static constexpr char* ERROR_BINARY_SIZE_HEAD = "size of binary head must be a positive integer, binary function head or equivalent entity";

			void destroyUnion()
			{
				switch (t)
				{
				case Type::FUNCTION_HEAD:
					delete fhead;
					break;
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
					entNumber.~BasicEntity();
					break;
				case Type::ENTITY_FUNCTION_HEAD:
					entFunctionHead.~BasicEntity();
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
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
					new (&entNumber) EntityNumber(std::move(o.entNumber));
					break;
				case Type::ENTITY_FUNCTION_HEAD:
					new (&entFunctionHead) EntityFunctionHead(std::move(o.entFunctionHead));
					break;
				default:
					throw std::domain_error("");
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
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
					new (&entNumber) EntityNumber(o.entNumber);
					break;
				case Type::ENTITY_FUNCTION_HEAD:
					new (&entFunctionHead) EntityFunctionHead(o.entFunctionHead);
					break;
				default:
					throw std::domain_error("");
				}

				if ((flag = o.flag) != Flag::NONE)
					defaultValue = o.defaultValue;
			}
		};

		class BasicSizeList
		{
		public:
			using Entity = BasicEntity<WebssBinarySize>;
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, ONE, NUMBER, ENTITY_NUMBER };

			Type t;
			union
			{
				WebssBinarySize number;
				Entity ent;
			};

			BasicSizeList() : t(Type::NONE) {}
			BasicSizeList(const Entity& newEnt) : ent(newEnt)
			{
				auto num = newEnt.getContent();
				if (num > 0)
					t = Type::ENTITY_NUMBER;
				else if (num == 0)
					t = Type::EMPTY_ENTITY_NUMBER;
				else
					throw std::runtime_error(ERROR_BINARY_SIZE_LIST);
			}
			BasicSizeList(WebssBinarySize number) : number(number)
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
					throw std::domain_error("");
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

			bool isEmpty() const { return t == Type::EMPTY || t == Type::EMPTY_ENTITY_NUMBER; }
			bool isOne() const { return t == Type::ONE; }

			WebssBinarySize size() const
			{
				switch (t)
				{
				case Type::NUMBER:
					return number;
				case Type::ENTITY_NUMBER:
					return ent.getContent();
				default:
					throw std::domain_error("");
				}
			}

			bool hasEntity() const { return t == Type::ENTITY_NUMBER || t == Type::EMPTY_ENTITY_NUMBER; }

			const std::string& getEntName() const
			{
				assert(hasEntity());
				return ent.getName();
			}
		private:
			static constexpr char* ERROR_BINARY_SIZE_LIST = "size of binary list must be a positive integer or equivalent entity";

			void destroyUnion()
			{
				if (hasEntity())
					ent.~BasicEntity();
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
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
					new (&ent) Entity(std::move(o.ent));
					break;
				default:
					throw std::domain_error("");
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
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
					new (&ent) Entity(o.ent);
					break;
				default:
					throw std::domain_error("");
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