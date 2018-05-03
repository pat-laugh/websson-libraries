//MIT License
//Copyright 2017-2018 Patrick Laughrea
#pragma once

#include <memory>

#include "base.hpp"
#include "paramStandard.hpp"

namespace webss
{
	class TheadFunPointer
	{
	public:
		TheadFunPointer(const Tuple** ptr, const Tuple* newVal);
		~TheadFunPointer();
		
	private:
		const Tuple** ptr;
		const Tuple* oldVal;
	};
	
	class TheadFun
	{
	private:
		std::shared_ptr<TheadStd> thead;
		std::shared_ptr<Webss> structure;
		std::shared_ptr<const Tuple*> ptr;
		
		bool isForeachList = false;
		int foreachIndex;
	public:
		TheadFun(TheadStd thead);
		TheadFun(const TheadFun& o, int foreachIndex);

		bool operator==(const TheadFun& o) const;
		bool operator!=(const TheadFun& o) const;
		
		const TheadStd& getThead() const;
		
		const Webss& getStructure() const;
		void setStructure(Webss webss);
		
		const Tuple** getPointerRaw() const;
		TheadFunPointer setPointer(const Tuple* tuplePtr) const;
	};
}