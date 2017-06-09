//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "sharedMap.hpp"

namespace webss
{
#define This BasicParameters
	template <class Parameter>
	class This : public BasicSharedMap<Parameter>
	{
	private:
		using base = BasicSharedMap<Parameter>;
		using Data = typename base::Data;
		using Keymap = typename base::Keymap;

	public:
		This() : base() {}
		This(Data&& data) : base(std::move(data)) {}
		This(const std::shared_ptr<Keymap>& keys) : base(keys) {}

		bool operator==(const This& o) const { return base::operator==(o); }
		bool operator!=(const This& o) const { return !(*this == o); }

		//instead of the keys being shared, this creates an indepedent copy of the keys and the data
		This makeCompleteCopy() const { return This(*base::keys, base::data); }

		void merge(const This& sender)
		{
			auto orderedKeyValues = sender.getOrderedKeyValues();

			for (const auto& keyValue : orderedKeyValues)
				if (keyValue.first == nullptr)
					base::add(*keyValue.second);
				else
					base::addSafe(*keyValue.first, *keyValue.second);
		}

	private:
		//there's a subtle, but important difference with the default copy constructor: default shares the keymap (through the shared pointer),
		//whereas here the keymap itself is moved or copied
		This(const Keymap& keymap, const Data& data) : base(keymap, data) {}
	};
#undef This
}