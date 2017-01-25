#pragma once

#include "functionHead.h"
#include "paramBinary.h"
#include "functionScoped.h"
#include "typeWebss.h"
#include <memory>

namespace webss
{
#define This BasicParamStandard
	template <class Webss>
	class This
	{
	public:
		using FheadBinary = BasicFunctionHead<BasicParamBinary<Webss>>;
		using FheadScoped = BasicFunctionHeadScoped<Webss>;
		using FheadStandard = BasicFunctionHead<This<Webss>>;

		This() {}

		This(Webss&& webss) : typeFhead(WebssType::NONE), defaultValue(new Webss(std::move(webss))) {}
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

		const Webss& getDefaultValue() const { return *defaultValue; }
		const std::shared_ptr<Webss>& getDefaultPointer() const { return defaultValue; }

		WebssType getTypeFhead() const { return typeFhead; }

		const FheadBinary& getFunctionHeadBinary() const { return *fheadBin; }
		const FheadScoped& getFunctionHeadScoped() const { return *fheadScoped; }
		const FheadStandard& getFunctionHeadStandard() const { return *fheadStd; }

		void removeFunctionHead() { destroyUnion(); }
		void setFunctionHead(FheadBinary&& o)
		{
			fheadBin = new FheadBinary(std::move(o));
			typeFhead = WebssType::FUNCTION_HEAD_BINARY;
		}
		void setFunctionHead(FheadScoped&& o)
		{
			fheadScoped = new FheadScoped(std::move(o));
			typeFhead = WebssType::FUNCTION_HEAD_SCOPED;
		}
		void setFunctionHead(FheadStandard&& o)
		{
			fheadStd = new FheadStandard(std::move(o));
			typeFhead = WebssType::FUNCTION_HEAD_STANDARD;
		}
	private:
		WebssType typeFhead = WebssType::NONE;
		union
		{
			FheadBinary* fheadBin;
			FheadScoped* fheadScoped;
			FheadStandard* fheadStd;
		};

		std::shared_ptr<Webss> defaultValue;

		void destroyUnion()
		{
			switch (typeFhead)
			{
			case WebssType::FUNCTION_HEAD_BINARY:
				delete fheadBin;
				break;
			case WebssType::FUNCTION_HEAD_SCOPED:
				delete fheadScoped;
				break;
			case WebssType::FUNCTION_HEAD_STANDARD:
				delete fheadStd;
				break;
			default:
				break;
			}
			typeFhead = WebssType::NONE;
		}

		void copyUnion(This&& o)
		{
			switch (o.typeFhead)
			{
			case WebssType::FUNCTION_HEAD_BINARY:
				fheadBin = o.fheadBin;
				break;
			case WebssType::FUNCTION_HEAD_SCOPED:
				fheadScoped = o.fheadScoped;
				break;
			case WebssType::FUNCTION_HEAD_STANDARD:
				fheadStd = o.fheadStd;
				break;
			default:
				break;
			}
			typeFhead = o.typeFhead;
			o.typeFhead = WebssType::NONE;
			defaultValue = std::move(o.defaultValue);
		}
		void copyUnion(const This& o)
		{
			switch (o.typeFhead)
			{
			case WebssType::FUNCTION_HEAD_BINARY:
				fheadBin = new FheadBinary(*o.fheadBin);
				break;
			case WebssType::FUNCTION_HEAD_SCOPED:
				fheadScoped = new FheadScoped(*o.fheadScoped);
				break;
			case WebssType::FUNCTION_HEAD_STANDARD:
				fheadStd = new FheadStandard(*o.fheadStd);
				break;
			default:
				break;
			}
			typeFhead = o.typeFhead;
			defaultValue = o.defaultValue;
		}
	};
#undef This
}