//MIT License
//Copyright 2017 Patrick Laughrea
#include "tuple.hpp"

using namespace std;
using namespace webss;

Tuple::Tuple() : base() {}
Tuple::Tuple(Data&& data) : base(move(data)) {}
Tuple::Tuple(const shared_ptr<Keymap>& keys) : base(keys) {}
Tuple::Tuple(const shared_ptr<Keymap>& keys, const Data& data) : base(keys, data) {}