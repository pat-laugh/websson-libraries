//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "base.h"

namespace webss
{
	template <class T>
	class BasicList : public VectorWrapper<T>
	{
	private:
		using base = VectorWrapper<T>;
	public:
		BasicList(bool containerText = false) : base() {}
		BasicList(Data&& data, bool containerText = false) : base(std::move(data)) {}
	};
}