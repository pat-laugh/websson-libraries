//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <assert.h>
#include <limits>
#include <memory>

#include "base.h"
#include "keywords.h"
#include "templateHead.h"
#include "types.h"

namespace webss
{
	template <class Webss2>
	class BasicParamBinary
	{
	public:
#define entNumInt entNumber.getContent().getIntSafe()
#define This BasicSizeHead
		template <class Webss>
		class This
		{
		public:
			using TemplateHead = BasicTemplateHead<BasicParamBinary, Webss>;
			using Entity = BasicEntity<Webss>;
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, SELF, KEYWORD, NUMBER, TEMPLATE_HEAD, ENTITY_NUMBER, ENTITY_TEMPLATE_HEAD, BITS, ENTITY_BITS };

			This() {}
			This(Keyword keyword) : type(Type::KEYWORD)
			{
				switch (keyword)
				{
				case Keyword::BOOL: case Keyword::INT8: case Keyword::INT16: case Keyword::INT32: case Keyword::INT64: case Keyword::FLOAT: case Keyword::DOUBLE:
					this->keyword = keyword;
					break;
				case Keyword::STRING:
					type = Type::EMPTY;
					break;
				default:
					assert(false); throw std::domain_error("invalid binary type: " + keyword.toString());
				}
			}

			static This makeEntityThead(const Entity& entThead) { return This(entThead); }
			static This makeEntityNumber(const Entity& entNumber) { return This(entNumber, true); }
			static This makeSizeBits(WebssBinarySize num) { return This(num, true); }
			static This makeEntityBits(const Entity& entNumber) { return This(entNumber, true, true); }

			This(WebssBinarySize num) : type(num == 0 ? Type::EMPTY : Type::NUMBER), number(num) {}
			This(TemplateHeadSelf) : type(Type::SELF) {}
			This(Type type) : type(type)
			{
				assert(type == Type::NONE || type == Type::EMPTY || type == Type::SELF);
			}

			~This() { destroyUnion(); }

			This(TemplateHead&& o) : type(Type::TEMPLATE_HEAD), thead(new TemplateHead(std::move(o))) {}
			This(const TemplateHead& o) : type(Type::TEMPLATE_HEAD), thead(new TemplateHead(o)) {}

			This(This&& o) { copyUnion(std::move(o)); }
			This(const This& o) { copyUnion(o); }

			This& operator=(This&& o)
			{
				destroyUnion();
				copyUnion(std::move(o));
				return *this;
			}
			This& operator=(const This& o)
			{
				if (this != &o)
				{
					destroyUnion();
					copyUnion(o);
				}
				return *this;
			}

			bool isEmpty() const { return type == Type::EMPTY || type == Type::EMPTY_ENTITY_NUMBER; }
			bool isKeyword() const { return type == Type::KEYWORD; }
			bool isBool() const { return isKeyword() && keyword == Keyword::BOOL; }
			bool isTemplateHead() const { return type == Type::ENTITY_TEMPLATE_HEAD || type == Type::TEMPLATE_HEAD; }
			bool hasEntity() const { return type == Type::ENTITY_TEMPLATE_HEAD || type == Type::ENTITY_NUMBER || type == Type::EMPTY_ENTITY_NUMBER || type == Type::ENTITY_BITS; }

			bool hasDefaultValue() const { return defaultValue.get() != nullptr; }
			bool isSelf() const { return type == Type::SELF; }

			Type getType() const { return type; }
			Keyword getKeyword() const
			{
				assert(isKeyword());
				return keyword;
			}

			const Webss& getDefaultValue() const
			{
				assert(hasDefaultValue());
				return *defaultValue;
			}
			const std::shared_ptr<Webss>& getDefaultPointer() const
			{
				assert(hasDefaultValue());
				return defaultValue;
			}
			void setDefaultValue(Webss&& value) { defaultValue = std::shared_ptr<Webss>(new Webss(std::move(value))); }

			const Entity& getEntity() const
			{
				assert(hasEntity());
				return ent;
			}

			const TemplateHead& getTemplateHead() const
			{
				assert(isTemplateHead());
				return type == Type::ENTITY_TEMPLATE_HEAD ? ent.getContent(). template getElement<TemplateHead>() : *thead;
			}

			WebssBinarySize size() const
			{
				switch (type)
				{
				case Type::ENTITY_NUMBER: case Type::ENTITY_BITS:
					return static_cast<WebssBinarySize>(ent.getContent().getIntSafe());
				case Type::NUMBER: case Type::BITS:
					return number;
#ifdef assert
				default:
					assert(false);
					throw std::domain_error("");
#endif
				}
			}
		private:
			static constexpr const char* ERROR_BINARY_SIZE_HEAD = "size of binary head must be a positive integer, binary template head or equivalent entity";

			Type type = Type::NONE;
			union
			{
				Keyword keyword;
				WebssBinarySize number;
				Entity ent;
				TemplateHead* thead;
			};

			std::shared_ptr<Webss> defaultValue;

			This(const Entity& entThead) : type(Type::ENTITY_TEMPLATE_HEAD), ent(entThead)
			{
				assert(entThead.getContent().isTemplateHeadBinary());
			}
			This(const Entity& entNumber, bool isNumber)
				: type(entNumInt == 0 ? Type::EMPTY_ENTITY_NUMBER : Type::ENTITY_NUMBER), ent(entNumber)
			{
				assert(entNumInt >= 0 && static_cast<WebssBinarySize>(entNumInt) <= std::numeric_limits<WebssBinarySize>::max());
			}
			This(WebssBinarySize num, bool bits) : type(Type::BITS), number(num)
			{
				assert(num > 0 && num <= 8);
			}
			This(const Entity& entNumber, bool isNumber, bool bits) : type(Type::ENTITY_BITS), ent(entNumber)
			{
				assert(entNumInt > 0 && entNumInt <= 8);
			}

			void destroyUnion()
			{
				switch (type)
				{
				case Type::TEMPLATE_HEAD:
					delete thead;
					break;
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_TEMPLATE_HEAD: case Type::ENTITY_BITS:
					ent.~BasicEntity();
					break;
				default:
					break;
				}
				type = Type::NONE;
			}

			void copyUnion(This&& o)
			{
				switch (o.type)
				{
				case Type::NONE: case Type::EMPTY: case Type::SELF:
					break;
				case Type::KEYWORD:
					keyword = o.keyword;
					break;
				case Type::NUMBER: case Type::BITS:
					number = o.number;
					break;
				case Type::TEMPLATE_HEAD:
					thead = o.thead;
					break;
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_TEMPLATE_HEAD: case Type::ENTITY_BITS:
					new (&ent) Entity(std::move(o.ent));
					o.ent.~BasicEntity();
					break;
				default:
					assert(false); throw std::domain_error("");
				}
				type = o.type;
				o.type = Type::NONE;

				if (o.hasDefaultValue())
					defaultValue = std::move(o.defaultValue);
			}
			void copyUnion(const This& o)
			{
				switch (o.type)
				{
				case Type::NONE: case Type::EMPTY: case Type::SELF:
					break;
				case Type::KEYWORD:
					keyword = o.keyword;
					break;
				case Type::NUMBER: case Type::BITS:
					number = o.number;
					break;
				case Type::TEMPLATE_HEAD:
					thead = new TemplateHead(*o.thead);
					break;
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_TEMPLATE_HEAD: case Type::ENTITY_BITS:
					new (&ent) Entity(o.ent);
					break;
				default:
					assert(false); throw std::domain_error("");
				}
				type = o.type;

				if (o.hasDefaultValue())
					defaultValue = o.defaultValue;
			}
		};
#undef This

		class BasicSizeList
		{
		public:
			using Entity = BasicEntity<Webss2>;
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, ONE, NUMBER, ENTITY_NUMBER };

			BasicSizeList() {}
			BasicSizeList(Type type) : type(type)
			{
				assert(type == Type::NONE || type == Type::EMPTY || type == Type::ONE);
			}
			BasicSizeList(const Entity& entNumber)
				: type(entNumInt == 0 ? Type::EMPTY_ENTITY_NUMBER : Type::ENTITY_NUMBER), ent(entNumber)
			{
				assert(entNumInt >= 0 && static_cast<WebssBinarySize>(entNumInt) <= std::numeric_limits<WebssBinarySize>::max());
			}
			BasicSizeList(WebssBinarySize num) : type(num == 0 ? Type::EMPTY : Type::NUMBER), number(num) {}

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

			bool isEmpty() const { return type == Type::EMPTY || type == Type::EMPTY_ENTITY_NUMBER; }
			bool isOne() const { return type == Type::ONE; }
			bool hasEntity() const { return type == Type::ENTITY_NUMBER || type == Type::EMPTY_ENTITY_NUMBER; }

			Type getType() const { return type; }

			WebssBinarySize size() const
			{
				assert(type == Type::NUMBER || type == Type::ENTITY_NUMBER);
				if (type == Type::NUMBER)
					return number;
				else
					return static_cast<WebssBinarySize>(ent.getContent().getIntSafe());
			}

			const Entity& getEntity() const
			{
				assert(hasEntity());
				return ent;
			}
		private:
			static constexpr const char* ERROR_BINARY_SIZE_LIST = "size of binary list must be a positive integer or equivalent entity";

			Type type = Type::NONE;
			union
			{
				WebssBinarySize number;
				Entity ent;
			};

			void destroyUnion()
			{
				if (hasEntity())
					ent.~BasicEntity();
				type = Type::NONE;
			}

			void copyUnion(BasicSizeList&& o)
			{
				switch (o.type)
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
					assert(false); throw std::domain_error("");
				}
				type = o.type;
				o.type = Type::NONE;
			}
			void copyUnion(const BasicSizeList& o)
			{
				switch (o.type)
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
					assert(false); throw std::domain_error("");
				}
				type = o.type;
			}
		};
#undef entNumInt

		using SizeHead = BasicSizeHead<Webss2>;
		using SizeList = BasicSizeList;

		BasicParamBinary() {}
		BasicParamBinary(SizeHead&& sizeHead, SizeList&& sizeList) : sizeHead(std::move(sizeHead)), sizeList(std::move(sizeList)) {}
		BasicParamBinary(const SizeHead& sizeHead, const SizeList& sizeList) : sizeHead(sizeHead), sizeList(sizeList) {}
		~BasicParamBinary() {}

		BasicParamBinary(BasicParamBinary&& o) : sizeHead(std::move(o.sizeHead)), sizeList(std::move(o.sizeList)) {}
		BasicParamBinary(const BasicParamBinary& o) : sizeHead(o.sizeHead), sizeList(o.sizeList) {}

		BasicParamBinary& operator=(BasicParamBinary&& o)
		{
			sizeHead = std::move(o.sizeHead);
			sizeList = std::move(o.sizeList);
			return *this;
		}
		BasicParamBinary& operator=(const BasicParamBinary& o)
		{
			if (this != &o)
			{
				sizeHead = o.sizeHead;
				sizeList = o.sizeList;
			}
			return *this;
		}

		const SizeHead& getSizeHead() const { return sizeHead; }
		const SizeList& getSizeList() const { return sizeList; }
	private:
		SizeHead sizeHead;
		SizeList sizeList;
	};
}
