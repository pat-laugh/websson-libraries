#pragma once

#include <memory>

#include "base.h"
#include "functionHead.h"
#include "paramBinary.h"
#include "paramText.h"
#include "functionScoped.h"
#include "typeWebss.h"

namespace webss
{
#define This BasicParamStandard
	template <class Webss>
	class This
	{
	private:
		using FheadBinary = BasicFunctionHead<BasicParamBinary<Webss>, Webss>;
		using FheadScoped = BasicFunctionHeadScoped<Webss>;
		using FheadStandard = BasicFunctionHead<This<Webss>, Webss>;
		using FheadText = BasicFunctionHead<BasicParamText<Webss>, Webss>;

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
		bool hasFunctionHead() const { return typeFhead != WebssType::NONE; }

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

		//returns WebssType::NONE if has no fhead
		WebssType getTypeFhead() const
		{
			return typeFhead;
		}

		const FheadBinary& getFunctionHeadBinary() const
		{
			assert(typeFhead == WebssType::FUNCTION_HEAD_BINARY);
			return *fheadBin;
		}
		const FheadScoped& getFunctionHeadScoped() const
		{
			assert(typeFhead == WebssType::FUNCTION_HEAD_SCOPED);
			return *fheadScoped; 
		}
		const FheadStandard& getFunctionHeadStandard() const
		{
			assert(typeFhead == WebssType::FUNCTION_HEAD_STANDARD); 
			return *fheadStd;
		}
		const FheadText& getFunctionHeadText() const
		{
			assert(typeFhead == WebssType::FUNCTION_HEAD_TEXT);
			return *fheadText; 
		}

		void removeFunctionHead() { destroyUnion(); }
		void setFunctionHead(FheadBinary&& o)
		{
			assert(!hasFunctionHead());
			fheadBin = new FheadBinary(std::move(o));
			typeFhead = WebssType::FUNCTION_HEAD_BINARY;
		}
		void setFunctionHead(FheadScoped&& o)
		{
			assert(!hasFunctionHead());
			fheadScoped = new FheadScoped(std::move(o));
			typeFhead = WebssType::FUNCTION_HEAD_SCOPED;
		}
		void setFunctionHead(FheadStandard&& o)
		{
			assert(!hasFunctionHead());
			fheadStd = new FheadStandard(std::move(o));
			typeFhead = WebssType::FUNCTION_HEAD_STANDARD;
		}
		void setFunctionHead(FheadText&& o)
		{
			assert(!hasFunctionHead());
			fheadText = new FheadText(std::move(o));
			typeFhead = WebssType::FUNCTION_HEAD_TEXT;
		}
		void setFunctionHead(FunctionHeadSelf)
		{
			assert(!hasFunctionHead());
			typeFhead = WebssType::FUNCTION_HEAD_SELF;
		}
	private:
		void destroyUnion()
		{
			switch (typeFhead)
			{
			case WebssType::NONE: case WebssType::FUNCTION_HEAD_SELF:
				break;
			case WebssType::FUNCTION_HEAD_BINARY:
				delete fheadBin;
				break;
			case WebssType::FUNCTION_HEAD_SCOPED:
				delete fheadScoped;
				break;
			case WebssType::FUNCTION_HEAD_STANDARD:
				delete fheadStd;
				break;
			case WebssType::FUNCTION_HEAD_TEXT:
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
			case WebssType::NONE: case WebssType::FUNCTION_HEAD_SELF:
				break;
			case WebssType::FUNCTION_HEAD_BINARY:
				fheadBin = o.fheadBin;
				break;
			case WebssType::FUNCTION_HEAD_SCOPED:
				fheadScoped = o.fheadScoped;
				break;
			case WebssType::FUNCTION_HEAD_STANDARD:
				fheadStd = o.fheadStd;
				break;
			case WebssType::FUNCTION_HEAD_TEXT:
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
			case WebssType::NONE: case WebssType::FUNCTION_HEAD_SELF:
				break;
			case WebssType::FUNCTION_HEAD_BINARY:
				fheadBin = new FheadBinary(*o.fheadBin);
				break;
			case WebssType::FUNCTION_HEAD_SCOPED:
				fheadScoped = new FheadScoped(*o.fheadScoped);
				break;
			case WebssType::FUNCTION_HEAD_STANDARD:
				fheadStd = new FheadStandard(*o.fheadStd);
				break;
			case WebssType::FUNCTION_HEAD_TEXT:
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