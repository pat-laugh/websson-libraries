//MIT License
//Copyright 2017-2018 Patrick Laughrea
#include "theadFun.hpp"

#include "list.hpp"
#include "tuple.hpp"
#include "utils.hpp"
#include "webss.hpp"

using namespace std;
using namespace webss;

TheadFunPointer::TheadFunPointer(const Tuple** ptr, const Tuple* newVal) : ptr(ptr), oldVal(*ptr) { *ptr = newVal; }
TheadFunPointer::~TheadFunPointer() { *ptr = oldVal; }

TheadFun::TheadFun(TheadStd thead) : thead(shared_ptr<TheadStd>(new TheadStd(move(thead)))), ptr(new const Tuple*(reinterpret_cast<const Tuple*>(&this->thead->getParams()))) {}
TheadFun::TheadFun(const TheadFun& o, int foreachIndex) : thead(o.thead), structure(o.structure),
		ptr(o.ptr), isForeachList(true), foreachIndex(foreachIndex) {}

bool TheadFun::operator==(const TheadFun& o) const
{
	if (!equalPtrs(thead, o.thead))
		return false;
	if (isForeachList)
	{
		if (!o.isForeachList || foreachIndex != o.foreachIndex)
			return false;
	}
	else if (o.isForeachList)
		return false;
	
	//compare the structure; we already know the heads are the same
	const Tuple* dummyTuple = reinterpret_cast<const Tuple*>(&this->thead->getParams());
	TheadFunPointer ptr1 = setPointer(dummyTuple);
	TheadFunPointer ptr2 = o.setPointer(dummyTuple);
	return equalPtrs(structure, o.structure);
}
bool TheadFun::operator!=(const TheadFun& o) const { return !(*this == o); }

const TheadStd& TheadFun::getThead() const { assert(thead != nullptr); return *thead; }

const Webss& TheadFun::getStructure() const
{
	assert(structure != nullptr);
	if (!isForeachList)
		return *structure;
	return structure->getList()[foreachIndex];
}

void TheadFun::setStructure(Webss webss) { structure = shared_ptr<Webss>(new Webss(move(webss))); }

const Tuple** TheadFun::getPointerRaw() const { return ptr.get(); }

TheadFunPointer TheadFun::setPointer(const Tuple* tuplePtr) const
{
	return TheadFunPointer(ptr.get(), tuplePtr);
}
