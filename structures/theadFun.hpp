//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <memory>

#include "base.hpp"
#include "paramStandard.hpp"

namespace webss
{
	class TheadFun
	{
	public:
		TheadStd thead;
		std::shared_ptr<Webss> structure;
		std::shared_ptr<const Tuple*> ptr;

		TheadFun();

		bool operator==(const TheadFun& o) const;
		bool operator!=(const TheadFun& o) const;

		void setStructure(Webss webss);
		const Tuple** getPointerRaw() const;
		void setPointer(const Tuple* tuplePtr) const;
	};
}