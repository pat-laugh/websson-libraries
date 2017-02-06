#pragma once

#include <memory>

#include "base.h"
#include "templateHead.h"
#include "paramBinary.h"
#include "paramText.h"
#include "templateScoped.h"
#include "typeWebss.h"

namespace webss
{
#define This BasicParamStandard
	template <class Webss>
	class This
	{
	private:
		using FheadBinary = BasicTemplateHead<BasicParamBinary<Webss>, Webss>;
		using FheadScoped = BasicTemplateHeadScoped<Webss>;
		using FheadStandard = BasicTemplateHead<This<Webss>, Webss>;
		using FheadText = BasicTemplateHead<BasicParamText<Webss>, Webss>;

		WebssType typeFhead = WebssType::NONE;
		union
		{
			FheadBinary* fheadBin;
			FheadScoped* fheadScoped;
			FheadStandard* fheadStd;
			FheadText* fheadText;
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
		bool hasTemplateHead() const { return typeFhead != WebssType::NONE; }

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
		WebssType getTypeFhead() const
		{
			return typeFhead;
		}

		const FheadBinary& getTemplateHeadBinary() const
		{
			assert(typeFhead == WebssType::TEMPLATE_HEAD_BINARY);
			return *fheadBin;
		}
		const FheadScoped& getTemplateHeadScoped() const
		{
			assert(typeFhead == WebssType::TEMPLATE_HEAD_SCOPED);
			return *fheadScoped; 
		}
		const FheadStandard& getTemplateHeadStandard() const
		{
			assert(typeFhead == WebssType::TEMPLATE_HEAD_STANDARD); 
			return *fheadStd;
		}
		const FheadText& getTemplateHeadText() const
		{
			assert(typeFhead == WebssType::TEMPLATE_HEAD_TEXT);
			return *fheadText; 
		}

		void removeTemplateHead() { destroyUnion(); }
		void setTemplateHead(FheadBinary&& o)
		{
			assert(!hasTemplateHead());
			fheadBin = new FheadBinary(std::move(o));
			typeFhead = WebssType::TEMPLATE_HEAD_BINARY;
		}
		void setTemplateHead(FheadScoped&& o)
		{
			assert(!hasTemplateHead());
			fheadScoped = new FheadScoped(std::move(o));
			typeFhead = WebssType::TEMPLATE_HEAD_SCOPED;
		}
		void setTemplateHead(FheadStandard&& o)
		{
			assert(!hasTemplateHead());
			fheadStd = new FheadStandard(std::move(o));
			typeFhead = WebssType::TEMPLATE_HEAD_STANDARD;
		}
		void setTemplateHead(FheadText&& o)
		{
			assert(!hasTemplateHead());
			fheadText = new FheadText(std::move(o));
			typeFhead = WebssType::TEMPLATE_HEAD_TEXT;
		}
		void setTemplateHead(TemplateHeadSelf)
		{
			assert(!hasTemplateHead());
			typeFhead = WebssType::TEMPLATE_HEAD_SELF;
		}
	private:
		void destroyUnion()
		{
			switch (typeFhead)
			{
			case WebssType::NONE: case WebssType::TEMPLATE_HEAD_SELF:
				break;
			case WebssType::TEMPLATE_HEAD_BINARY:
				delete fheadBin;
				break;
			case WebssType::TEMPLATE_HEAD_SCOPED:
				delete fheadScoped;
				break;
			case WebssType::TEMPLATE_HEAD_STANDARD:
				delete fheadStd;
				break;
			case WebssType::TEMPLATE_HEAD_TEXT:
				delete fheadText;
				break;
			default:
				assert(false); throw std::domain_error("");
			}
			typeFhead = WebssType::NONE;
		}

		void copyUnion(This&& o)
		{
			switch (o.typeFhead)
			{
			case WebssType::NONE: case WebssType::TEMPLATE_HEAD_SELF:
				break;
			case WebssType::TEMPLATE_HEAD_BINARY:
				fheadBin = o.fheadBin;
				break;
			case WebssType::TEMPLATE_HEAD_SCOPED:
				fheadScoped = o.fheadScoped;
				break;
			case WebssType::TEMPLATE_HEAD_STANDARD:
				fheadStd = o.fheadStd;
				break;
			case WebssType::TEMPLATE_HEAD_TEXT:
				fheadText = o.fheadText;
				break;
			default:
				assert(false); throw std::domain_error("");
			}
			typeFhead = o.typeFhead;
			o.typeFhead = WebssType::NONE;
			defaultValue = std::move(o.defaultValue);
		}
		void copyUnion(const This& o)
		{
			switch (o.typeFhead)
			{
			case WebssType::NONE: case WebssType::TEMPLATE_HEAD_SELF:
				break;
			case WebssType::TEMPLATE_HEAD_BINARY:
				fheadBin = new FheadBinary(*o.fheadBin);
				break;
			case WebssType::TEMPLATE_HEAD_SCOPED:
				fheadScoped = new FheadScoped(*o.fheadScoped);
				break;
			case WebssType::TEMPLATE_HEAD_STANDARD:
				fheadStd = new FheadStandard(*o.fheadStd);
				break;
			case WebssType::TEMPLATE_HEAD_TEXT:
				fheadText = new FheadText(*o.fheadText);
				break;
			default:
				assert(false); throw std::domain_error("");
			}
			typeFhead = o.typeFhead;
			defaultValue = o.defaultValue;
		}
	};
#undef This
}