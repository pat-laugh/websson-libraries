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
		using Data = typename base::Data;
	public:
		BasicList() : base() {}
		BasicList(Data&& data) : base(std::move(data)) {}
	};
}