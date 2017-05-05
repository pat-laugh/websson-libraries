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
	class ParamBinary
	{
	public:
#define entNumInt entNumber.getContent().getInt()
		class SizeHead
		{
		public:
			using TemplateHead = BasicTemplateHead<ParamBinary>;
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, SELF, KEYWORD, NUMBER, TEMPLATE_HEAD, ENTITY_NUMBER, ENTITY_TEMPLATE_HEAD, BITS, ENTITY_BITS };

			SizeHead() {}
			SizeHead(Keyword keyword) : type(Type::KEYWORD)
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

			static SizeHead makeEntityThead(const Entity& entThead) { return SizeHead(entThead); }
			static SizeHead makeEntityNumber(const Entity& entNumber) { return SizeHead(entNumber, true); }
			static SizeHead makeSizeBits(WebssBinarySize num) { return SizeHead(num, true); }
			static SizeHead makeEntityBits(const Entity& entNumber) { return SizeHead(entNumber, true, true); }

			SizeHead(WebssBinarySize num) : type(num == 0 ? Type::EMPTY : Type::NUMBER), number(num) {}
			SizeHead(TemplateHeadSelf) : type(Type::SELF) {}
			SizeHead(Type type) : type(type)
			{
				assert(type == Type::NONE || type == Type::EMPTY || type == Type::SELF);
			}

			~SizeHead() { destroyUnion(); }

			SizeHead(TemplateHead&& o) : type(Type::TEMPLATE_HEAD), thead(new TemplateHead(std::move(o))) {}
			SizeHead(const TemplateHead& o) : type(Type::TEMPLATE_HEAD), thead(new TemplateHead(o)) {}

			SizeHead(SizeHead&& o) { copyUnion(std::move(o)); }
			SizeHead(const SizeHead& o) { copyUnion(o); }

			SizeHead& operator=(SizeHead&& o)
			{
				destroyUnion();
				copyUnion(std::move(o));
				return *this;
			}
			SizeHead& operator=(const SizeHead& o)
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
				return type == Type::ENTITY_TEMPLATE_HEAD ? ent.getContent(). template getElement<TemplateHeadBinary>() : *thead;
			}

			WebssBinarySize size() const
			{
				switch (type)
				{
				case Type::ENTITY_NUMBER: case Type::ENTITY_BITS:
					return static_cast<WebssBinarySize>(ent.getContent().getInt());
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

			SizeHead(const Entity& entThead) : type(Type::ENTITY_TEMPLATE_HEAD), ent(entThead)
			{
				assert(entThead.getContent().isTemplateHeadBinary());
			}
			SizeHead(const Entity& entNumber, bool)
				: type(entNumInt == 0 ? Type::EMPTY_ENTITY_NUMBER : Type::ENTITY_NUMBER), ent(entNumber)
			{
				assert(entNumInt >= 0 && static_cast<WebssBinarySize>(entNumInt) <= std::numeric_limits<WebssBinarySize>::max());
			}
			SizeHead(WebssBinarySize num, bool) : type(Type::BITS), number(num)
			{
				assert(num > 0 && num <= 8);
			}
			SizeHead(const Entity& entNumber, bool, bool) : type(Type::ENTITY_BITS), ent(entNumber)
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
					ent.~Entity();
					break;
				default:
					break;
				}
				type = Type::NONE;
			}

			void copyUnion(SizeHead&& o)
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
					o.ent.~Entity();
					break;
				default:
					assert(false); throw std::domain_error("");
				}
				type = o.type;
				o.type = Type::NONE;

				if (o.hasDefaultValue())
					defaultValue = std::move(o.defaultValue);
			}
			void copyUnion(const SizeHead& o)
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

		class SizeList
		{
		public:
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, ONE, NUMBER, ENTITY_NUMBER };

			SizeList() {}
			SizeList(Type type) : type(type)
			{
				assert(type == Type::NONE || type == Type::EMPTY || type == Type::ONE);
			}
			SizeList(const Entity& entNumber)
				: type(entNumInt == 0 ? Type::EMPTY_ENTITY_NUMBER : Type::ENTITY_NUMBER), ent(entNumber)
			{
				assert(entNumInt >= 0 && static_cast<WebssBinarySize>(entNumInt) <= std::numeric_limits<WebssBinarySize>::max());
			}
			SizeList(WebssBinarySize num) : type(num == 0 ? Type::EMPTY : Type::NUMBER), number(num) {}

			~SizeList() { destroyUnion(); }

			SizeList(SizeList&& o) { copyUnion(std::move(o)); }
			SizeList(const SizeList& o) { copyUnion(o); }

			SizeList& operator=(SizeList&& o)
			{
				destroyUnion();
				copyUnion(std::move(o));
				return *this;
			}
			SizeList& operator=(const SizeList& o)
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
					return static_cast<WebssBinarySize>(ent.getContent().getInt());
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
					ent.~Entity();
				type = Type::NONE;
			}

			void copyUnion(SizeList&& o)
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
					o.ent.~Entity();
					break;
				default:
					assert(false); throw std::domain_error("");
				}
				type = o.type;
				o.type = Type::NONE;
			}
			void copyUnion(const SizeList& o)
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

		using SizeHead = SizeHead;
		using SizeList = SizeList;

		ParamBinary() {}
		ParamBinary(SizeHead&& sizeHead, SizeList&& sizeList) : sizeHead(std::move(sizeHead)), sizeList(std::move(sizeList)) {}
		ParamBinary(const SizeHead& sizeHead, const SizeList& sizeList) : sizeHead(sizeHead), sizeList(sizeList) {}
		~ParamBinary() {}

		ParamBinary(ParamBinary&& o) : sizeHead(std::move(o.sizeHead)), sizeList(std::move(o.sizeList)) {}
		ParamBinary(const ParamBinary& o) : sizeHead(o.sizeHead), sizeList(o.sizeList) {}

		ParamBinary& operator=(ParamBinary&& o)
		{
			sizeHead = std::move(o.sizeHead);
			sizeList = std::move(o.sizeList);
			return *this;
		}
		ParamBinary& operator=(const ParamBinary& o)
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
