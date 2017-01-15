#pragma once

#include "functionHead.h"
#include "paramBinary.h"
#include <memory>

namespace webss
{
	template <class Webss>
	class BasicParamStandard
	{
	public:
		using FheadStandard = BasicFunctionHead<BasicParamStandard<Webss>>;
		using FheadBinary = BasicFunctionHead<BasicParamBinary<Webss>>;
		using Default = std::shared_ptr<Webss>;
		enum class TypeFhead { NONE, STANDARD, BINARY };

		TypeFhead typeFhead;
		union
		{
			FheadStandard* fheadStd;
			FheadBinary* fheadBin;
		};

		Default defaultValue;


		BasicParamStandard() : typeFhead(TypeFhead::NONE) {}

		BasicParamStandard(Webss&& webss) : typeFhead(TypeFhead::NONE), defaultValue(new Webss(std::move(webss))) {}

		BasicParamStandard(FheadStandard&& o) : typeFhead(TypeFhead::STANDARD), fheadStd(new FheadStandard(std::move(o))) {}
		BasicParamStandard(const FheadStandard& o) : typeFhead(TypeFhead::STANDARD), fheadStd(new FheadStandard(o)) {}
		BasicParamStandard(FheadBinary&& o) : typeFhead(TypeFhead::BINARY), entFheadBin(new FheadBinary(std::move(o))) {}
		BasicParamStandard(const FheadBinary& o) : typeFhead(TypeFhead::BINARY), entFheadBin(new FheadBinary(o)) {}
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

		const FheadStandard& getFunctionHeadStandard() const
		{
			return *fheadStd;
		}

		const FheadBinary& getFunctionHeadBinary() const
		{
			return *fheadBin;
		}

		void setFunctionHead(FheadStandard&& o)
		{
			typeFhead = TypeFhead::STANDARD;
			fheadStd = new FheadStandard(std::move(o));
		}
		void setFunctionHead(FheadBinary&& o)
		{
			typeFhead = TypeFhead::BINARY;
			fheadBin = new FheadBinary(std::move(o));
		}
	private:
		void destroyUnion()
		{
			switch (typeFhead)
			{
			case TypeFhead::STANDARD:
				delete fheadStd;
				break;
			case TypeFhead::BINARY:
				delete fheadBin;
				break;
			default:
				break;
			}
		}

		void copyUnion(BasicParamStandard&& o)
		{
			switch (typeFhead = o.typeFhead)
			{
			case TypeFhead::STANDARD:
				fheadStd = o.fheadStd;
				o.typeFhead = TypeFhead::NONE;
				break;
			case TypeFhead::BINARY:
				fheadBin = o.fheadBin;
				o.typeFhead = TypeFhead::NONE;
				break;
			default:
				break;
			}

			defaultValue = std::move(o.defaultValue);
		}
		void copyUnion(const BasicParamStandard& o)
		{
			switch (typeFhead = o.typeFhead)
			{
			case TypeFhead::STANDARD:
				fheadStd = new FheadStandard(*o.fheadStd);
				break;
			case TypeFhead::BINARY:
				fheadBin = new FheadBinary(*o.fheadBin);
				break;
			default:
				break;
			}

			defaultValue = o.defaultValue;
		}
	};
}