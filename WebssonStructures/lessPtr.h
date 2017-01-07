//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

namespace webss
{
	template <class T>
	struct less_ptr { bool operator()(T* t1, T* t2) const { return *t1 < *t2; } };
}