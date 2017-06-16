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
	class ParamBin
	{
	public:
		class SizeHead
		{
		public:
			enum class Type { NONE, EMPTY, EMPTY_ENTITY_NUMBER, SELF, KEYWORD, NUMBER, THEAD, ENTITY_NUMBER, ENTITY_THEAD, BITS, ENTITY_BITS };

			SizeHead();
			SizeHead(Keyword keyword);

			static SizeHead makeEntityThead(const Entity& entThead) { return SizeHead(entThead); }
			static SizeHead makeEntityNumber(const Entity& entNumber) { return SizeHead(entNumber, true); }
			static SizeHead makeSizeBits(WebssBinSize num) { return SizeHead(num, true); }
			static SizeHead makeEntityBits(const Entity& entNumber) { return SizeHead(entNumber, true, true); }

			SizeHead(WebssBinSize num);
			SizeHead(TheadSelf);
			SizeHead(Type type);

			~SizeHead();

			SizeHead(TheadBin&& o);
			SizeHead(const TheadBin& o);

			SizeHead(SizeHead&& o);
			SizeHead(const SizeHead& o);

			SizeHead& operator=(SizeHead&& o);
			SizeHead& operator=(const SizeHead& o);

			bool operator==(const SizeHead& o) const;
			bool operator!=(const SizeHead& o) const;

			bool isEmpty() const;
			bool isKeyword() const;
			bool isBool() const;
			bool isTheadBin() const;

			bool hasEntity() const;

			bool hasDefaultValue() const;
			bool isTheadSelf() const;

			Type getType() const;
			Keyword getKeyword() const;

			const Webss& getDefaultValue() const;
			const std::shared_ptr<Webss>& getDefaultPointer() const;
			void setDefaultValue(Webss&& value);

			const Entity& getEntity() const;

			const TheadBin& getThead() const;

			WebssBinSize size() const;

		private:
			Type type = Type::NONE;
			union
			{
				Keyword keyword;
				WebssBinSize number;
				Entity ent;
				TheadBin* thead;
			};

			std::shared_ptr<Webss> defaultValue;

			SizeHead(const Entity& entThead);
			SizeHead(const Entity& entNumber, bool);
			SizeHead(WebssBinSize num, bool);
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
			SizeList(WebssBinSize num);

			~SizeList();

			SizeList(SizeList&& o);
			SizeList(const SizeList& o);

			SizeList& operator=(SizeList&& o);
			SizeList& operator=(const SizeList& o);

			bool operator==(const SizeList& o) const;
			bool operator!=(const SizeList& o) const;

			bool isEmpty() const;
			bool isOne() const;
			bool hasEntity() const;

			Type getType() const;

			WebssBinSize size() const;

			const Entity& getEntity() const;

		private:
			Type type = Type::NONE;
			union
			{
				WebssBinSize number;
				Entity ent;
			};

			void destroyUnion();
			void copyUnion(SizeList&& o);
			void copyUnion(const SizeList& o);
		};

		ParamBin();
		ParamBin(SizeHead&& sizeHead, SizeList&& sizeList);
		ParamBin(const SizeHead& sizeHead, const SizeList& sizeList);
		~ParamBin();

		ParamBin(ParamBin&& o);
		ParamBin(const ParamBin& o);

		ParamBin& operator=(ParamBin&& o);
		ParamBin& operator=(const ParamBin& o);

		bool operator==(const ParamBin& o) const;
		bool operator!=(const ParamBin& o) const;

		const SizeHead& getSizeHead() const;
		const SizeList& getSizeList() const;

		//functions for sizeHead
		bool hasDefaultValue() const;
		const std::shared_ptr<Webss>& getDefaultPointer() const;
		bool isTheadBin() const;
		bool isTheadSelf() const;
		const TheadBin& getThead() const;

	private:
		SizeHead sizeHead;
		SizeList sizeList;
	};
}
