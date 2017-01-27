//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "sharedMap.h"

namespace webss
{
#define This BasicParameters
	template <class Webss>
	class This : public BasicSharedMap<Webss>
	{
	public:
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
		//there's a subtle, but important difference with the default copy constructor: default shares the keymap (through the shared pointer),
		//whereas here the keymap itself is moved or copied
		This(const Keymap& keymap, const Data& data) : keys(new Keymap(keymap)), data(data) {}
	};
#undef This
}