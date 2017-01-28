//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "base.h"

namespace webss
{
	template <class T>
	class BasicList : public VectorWrapper<T>
	{
	private:
		using base = VectorWrapper<T>;
		bool containerText;
	public:
		BasicList(bool containerText = false) : base(), containerText(containerText) {}
		BasicList(Data&& data, bool containerText = false) : base(std::move(data)), containerText(containerText) {}

		bool isText() const { return containerText; }
	};
}