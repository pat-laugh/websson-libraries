//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <assert.h>
#include <memory>

#include "base.h"
#include "keywords.h"
#include "types.h"
#include "templateHead.h"

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
			using TemplateHead = BasicTemplateHead<BasicParamBinary, Webss>;
			using Entity = BasicEntity<Webss>;
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, SELF, KEYWORD, NUMBER, TEMPLATE_HEAD, ENTITY_NUMBER, ENTITY_TEMPLATE_HEAD };

			BasicSizeHead() {}
			BasicSizeHead(Keyword keyword) : type(Type::KEYWORD)
			{
				switch (keyword)
				{
				case Keyword::BOOL: case Keyword::INT1: case Keyword::INT2: case Keyword::INT4: case Keyword::INT8: case Keyword::DEC4: case Keyword::DEC8:
					this->keyword = keyword;
					break;
				case Keyword::STRING:
					type = Type::EMPTY;
					break;
				default:
					throw std::runtime_error("invalid binary type: " + keyword.toString());
				}
			}
			BasicSizeHead(const Entity& entFhead) : type(Type::ENTITY_TEMPLATE_HEAD), ent(entFhead) {}
			BasicSizeHead(const Entity& entNumber, bool number) : type(Type::ENTITY_NUMBER), ent(entNumber)
			{
				auto num = entNumber.getContent().getPrimitive<WebssBinarySize>();
				if (num <= 0)
				{
					if (num == 0)
						type = Type::EMPTY_ENTITY_NUMBER;
					else
					{
						ent.~BasicEntity();
						throw std::runtime_error(ERROR_BINARY_SIZE_HEAD);
					}
				}
			}
			BasicSizeHead(WebssBinarySize number) : type(Type::NUMBER), number(number)
			{
				if (number <= 0)
				{
					if (number == 0)
						type = Type::EMPTY;
					else
						throw std::runtime_error(ERROR_BINARY_SIZE_HEAD);
				}
			}
			BasicSizeHead(TemplateHead&& o) : type(Type::TEMPLATE_HEAD), thead(new TemplateHead(std::move(o))) {}
			BasicSizeHead(const TemplateHead& o) : type(Type::TEMPLATE_HEAD), thead(new TemplateHead(o)) {}

			BasicSizeHead(TemplateHeadSelf) : type(Type::SELF) {}

			BasicSizeHead(Type type) : type(type)
			{
				switch (type)
				{
				case Type::NONE: case Type::EMPTY: case Type::SELF:
					break;
				default:
					assert(false); throw std::domain_error("");
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

			bool isEmpty() const { return type == Type::EMPTY || type == Type::EMPTY_ENTITY_NUMBER; }
			bool isKeyword() const { return type == Type::KEYWORD; }
			bool isBool() const { return isKeyword() && keyword == Keyword::BOOL; }
			bool isTemplateHead() const { return type == Type::ENTITY_TEMPLATE_HEAD || type == Type::TEMPLATE_HEAD; }
			bool hasEntity() const { return type == Type::ENTITY_TEMPLATE_HEAD || type == Type::ENTITY_NUMBER || type == Type::EMPTY_ENTITY_NUMBER; }

			bool hasDefaultValue() const { return defaultValue.get() != nullptr; }
			bool isSelf() const { return type == Type::SELF; }

			Type getType() const{ return type; }
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
				switch (type)
				{
				case Type::TEMPLATE_HEAD:
					return *thead;
				case Type::ENTITY_TEMPLATE_HEAD:
					return ent.getContent().getElement<TemplateHead>();
				default:
					assert(false && "binary size head does not contain a template head"); throw std::domain_error("");
				}
			}

			WebssBinarySize size() const
			{
				switch (type)
				{
				case Type::KEYWORD:
					return keyword.getSize();
				case Type::NUMBER:
					return number;
				case Type::ENTITY_NUMBER:
					return ent.getContent().getPrimitive<WebssBinarySize>();
				default:
					assert(false); throw std::domain_error("");
				}
			}
		private:
			static constexpr char* ERROR_BINARY_SIZE_HEAD = "size of binary head must be a positive integer, binary template head or equivalent entity";

			Type type = Type::NONE;
			union
			{
				Keyword keyword;
				WebssBinarySize number;
				Entity ent;
				TemplateHead* thead;
			};

			std::shared_ptr<Webss> defaultValue;

			void destroyUnion()
			{
				switch (type)
				{
				case Type::TEMPLATE_HEAD:
					delete thead;
					break;
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_TEMPLATE_HEAD:
					ent.~BasicEntity();
					break;
				default:
					break;
				}
				type = Type::NONE;
			}

			void copyUnion(BasicSizeHead&& o)
			{
				switch (o.type)
				{
				case Type::NONE: case Type::EMPTY: case Type::SELF:
					break;
				case Type::KEYWORD:
					keyword = o.keyword;
					break;
				case Type::NUMBER:
					number = o.number;
					break;
				case Type::TEMPLATE_HEAD:
					thead = o.thead;
					break;
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_TEMPLATE_HEAD:
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
			void copyUnion(const BasicSizeHead& o)
			{
				switch (o.type)
				{
				case Type::NONE: case Type::EMPTY: case Type::SELF:
					break;
				case Type::KEYWORD:
					keyword = o.keyword;
					break;
				case Type::NUMBER:
					number = o.number;
					break;
				case Type::TEMPLATE_HEAD:
					thead = new TemplateHead(*o.thead);
					break;
				case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_TEMPLATE_HEAD:
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

		class BasicSizeList
		{
		public:
			using Entity = BasicEntity<Webss>;
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, ONE, NUMBER, ENTITY_NUMBER };

			BasicSizeList() {}
			BasicSizeList(Type type) : type(type)
			{
				switch (type)
				{
				case Type::NONE: case Type::EMPTY: case Type::ONE:
					break;
				default:
					assert(false); throw std::domain_error("");
				}
			}
			BasicSizeList(const Entity& entNumber) : type(Type::ENTITY_NUMBER), ent(entNumber)
			{
				auto num = entNumber.getContent().getPrimitive<WebssBinarySize>();
				if (num <= 0)
				{
					if (num == 0)
						type = Type::EMPTY_ENTITY_NUMBER;
					else
					{
						ent.~BasicEntity();
						throw std::runtime_error(ERROR_BINARY_SIZE_LIST);
					}
				}
			}
			BasicSizeList(WebssBinarySize number) : type(Type::NUMBER), number(number)
			{
				if (number <= 0)
				{
					if (number == 0)
						type = Type::EMPTY;
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

			bool isEmpty() const { return type == Type::EMPTY || type == Type::EMPTY_ENTITY_NUMBER; }
			bool isOne() const { return type == Type::ONE; }
			bool hasEntity() const { return type == Type::ENTITY_NUMBER || type == Type::EMPTY_ENTITY_NUMBER; }

			Type getType() const { return type; }

			WebssBinarySize size() const
			{
				switch (type)
				{
				case Type::NUMBER:
					return number;
				case Type::ENTITY_NUMBER:
					return ent.getContent().getPrimitive<WebssBinarySize>();
				default:
					assert(false); throw std::domain_error("");
				}
			}

			const Entity& getEntity() const
			{
				assert(hasEntity());
				return ent;
			}
		private:
			static constexpr char* ERROR_BINARY_SIZE_LIST = "size of binary list must be a positive integer or equivalent entity";

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

		using SizeHead = BasicSizeHead<Webss>;
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

		SizeHead sizeHead;
		SizeList sizeList;
	};
}