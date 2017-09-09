//MIT License
//Copyright 2017 Patrick Laughrea
#include "theadFun.hpp"

#include "utils.hpp"
#include "webss.hpp"

using namespace std;
using namespace webss;

TheadFun::TheadFun() : ptr(shared_ptr<const Tuple*>(new const Tuple*)) {}

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

void TheadFun::setStructure(Webss webss)
{
	structure = shared_ptr<Webss>(new Webss(move(webss)));
}

const Tuple** TheadFun::getPointerRaw() const { return ptr.get(); }

void TheadFun::setPointer(const Tuple* tuplePtr) const { *ptr = tuplePtr; }