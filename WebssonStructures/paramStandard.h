#pragma once

#include "functionHead.h"
#include "paramBinary.h"
#include "paramScoped.h"
#include <memory>

namespace webss
{
	template <class Webss>
	class BasicParamStandard
	{
	public:
		using FheadBinary = BasicFunctionHead<BasicParamBinary<Webss>>;
		using FheadScoped = BasicFunctionHead<BasicParamScoped<Webss>>;
		using FheadStandard = BasicFunctionHead<BasicParamStandard<Webss>>;
		using Default = std::shared_ptr<Webss>;
		enum class TypeFhead { NONE, BINARY, SCOPED, STANDARD };

		TypeFhead typeFhead;
		union
		{
			FheadBinary* fheadBin;
			FheadScoped* fheadScoped;
			FheadStandard* fheadStd;
		};

		Default defaultValue;


		BasicParamStandard() : typeFhead(TypeFhead::NONE) {}

		BasicParamStandard(Webss&& webss) : typeFhead(TypeFhead::NONE), defaultValue(new Webss(std::move(webss))) {}
		~BasicParamStandard() { destroyUnion(); }

		BasicParamStandard(BasicParamStandard&& o) { copyUnion(std::move(o)); }
		BasicParamStandard(const BasicParamStandard& o) { copyUnion(o); }

		BasicParamStandard& operator=(BasicParamStandard&& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(std::move(o));
			}
			return *this;
		}
		BasicParamStandard& operator=(const BasicParamStandard& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(o);
			}
			return *this;
		}

		bool hasDefaultValue() const { return defaultValue.get() != nullptr; }
		bool hasFunctionHead() const { return typeFhead != TypeFhead::NONE; }

		const Webss& getDefaultValue() const { return *defaultValue; }
		const Default& getDefaultPointer() const { return defaultValue; }

		TypeFhead getTypeFhead() const { return typeFhead; }

		const FheadBinary& getFunctionHeadBinary() const { return *fheadBin; }
		const FheadScoped& getFunctionHeadScoped() const { return *fheadScoped; }
		const FheadStandard& getFunctionHeadStandard() const { return *fheadStd; }

		void setFunctionHead(FheadBinary&& o)
		{
			typeFhead = TypeFhead::BINARY;
			fheadBin = new FheadBinary(std::move(o));
		}
		void setFunctionHead(FheadScoped&& o)
		{
			typeFhead = TypeFhead::SCOPED;
			fheadScoped = new FheadScoped(std::move(o));
		}
		void setFunctionHead(FheadStandard&& o)
		{
			typeFhead = TypeFhead::STANDARD;
			fheadStd = new FheadStandard(std::move(o));
		}
	private:
		void destroyUnion()
		{
			switch (typeFhead)
			{
			case TypeFhead::BINARY:
				delete fheadBin;
				break;
			case TypeFhead::SCOPED:
				delete fheadScoped;
				break;
			case TypeFhead::STANDARD:
				delete fheadStd;
				break;
			default:
				break;
			}
		}

		void copyUnion(BasicParamStandard&& o)
		{
			switch (typeFhead = o.typeFhead)
			{
			case TypeFhead::BINARY:
				fheadBin = o.fheadBin;
				break;
			case TypeFhead::SCOPED:
				fheadScoped = o.fheadScoped;
				break;
			case TypeFhead::STANDARD:
				fheadStd = o.fheadStd;
				break;
			default:
				break;
			}

			o.typeFhead = TypeFhead::NONE;
			defaultValue = std::move(o.defaultValue);
		}
		void copyUnion(const BasicParamStandard& o)
		{
			switch (typeFhead = o.typeFhead)
			{
			case TypeFhead::BINARY:
				fheadBin = new FheadBinary(*o.fheadBin);
				break;
			case TypeFhead::SCOPED:
				fheadScoped = new FheadScoped(*o.fheadScoped);
				break;
			case TypeFhead::STANDARD:
				fheadStd = new FheadStandard(*o.fheadStd);
				break;
			default:
				break;
			}

			defaultValue = o.defaultValue;
		}
	};
}