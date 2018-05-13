//MIT License
//Copyright 2017-2018 Patrick Laughrea
#include "theadFun.hpp"

#include "list.hpp"
#include "tuple.hpp"
#include "utils.hpp"
#include "webss.hpp"

using namespace std;
using namespace webss;

static Tuple dummyTuple;
static mutex mDummyTuple;

TheadFunPointer::TheadFunPointer(const Tuple** ptr, const Tuple* newVal) : ptr(ptr), oldVal(*ptr) { *ptr = newVal; }
TheadFunPointer::~TheadFunPointer() { *ptr = oldVal; }

static const Tuple** addDummyTuplePtr(std::map<std::thread::id, const Tuple**>& ptrMap)
{
	const Tuple** ptr = new const Tuple*(&dummyTuple);
	ptrMap.insert({this_thread::get_id(), ptr});
	return ptr;
}

TheadFunPointerCont::TheadFunPointerCont(TheadStd::Params::size_type sizeParams)
{
	{
		lock_guard<mutex> lockDummyTuple(mDummyTuple);
		while (sizeParams > dummyTuple.size())
			dummyTuple.add(Webss());
	}
	addDummyTuplePtr(ptrMap);
}

TheadFunPointerCont::~TheadFunPointerCont()
{
	for (const auto& p : ptrMap)
		delete p.second;
}

const Tuple** TheadFunPointerCont::getPointerRaw() const
{
	//no need for lock/mutex since insert does not invalidate iterator
	auto it = ptrMap.find(this_thread::get_id());
	if (it != ptrMap.end())
		return it->second;
	return addDummyTuplePtr(*const_cast<decltype(ptrMap)*>(&ptrMap));
}

//this creates the first instance of TheadFun
TheadFun::TheadFun(TheadStd thead) : thead(shared_ptr<TheadStd>(new TheadStd(move(thead)))),
	ptrCont(new TheadFunPointerCont(this->thead->getParams().size())) {}

TheadFun::TheadFun(const TheadFun& o, int foreachIndex) : thead(o.thead), structure(o.structure),
		ptrCont(o.ptrCont), isForeachList(true), foreachIndex(foreachIndex) {}

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
	TheadFunPointer ptr1 = setPointer(&dummyTuple);
	TheadFunPointer ptr2 = o.setPointer(&dummyTuple);
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

const Tuple** TheadFun::getPointerRaw() const { return ptrCont->getPointerRaw(); }

TheadFunPointer TheadFun::setPointer(const Tuple* tuplePtr) const
{
	return TheadFunPointer(ptrCont->getPointerRaw(), tuplePtr);
}
