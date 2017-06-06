//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "sharedMap.hpp"
#include "webss.hpp"

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
		Tuple(const std::shared_ptr<Keymap>& keys);
		Tuple(const std::shared_ptr<Keymap>& keys, const Data& data);

		bool operator==(const Tuple& o) const { return base::operator==(o); }
		bool operator!=(const Tuple& o) const { return !(*this == o); }
	};
}