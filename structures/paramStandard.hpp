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
	class ParamStd
	{
	private:
		std::unique_ptr<Thead> thead;
		std::shared_ptr<Webss> defaultValue; //shared pointer because value might be assigned to void params

	public:
		ParamStd();
		ParamStd(Webss defaultValue);

		ParamStd(ParamStd&& o);
		ParamStd(const ParamStd& o);

		ParamStd& operator=(ParamStd o);

		bool operator==(const ParamStd& o) const;
		bool operator!=(const ParamStd& o) const;

		bool hasDefaultValue() const;
		const Webss& getDefaultValue() const;
		const std::shared_ptr<Webss>& getDefaultPointer() const;

		bool hasThead() const;
		const Thead& getThead() const;

		//must have a thead
		TypeThead getTypeThead() const;

		const TheadStd& getTheadStd() const;
		const TheadBin& getTheadBin() const;

		bool isTextThead() const;
		bool isPlusThead() const;

		void setThead(Thead thead);
		void removeThead();
	};
}