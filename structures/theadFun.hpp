//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "base.hpp"

namespace webss
{
	class TheadFun
	{
	public:
		TheadStd thead;
		std::shared_ptr<Webss> structure;
		std::shared_ptr<Tuple*> ptr = std::shared_ptr<Tuple*>(new Tuple*);
		
		bool operator==(const TheadFun& o) const { return thead == o.thead && structure == o.structure; }
		bool operator!=(const TheadFun& o) const { return !(*this == o); }
	};
}