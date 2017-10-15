//MIT License
//Copyright 2017 Patrick Laughrea
#include "theadFun.hpp"

#include "utils.hpp"
#include "webss.hpp"
#include "list.hpp"

using namespace std;
using namespace webss;

TheadFun::TheadFun() : ptr(shared_ptr<const Tuple*>(new const Tuple*)) {}
TheadFun::TheadFun(const TheadFun& o, int foreachIndex) : thead(o.thead), structure(o.structure),
		ptr(o.ptr), isForeachList(true), foreachIndex(foreachIndex) {}

bool TheadFun::operator==(const TheadFun& o) const
{
	if (thead != o.thead)
		return false;
	return true;
	//something makes it so the two structures faile to be properly compared together
	//causes read access violation using Visual Studio
	//return equalPtrs(structure, o.structure);
}
bool TheadFun::operator!=(const TheadFun& o) const { return !(*this == o); }

const TheadStd& TheadFun::getThead() const { assert(thead != nullptr); return *thead; }
void TheadFun::setThead(TheadStd thead) { this->thead = shared_ptr<TheadStd>(new TheadStd(move(thead))); }

const Webss& TheadFun::getStructure() const
{
	assert(structure != nullptr);
	if (!isForeachList)
		return *structure;
	return structure->getList()[foreachIndex];
}

void TheadFun::setStructure(Webss webss) { structure = shared_ptr<Webss>(new Webss(move(webss))); }

const Tuple** TheadFun::getPointerRaw() const { return ptr.get(); }

void TheadFun::setPointer(const Tuple* tuplePtr) const { *ptr = tuplePtr; }