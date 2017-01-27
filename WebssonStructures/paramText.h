#pragma once

#include <memory>
#include <string>

namespace webss
{
#define This ParamText
	class This
	{
	public:
		This() {}
		This(std::string&& s) : defaultValue(new std::string(std::move(s))) {}

		bool hasDefaultValue() const { return defaultValue.get() != nullptr; }
		const std::string& getDefaultValue() const { return *defaultValue; }
		const std::shared_ptr<std::string>& getDefaultPointer() const { return defaultValue; }
	private:
		std::shared_ptr<std::string> defaultValue;
	};
#undef This
}