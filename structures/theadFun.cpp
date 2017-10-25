//MIT License
//Copyright 2017 Patrick Laughrea
#include "theadFun.hpp"

#include "list.hpp"
#include "tuple.hpp"
#include "utils.hpp"
#include "webss.hpp"

using namespace std;
using namespace webss;

TheadFun::TheadFun() : ptr(shared_ptr<const Tuple*>(new const Tuple*)) {}
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
	if (thead == nullptr) //TODO: it should be checked if this is necessary
		return true; //don't check structure since head isn't set up yet
	
	//make a dummy tuple that both thead functions will make placeholders point to
	Tuple dummy;
	for (decltype(thead->getParams().getData().size()) i = 0; i < thead->getParams().getData().size(); ++i)
		dummy.add(Webss());
	setPointer(&dummy);
	o.setPointer(&dummy);
	return equalPtrs(structure, o.structure);
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
