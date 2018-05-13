//MIT License
//Copyright 2017-2018 Patrick Laughrea
#pragma once

#include "sharedMap.hpp"

namespace webss
{
	template <class Param>
	class BasicParams : public BasicSharedMap<Param>
	{
	private:
		using base = BasicSharedMap<Param>;
		
	public:
		using Data = typename base::Data;
		using Keymap = typename base::Keymap;

		BasicParams() : base() {}
		BasicParams(Data&& data) : base(std::move(data)) {}
		BasicParams(std::shared_ptr<Keymap>&& keys) : base(std::move(keys)) {}
		BasicParams(const std::shared_ptr<Keymap>& keys) : base(keys) {}

		bool operator==(const BasicParams& o) const { return base::operator==(o); }
		bool operator!=(const BasicParams& o) const { return !(*this == o); }

		//instead of the keys being shared, this creates an indepedent copy of the keys and the data
		BasicParams makeCompleteCopy() const { return BasicParams(*base::keys, base::data); }

		void merge(const BasicParams& sender)
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
		BasicParams(const Keymap& keymap, const Data& data) : base(keymap, data) {}
	};
}