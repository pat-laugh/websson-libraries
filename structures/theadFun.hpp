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
		std::shared_ptr<const Tuple*> ptr = std::shared_ptr<const Tuple*>(new const Tuple*);
		
		bool operator==(const TheadFun& o) const { return thead == o.thead && structure == o.structure; }
		bool operator!=(const TheadFun& o) const { return !(*this == o); }
		
		void setStructure(Webss webss)
		{
			structure = std::shared_ptr<Webss>(new Webss(std::move(webss)));
		}
		
		const Tuple** getPointerRaw() const { return ptr.get(); }
		
		void setPointer(const Tuple* tuplePtr) const { *ptr = tuplePtr; }
	};
}