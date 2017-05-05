//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "sharedMap.h"
#include "webss.h"

namespace webss
{
	class Tuple : public BasicSharedMap<Webss>
	{
	private:
		using base = BasicSharedMap<Webss>;
	public:
		using Data = base::Data;
		using Keymap = base::Keymap;
		Tuple();
		Tuple(Data&& data);
		Tuple(const std::shared_ptr<Keymap>& keys);
		Tuple(const std::shared_ptr<Keymap>& keys, const Data& data);
	};
}