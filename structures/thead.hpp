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
	enum class TypeThead { NONE, ENTITY, SELF, BIN, STD, FUN };

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
		Thead(Entity ent, TheadOptions options);
		Thead(TheadSelf);
		Thead(TheadBin theadBin, TheadOptions options = TheadOptions());
		Thead(TheadFun theadFun, TheadOptions options = TheadOptions());
		Thead(TheadStd theadStd, TheadOptions options = TheadOptions());
		Thead(TheadBin theadBin, Entity base, TheadOptions options = TheadOptions());
		Thead(TheadFun theadFun, Entity base, TheadOptions options = TheadOptions());
		Thead(TheadStd theadStd, Entity base, TheadOptions options = TheadOptions());
		Thead(TheadFun theadFun, Entity base, TheadOptions options, Tuple modifierTuple);
		Thead(TheadStd theadStd, Entity base, TheadOptions options, Tuple modifierTuple);

		bool operator==(const Thead& o) const;
		bool operator!=(const Thead& o) const;

		const Thead& getTheadLast() const;

		TypeThead getType() const;
		const TheadBin& getTheadBin() const;
		const TheadFun& getTheadFun() const;
		const TheadStd& getTheadStd() const;

		TypeThead getTypeRaw() const;

		bool hasEntity() const;

		const Entity& getEntityRaw() const;
		const TheadBin& getTheadBinRaw() const;
		const TheadFun& getTheadFunRaw() const;
		const TheadStd& getTheadStdRaw() const;

		Entity& getEntityRaw();
		TheadBin& getTheadBinRaw();
		TheadFun& getTheadFunRaw();
		TheadStd& getTheadStdRaw();

		explicit operator const TheadBin&() const { return getTheadBin(); }
		explicit operator const TheadFun&() const { return getTheadFun(); }
		explicit operator const TheadStd&() const { return getTheadStd(); }

		template <class Element>
		const Element& getElement() const { return static_cast<const Element&>(*this); }

		bool isNone() const;
		bool isTheadBin() const;
		bool isTheadFun() const;
		bool isTheadStd() const;

		TheadOptions getOptions() const;

		bool isText() const;
		bool isPlus() const;

		bool hasBase() const;
		const Entity& getBase() const;

		bool hasModifierTuple() const;
		const Tuple& getModifierTuple() const;

	private:
		TypeThead type = TypeThead::NONE;
		TheadOptions options;
		union
		{
			TheadBin* theadBin;
			TheadFun* theadFun;
			TheadStd* theadStd;
			Entity ent;
		};
		Entity base;
		Tuple* modifierTuple = nullptr;

		void destroyUnion();
		void copyUnion(Thead&& o);
		void copyUnion(const Thead& o);
	};
}