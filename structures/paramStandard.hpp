//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <memory>

#include "paramBinary.hpp"
#include "documentHead.hpp"
#include "typeWebss.hpp"
#include "thead.hpp"

namespace webss
{
	class ParamStandard
	{
	private:
		std::unique_ptr<Thead> thead;
		std::shared_ptr<Webss> defaultValue; //shared pointer because value might be assigned to void params

	public:
		ParamStandard();
		ParamStandard(Webss defaultValue);
		~ParamStandard();

		ParamStandard(ParamStandard&& o);
		ParamStandard(const ParamStandard& o);

		ParamStandard& operator=(ParamStandard o);

		bool operator==(const ParamStandard& o) const;
		bool operator!=(const ParamStandard& o) const;

		bool hasDefaultValue() const;
		const Webss& getDefaultValue() const;
		const std::shared_ptr<Webss>& getDefaultPointer() const;

		bool hasThead() const;
		const Thead& getThead() const;

		//must have a thead
		TypeThead getTypeThead() const;

		const TemplateHeadStandard& getTheadStd() const;
		const TemplateHeadBinary& getTheadBin() const;

		bool isTextThead() const;
		bool isPlusThead() const;

		void setThead(Thead thead);
		void removeThead();

	private:
		void destroyUnion();
		void copyUnion(ParamStandard&& o);
		void copyUnion(const ParamStandard& o);
	};
}