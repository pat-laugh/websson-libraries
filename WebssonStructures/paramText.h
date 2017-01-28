#pragma once

#include <memory>
#include <string>

namespace webss
{
#define This BasicParamText
	template <class Webss>
	class This
	{
	public:
		This() {}
		This(Webss&& webss) : defaultValue(new Webss(std::move(webss)))
		{
			assert(defaultValue->isString());
		}

		bool hasDefaultValue() const { return defaultValue.get() != nullptr; }
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
	private:
		std::shared_ptr<Webss> defaultValue;
	};
#undef This
}