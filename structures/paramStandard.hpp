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

		bool operator==(const ParamStandard& o) const;
		bool operator!=(const ParamStandard& o) const;

		bool hasDefaultValue() const;

		//returns true if the param has any kind of template head, else false
		bool hasTemplateHead() const;

		const Webss& getDefaultValue() const;
		const std::shared_ptr<Webss>& getDefaultPointer() const;

		//returns binary, self, standard, or text if it has a template head, else WebssType::NONE
		WebssType getTypeTemplateHead() const;

		const TemplateHeadBinary& getTemplateHeadBinary() const;
		const TemplateHeadStandard& getTemplateHeadStandard() const;

		void removeTemplateHead();
		void setTemplateHead(TemplateHeadBinary&& o, WebssType type = WebssType::TEMPLATE_HEAD_BINARY);
		void setTemplateHead(TemplateHeadStandard&& o, WebssType type = WebssType::TEMPLATE_HEAD_STANDARD);
		void setTemplateHead(TemplateHeadSelf);

	private:
		void destroyUnion();
		void copyUnion(ParamStandard&& o);
		void copyUnion(const ParamStandard& o);
	};
}