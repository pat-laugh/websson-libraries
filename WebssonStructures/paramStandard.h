//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <memory>

#include "base.h"
#include "paramBinary.h"
#include "templateHead.h"
#include "templateScoped.h"
#include "typeWebss.h"

namespace webss
{
#define This BasicParamStandard
	template <class Webss>
	class This
	{
	private:
		using TheadBinary = BasicTemplateHead<BasicParamBinary<Webss>, Webss>;
		using TheadScoped = BasicTemplateHeadScoped<Webss>;
		using TheadStandard = BasicTemplateHead<This<Webss>, Webss>;
		using TheadText = TheadStandard;

		WebssType typeThead = WebssType::NONE;
		union
		{
			TheadBinary* theadBin;
			TheadScoped* theadScoped;
			TheadStandard* theadStd;
			TheadText* theadText;
		};

		std::shared_ptr<Webss> defaultValue;
	public:
		This() {}
		This(Webss&& webss) : defaultValue(new Webss(std::move(webss))) {}
		~This() { destroyUnion(); }

		This(This&& o) { copyUnion(std::move(o)); }
		This(const This& o) { copyUnion(o); }

		This& operator=(This&& o)
		{
			destroyUnion();
			copyUnion(std::move(o));
			return *this;
		}
		This& operator=(const This& o)
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
			assert(typeThead == WebssType::TEMPLATE_HEAD_STANDARD); 
			return *theadStd;
		}
		const TheadText& getTemplateHeadText() const
		{
			assert(typeThead == WebssType::TEMPLATE_HEAD_TEXT);
			return *theadText; 
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
		void setTemplateHead(TheadStandard&& o, bool isText)
		{
			assert(!hasTemplateHead());
			theadText = new TheadText(std::move(o));
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
			case WebssType::TEMPLATE_HEAD_STANDARD:
				delete theadStd;
				break;
			case WebssType::TEMPLATE_HEAD_TEXT:
				delete theadText;
				break;
			default:
				assert(false); throw std::domain_error("");
			}
			typeThead = WebssType::NONE;
		}

		void copyUnion(This&& o)
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
			case WebssType::TEMPLATE_HEAD_STANDARD:
				theadStd = o.theadStd;
				break;
			case WebssType::TEMPLATE_HEAD_TEXT:
				theadText = o.theadText;
				break;
			default:
				assert(false); throw std::domain_error("");
			}
			typeThead = o.typeThead;
			o.typeThead = WebssType::NONE;
			defaultValue = std::move(o.defaultValue);
		}
		void copyUnion(const This& o)
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
			case WebssType::TEMPLATE_HEAD_STANDARD:
				theadStd = new TheadStandard(*o.theadStd);
				break;
			case WebssType::TEMPLATE_HEAD_TEXT:
				theadText = new TheadText(*o.theadText);
				break;
			default:
				assert(false); throw std::domain_error("");
			}
			typeThead = o.typeThead;
			defaultValue = o.defaultValue;
		}
	};
#undef This
}