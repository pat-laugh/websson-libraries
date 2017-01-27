//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "sharedMap.h"

namespace webss
{
#define This BasicParameters
	template <class Parameter>
	class This : public BasicSharedMap<Parameter>
	{
	public:
		This() : base() {}
		This(Data&& data) : base(std::move(data)) {}
		This(const std::shared_ptr<Keymap>& keys) : base(keys) {}

		//instead of the keys being shared, this creates an indepedent copy of the keys and the data
		This makeCompleteCopy() const { return This(*keys, data); }

		void merge(const This& sender)
		{
			auto orderedKeyValues = sender.getOrderedKeyValues();

			for (const auto& keyValue : orderedKeyValues)
				if (keyValue.first == nullptr)
					add(*keyValue.second);
				else
					addSafe(*keyValue.first, *keyValue.second);
		}
	private:
		using base = BasicSharedMap<Parameter>;

		//there's a subtle, but important difference with the default copy constructor: default shares the keymap (through the shared pointer),
		//whereas here the keymap itself is moved or copied
		This(const Keymap& keymap, const Data& data) : base(keymap, data) {}
	};
#undef This
}