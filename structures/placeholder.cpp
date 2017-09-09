//MIT License
//Copyright 2017 Patrick Laughrea
#include "placeholder.hpp"

#include "tuple.hpp"

using namespace std;
using namespace webss;

Placeholder::Placeholder(int index, const Tuple** ptr) : index(index), ptr(ptr) {}

bool Placeholder::operator==(const Placeholder& o) const { return index == o.index; }
bool Placeholder::operator!=(const Placeholder& o) const { return !(*this == o); }

const Webss& Placeholder::getValue() const { return (**ptr)[index]; }