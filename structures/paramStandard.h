//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <memory>

#include "paramBinary.h"
#include "documentHead.h"
#include "typeWebss.h"

namespace webss
{
	class ParamStandard
	{
	private:
		using TheadBinary = BasicTemplateHead<ParamBinary>;
		using TheadScoped = TemplateHeadScoped;
		using TheadStandard = BasicTemplateHead<ParamStandard>;

		WebssType typeThead = WebssType::NONE;
		union
		{
			TheadBinary* theadBin;
			TheadScoped* theadScoped;
			TheadStandard* theadStd;
		};

		std::shared_ptr<Webss> defaultValue;
	public:
		ParamStandard() {}
		ParamStandard(Webss&& webss) : defaultValue(new Webss(std::move(webss))) {}
		~ParamStandard() { destroyUnion(); }

		ParamStandard(ParamStandard&& o) { copyUnion(std::move(o)); }
		ParamStandard(const ParamStandard& o) { copyUnion(o); }

		ParamStandard& operator=(ParamStandard&& o)
		{
			destroyUnion();
			copyUnion(std::move(o));
			return *this;
		}
		ParamStandard& operator=(const ParamStandard& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(o);
			}
			return *this;
		}

		bool hasDefaultValue() const { return defaultValue.get() != nullptr; }
		bool hasTemplateHead() const { return typeThead != WebssType::NONE; }

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

		//returns WebssType::NONE if has no thead
		WebssType getTypeThead() const
		{
			return typeThead;
		}

		const TheadBinary& getTemplateHeadBinary() const
		{
			assert(typeThead == WebssType::TEMPLATE_HEAD_BINARY);
			return *theadBin;
		}
		const TheadScoped& getTemplateHeadScoped() const
		{
			assert(typeThead == WebssType::TEMPLATE_HEAD_SCOPED);
			return *theadScoped;
		}
		const TheadStandard& getTemplateHeadStandard() const
		{
			assert(typeThead == WebssType::TEMPLATE_HEAD_STANDARD || typeThead == WebssType::TEMPLATE_HEAD_TEXT);
			return *theadStd;
		}

		void removeTemplateHead() { destroyUnion(); }
		void setTemplateHead(TheadBinary&& o)
		{
			assert(!hasTemplateHead());
			theadBin = new TheadBinary(std::move(o));
			typeThead = WebssType::TEMPLATE_HEAD_BINARY;
		}
		void setTemplateHead(TheadScoped&& o)
		{
			assert(!hasTemplateHead());
			theadScoped = new TheadScoped(std::move(o));
			typeThead = WebssType::TEMPLATE_HEAD_SCOPED;
		}
		void setTemplateHead(TheadStandard&& o)
		{
			assert(!hasTemplateHead());
			theadStd = new TheadStandard(std::move(o));
			typeThead = WebssType::TEMPLATE_HEAD_STANDARD;
		}
		void setTemplateHead(TheadStandard&& o, bool)
		{
			assert(!hasTemplateHead());
			theadStd = new TheadStandard(std::move(o));
			typeThead = WebssType::TEMPLATE_HEAD_TEXT;
		}
		void setTemplateHead(TemplateHeadSelf)
		{
			assert(!hasTemplateHead());
			typeThead = WebssType::TEMPLATE_HEAD_SELF;
		}
	private:
		void destroyUnion()
		{
			switch (typeThead)
			{
			case WebssType::NONE: case WebssType::TEMPLATE_HEAD_SELF:
				break;
			case WebssType::TEMPLATE_HEAD_BINARY:
				delete theadBin;
				break;
			case WebssType::TEMPLATE_HEAD_SCOPED:
				delete theadScoped;
				break;
			case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
				delete theadStd;
				break;
			default:
				assert(false); throw std::domain_error("");
			}
			typeThead = WebssType::NONE;
		}

		void copyUnion(ParamStandard&& o)
		{
			switch (o.typeThead)
			{
			case WebssType::NONE: case WebssType::TEMPLATE_HEAD_SELF:
				break;
			case WebssType::TEMPLATE_HEAD_BINARY:
				theadBin = o.theadBin;
				break;
			case WebssType::TEMPLATE_HEAD_SCOPED:
				theadScoped = o.theadScoped;
				break;
			case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
				theadStd = o.theadStd;
				break;
			default:
				assert(false); throw std::domain_error("");
			}
			typeThead = o.typeThead;
			o.typeThead = WebssType::NONE;
			defaultValue = std::move(o.defaultValue);
		}
		void copyUnion(const ParamStandard& o)
		{
			switch (o.typeThead)
			{
			case WebssType::NONE: case WebssType::TEMPLATE_HEAD_SELF:
				break;
			case WebssType::TEMPLATE_HEAD_BINARY:
				theadBin = new TheadBinary(*o.theadBin);
				break;
			case WebssType::TEMPLATE_HEAD_SCOPED:
				theadScoped = new TheadScoped(*o.theadScoped);
				break;
			case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
				theadStd = new TheadStandard(*o.theadStd);
				break;
			default:
				assert(false); throw std::domain_error("");
			}
			typeThead = o.typeThead;
			defaultValue = o.defaultValue;
		}
	};
}