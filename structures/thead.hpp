//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <string>

#include "base.hpp"
#include "enum.hpp"
#include "entity.hpp"
#include "keywords.hpp"
#include "namespace.hpp"
#include "types.hpp"
#include "typeWebss.hpp"

namespace webss
{
	enum class TypeThead { NONE, ENTITY, SELF, BIN, STD };

	struct TheadOptions
	{
		bool isText = false;
		bool isPlus = false;
	};

	class Thead
	{
	public:
		Thead();
		~Thead();

		Thead(Thead&& o);
		Thead(const Thead& o);

		Thead& operator=(Thead o);

		Thead(Entity ent);
		Thead(TheadSelf, TheadOptions options = TheadOptions());
		Thead(TheadBin theadBin, TheadOptions options = TheadOptions());
		Thead(TheadStd theadStd, TheadOptions options = TheadOptions());

		bool operator==(const Thead& o) const;
		bool operator!=(const Thead& o) const;

		const Thead& getTheadLast() const;

		TypeThead getType() const;
		const TheadBin& getTheadBin() const;
		const TheadStd& getTheadStd() const;

		TypeThead getTypeRaw() const;

		const Entity& getEntityRaw() const;
		const TheadBin& getTheadBinRaw() const;
		const TheadStd& getTheadStdRaw() const;

		Entity& getEntityRaw();
		TheadBin& getTheadBinRaw();
		TheadStd& getTheadStdRaw();

		explicit operator const TheadBin&() const { return getTheadBin(); }
		explicit operator const TheadStd&() const { return getTheadStd(); }

		template <class Element>
		const Element& getElement() const { return static_cast<const Element&>(*this); }

		bool isNone() const;
		bool isTheadBin() const;
		bool isTheadStd() const;

		TheadOptions getOptions() const;

		bool isText() const;
		bool isPlus() const;

	private:
		TypeThead type = TypeThead::NONE;
		TheadOptions options;
		union
		{
			TheadBin* theadBin;
			TheadStd* theadStd;
			Entity ent;
		};

		void destroyUnion();
		void copyUnion(Thead&& o);
		void copyUnion(const Thead& o);
	};
}