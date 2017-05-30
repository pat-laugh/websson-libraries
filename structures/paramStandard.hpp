//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <memory>

#include "paramBinary.hpp"
#include "documentHead.hpp"
#include "typeWebss.hpp"

namespace webss
{
	class ParamStandard
	{
	private:
		WebssType typeThead = WebssType::NONE;
		union
		{
			TemplateHeadBinary* theadBin;
			TemplateHeadStandard* theadStd;
		};

		std::shared_ptr<Webss> defaultValue;
	public:
		ParamStandard();
		ParamStandard(Webss&& webss);
		~ParamStandard();

		ParamStandard(ParamStandard&& o);
		ParamStandard(const ParamStandard& o);

		ParamStandard& operator=(ParamStandard&& o);
		ParamStandard& operator=(const ParamStandard& o);

		bool hasDefaultValue() const;
		bool hasTemplateHead() const;

		const Webss& getDefaultValue() const;
		const std::shared_ptr<Webss>& getDefaultPointer() const;

		//returns WebssType::NONE if has no thead
		WebssType getTypeThead() const;

		const TemplateHeadBinary& getTemplateHeadBinary() const;
		const TemplateHeadStandard& getTemplateHeadStandard() const;

		void removeTemplateHead();
		void setTemplateHead(TemplateHeadBinary&& o);
		void setTemplateHead(TemplateHeadStandard&& o);
		void setTemplateHead(TemplateHeadStandard&& o, bool);
		void setTemplateHead(TemplateHeadSelf);
	private:
		void destroyUnion();

		void copyUnion(ParamStandard&& o);
		void copyUnion(const ParamStandard& o);
	};
}