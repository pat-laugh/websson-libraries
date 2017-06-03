//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <cassert>
#include <limits>
#include <memory>

#include "keywords.hpp"
#include "templateHead.hpp"
#include "types.hpp"

namespace webss
{
	class ParamBinary
	{
	public:
		class SizeHead
		{
		public:
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, SELF, KEYWORD, NUMBER, TEMPLATE_HEAD, ENTITY_NUMBER, ENTITY_TEMPLATE_HEAD, BITS, ENTITY_BITS };

			SizeHead();
			SizeHead(Keyword keyword);

			static SizeHead makeEntityThead(const Entity& entThead) { return SizeHead(entThead); }
			static SizeHead makeEntityNumber(const Entity& entNumber) { return SizeHead(entNumber, true); }
			static SizeHead makeSizeBits(WebssBinarySize num) { return SizeHead(num, true); }
			static SizeHead makeEntityBits(const Entity& entNumber) { return SizeHead(entNumber, true, true); }

			SizeHead(WebssBinarySize num);
			SizeHead(TemplateHeadSelf);
			SizeHead(Type type);

			~SizeHead();

			SizeHead(TemplateHeadBinary&& o);
			SizeHead(const TemplateHeadBinary& o);

			SizeHead(SizeHead&& o);
			SizeHead(const SizeHead& o);

			SizeHead& operator=(SizeHead&& o);
			SizeHead& operator=(const SizeHead& o);

			bool isEmpty() const;
			bool isKeyword() const;
			bool isBool() const;
			bool isTemplateHeadBinary() const;

			bool hasEntity() const;

			bool hasDefaultValue() const;
			bool isTemplateHeadSelf() const;

			Type getType() const;
			Keyword getKeyword() const;

			const Webss& getDefaultValue() const;
			const std::shared_ptr<Webss>& getDefaultPointer() const;
			void setDefaultValue(Webss&& value);

			const Entity& getEntity() const;

			const TemplateHeadBinary& getTemplateHead() const;

			WebssBinarySize size() const;
		private:
			Type type = Type::NONE;
			union
			{
				Keyword keyword;
				WebssBinarySize number;
				Entity ent;
				TemplateHeadBinary* thead;
			};

			std::shared_ptr<Webss> defaultValue;

			SizeHead(const Entity& entThead);
			SizeHead(const Entity& entNumber, bool);
			SizeHead(WebssBinarySize num, bool);
			SizeHead(const Entity& entNumber, bool, bool);

			void destroyUnion();

			void copyUnion(SizeHead&& o);
			void copyUnion(const SizeHead& o);
		};

		class SizeList
		{
		public:
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, ONE, NUMBER, ENTITY_NUMBER };

			SizeList();
			SizeList(Type type);
			SizeList(const Entity& entNumber);
			SizeList(WebssBinarySize num);

			~SizeList();

			SizeList(SizeList&& o);
			SizeList(const SizeList& o);

			SizeList& operator=(SizeList&& o);
			SizeList& operator=(const SizeList& o);

			bool isEmpty() const;
			bool isOne() const;
			bool hasEntity() const;

			Type getType() const;

			WebssBinarySize size() const;

			const Entity& getEntity() const;
		private:
			Type type = Type::NONE;
			union
			{
				WebssBinarySize number;
				Entity ent;
			};

			void destroyUnion();

			void copyUnion(SizeList&& o);
			void copyUnion(const SizeList& o);
		};

		ParamBinary();
		ParamBinary(SizeHead&& sizeHead, SizeList&& sizeList);
		ParamBinary(const SizeHead& sizeHead, const SizeList& sizeList);
		~ParamBinary();

		ParamBinary(ParamBinary&& o);
		ParamBinary(const ParamBinary& o);

		ParamBinary& operator=(ParamBinary&& o);
		ParamBinary& operator=(const ParamBinary& o);

		const SizeHead& getSizeHead() const;
		const SizeList& getSizeList() const;

		//functions for sizeHead
		bool hasDefaultValue() const;
		const std::shared_ptr<Webss>& getDefaultPointer() const;
		bool isTemplateHeadBinary() const;
		bool isTemplateHeadSelf() const;
		const TemplateHeadBinary& getTemplateHead() const;

	private:
		SizeHead sizeHead;
		SizeList sizeList;
	};
}
