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
	enum class TheadType { NONE, ENTITY, SELF, BINARY, STANDARD };

	class Thead
	{
	public:
		Thead();
		~Thead();

		Thead(Thead&& o);
		Thead(const Thead& o);

		Thead& operator=(Thead o);

		Thead(Entity ent);
		Thead(TemplateHeadSelf, bool isText = false, bool isPlus = false);
		Thead(TemplateHeadBinary theadBinary, bool isText = false, bool isPlus = false);
		Thead(TemplateHeadStandard theadStandard, bool isText = false, bool isPlus = false);

		bool operator==(const Thead& o) const;
		bool operator!=(const Thead& o) const;

		const Thead& getTheadLast() const;

		TheadType getType() const;
		const TemplateHeadBinary& getTheadBinary() const;
		const TemplateHeadStandard& getTheadStandard() const;

		TheadType getTypeRaw() const;

		const Entity& getEntityRaw() const;
		const TemplateHeadBinary& getTheadBinaryRaw() const;
		const TemplateHeadStandard& getTheadStandardRaw() const;

		Entity& getEntityRaw();
		TemplateHeadBinary& getTheadBinaryRaw();
		TemplateHeadStandard& getTheadStandardRaw();

		explicit operator const TemplateHeadBinary&() const { return getTheadBinary(); }
		explicit operator const TemplateHeadStandard&() const { return getTheadStandard(); }

		template <class Element>
		const Element& getElement() const { return static_cast<const Element&>(*this); }

		bool isNone() const;
		bool isTheadBinary() const;
		bool isTheadStandard() const;

	private:
		TheadType type = TheadType::NONE;
		bool isText, isPlus;
		union
		{
			TemplateHeadBinary* theadBinary;
			TemplateHeadStandard* theadStandard;
			Entity ent;
		};

		void destroyUnion();
		void copyUnion(Thead&& o);
		void copyUnion(const Thead& o);
	};
}