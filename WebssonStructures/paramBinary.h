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
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, KEYWORD, NUMBER, FUNCTION_HEAD, FUNCTION_HEAD_POINTER, ENTITY_NUMBER, ENTITY_FUNCTION_HEAD };
			enum class Flag { NONE, DEFAULT, SELF };

			BasicSizeHead() {}
			BasicSizeHead(Keyword keyword) : t(Type::KEYWORD)
			{
				switch (keyword)
				{
				case Keyword::BOOL: case Keyword::INT1: case Keyword::INT2: case Keyword::INT4: case Keyword::INT8: case Keyword::DEC4: case Keyword::DEC8:
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
			BasicSizeHead(const EntityNumber& newEnt) : t(Type::ENTITY_NUMBER), entNumber(newEnt)
			{
				auto num = newEnt.getContent();
				if (num <= 0)
				{
					if (num == 0)
						t = Type::EMPTY_ENTITY_NUMBER;
					else
					{
						entNumber.~BasicEntity();
						throw std::runtime_error(ERROR_BINARY_SIZE_HEAD);
					}
				}
			}
			BasicSizeHead(WebssBinarySize number) : t(Type::NUMBER), number(number)
			{
				if (number <= 0)
				{
					if (number == 0)
						t = Type::EMPTY;
					else
						throw std::runtime_error(ERROR_BINARY_SIZE_HEAD);
				}
			}
			BasicSizeHead(FunctionHead&& o) : t(Type::FUNCTION_HEAD), fhead(new FunctionHead(std::move(o))) {}
			BasicSizeHead(const FunctionHead& o) : t(Type::FUNCTION_HEAD), fhead(new FunctionHead(o)) {}
			BasicSizeHead(FunctionHead* o) : t(Type::FUNCTION_HEAD_POINTER), fhead(o) {}

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
				destroyUnion();
				copyUnion(std::move(o));
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

			Flag getFlag() const { return flag; }
			void setFlag(Flag f) { flag = f; }
			bool isEmpty() const { return t == Type::EMPTY || t == Type::EMPTY_ENTITY_NUMBER; }
			bool isKeyword() const { return t == Type::KEYWORD; }
			bool isBool() const { return isKeyword() && keyword == Keyword::BOOL; }
			bool isFunctionHead() const { return t == Type::ENTITY_FUNCTION_HEAD || t == Type::FUNCTION_HEAD || t == Type::FUNCTION_HEAD_POINTER; }
			bool hasEntity() const { return t == Type::ENTITY_FUNCTION_HEAD || t == Type::ENTITY_NUMBER || t == Type::EMPTY_ENTITY_NUMBER; }

			Type getType() const { return t; }
			Keyword getKeyword() const { return keyword; }
			WebssBinarySize getNumber() const { return number; }
			const Webss& getDefaultValue() const { return *defaultValue; }
			const std::shared_ptr<Webss>& getDefaultPointer() const { return defaultValue; }
			void setDefaultValue(Webss&& value) { defaultValue = std::shared_ptr<Webss>(new Webss(std::move(value))); }

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
		private:
			static constexpr char* ERROR_BINARY_SIZE_HEAD = "size of binary head must be a positive integer, binary function head or equivalent entity";

			Type t = Type::NONE;
			union
			{
				Keyword keyword;
				WebssBinarySize number;
				EntityFunctionHead entFunctionHead;
				EntityNumber entNumber;
				FunctionHead* fhead;
			};

			Flag flag = Flag::NONE;
			std::shared_ptr<Webss> defaultValue;

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
				t = Type::NONE;
			}

			void copyUnion(BasicSizeHead&& o)
			{
				switch (o.t)
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
					o.entNumber.~BasicEntity();
					break;
				case Type::ENTITY_FUNCTION_HEAD:
					new (&entFunctionHead) EntityFunctionHead(std::move(o.entFunctionHead));
					o.entFunctionHead.~BasicEntity();
					break;
				default:
					throw std::domain_error("");
				}
				t = o.t;
				o.t = Type::NONE;

				if (o.flag != Flag::NONE)
				{
					defaultValue = std::move(o.defaultValue);
					flag = o.flag;
					o.flag = Flag::NONE;
				}
			}
			void copyUnion(const BasicSizeHead& o)
			{
				switch (o.t)
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
				t = o.t;

				if (o.flag != Flag::NONE)
				{
					defaultValue = o.defaultValue;
					flag = o.flag;
				}
			}
		};

		class BasicSizeList
		{
		public:
			using Entity = BasicEntity<WebssBinarySize>;
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, ONE, NUMBER, ENTITY_NUMBER };

			BasicSizeList() {}
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
			BasicSizeList(const Entity& newEnt) : t(Type::ENTITY_NUMBER), ent(newEnt)
			{
				auto num = newEnt.getContent();
				if (num <= 0)
				{
					if (num == 0)
						t = Type::EMPTY_ENTITY_NUMBER;
					else
					{
						ent.~BasicEntity();
						throw std::runtime_error(ERROR_BINARY_SIZE_LIST);
					}
				}
			}
			BasicSizeList(WebssBinarySize number) : t(Type::NUMBER), number(number)
			{
				if (number <= 0)
				{
					if (number == 0)
						t = Type::EMPTY;
					else
						throw std::runtime_error(ERROR_BINARY_SIZE_LIST);
				}
			}

			~BasicSizeList() { destroyUnion(); }

			BasicSizeList(BasicSizeList&& o) { copyUnion(std::move(o)); }
			BasicSizeList(const BasicSizeList& o) { copyUnion(o); }

			BasicSizeList& operator=(BasicSizeList&& o)
			{
				destroyUnion();
				copyUnion(std::move(o));
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
			bool hasEntity() const { return t == Type::ENTITY_NUMBER || t == Type::EMPTY_ENTITY_NUMBER; }

			Type getType() const { return t; }

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

			const std::string& getEntName() const
			{
				assert(hasEntity());
				return ent.getName();
			}
		private:
			static constexpr char* ERROR_BINARY_SIZE_LIST = "size of binary list must be a positive integer or equivalent entity";

			Type t = Type::NONE;
			union
			{
				WebssBinarySize number;
				Entity ent;
			};

			void destroyUnion()
			{
				if (hasEntity())
					ent.~BasicEntity();
				t = Type::NONE;
			}

			void copyUnion(BasicSizeList&& o)
			{
				switch (o.t)
				{
				case Type::NONE: case Type::EMPTY: case Type::ONE:
					break;
				case Type::NUMBER:
					number = o.number;
					break;
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
					new (&ent) Entity(std::move(o.ent));
					o.ent.~BasicEntity();
					break;
				default:
					throw std::domain_error("");
				}
				t = o.t;
				o.t = Type::NONE;
			}
			void copyUnion(const BasicSizeList& o)
			{
				switch (o.t)
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
				t = o.t;
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
			sizeHead = std::move(o.sizeHead);
			sizeList = std::move(o.sizeList);
			return *this;
		}

		SizeHead sizeHead;
		SizeList sizeList;
	};
}