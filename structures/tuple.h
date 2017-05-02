//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "sharedMap.h"

namespace webss
{
	template <class Webss>
	class BasicTuple : public BasicSharedMap<Webss>
	{
	private:
		using base = BasicSharedMap<Webss>;
		using Data = typename base::Data;
		using Keymap = typename base::Keymap;
	public:
		BasicTuple() : base() {}
		BasicTuple(Data&& data) : base(std::move(data)) {}
		BasicTuple(const std::shared_ptr<Keymap>& keys) : base(keys) {}
		BasicTuple(const std::shared_ptr<Keymap>& keys, const Data& data) : base(keys, data) {}
	};
}