//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "sharedMap.h"

namespace webss
{
	template <class Webss>
	class BasicTuple : public BasicSharedMap<Webss>
	{
	private:
		using base = BasicSharedMap<Webss>;
		bool containerText;
	public:
		BasicTuple(bool containerText = false) : base(), containerText(containerText) {}
		BasicTuple(Data&& data, bool containerText = false) : base(std::move(data)), containerText(containerText) {}
		BasicTuple(const std::shared_ptr<Keymap>& keys, bool containerText = false) : base(keys), containerText(containerText) {}

		bool isText() const { return containerText; }
	};
}