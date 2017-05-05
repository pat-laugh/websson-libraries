//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "list.h"

#include "utils.h"

using namespace std;
using namespace webss;

List::List() {}
List::List(Data&& data) : data(std::move(data)) {}
List::List(const Data& data) : data(data) {}
List::~List() {}

bool List::empty() const { return data.empty(); }
List::size_type List::size() const { return data.size(); }

void List::add(Webss&& value) { data.push_back(std::move(value)); }
void List::add(const Webss& value) { data.push_back(value); }

Webss& List::operator[](size_type index) { return accessIndexUnsafe<Data, Webss>(data, index); }
const Webss& List::operator[](size_type index) const { return accessIndexUnsafe<Data, Webss>(data, index); }
Webss& List::at(size_type index) { return accessIndexSafe<Data, Webss>(data, index); }
const Webss& List::at(size_type index) const { return accessIndexSafe<Data, Webss>(data, index); }

List::iterator List::begin() { return data.begin(); }
List::iterator List::end() { return data.end(); }
List::const_iterator List::begin() const { return data.begin(); }
List::const_iterator List::end() const { return data.end(); }
List::reverse_iterator List::rbegin() { return data.rbegin(); }
List::reverse_iterator List::rend() { return data.rend(); }
List::const_reverse_iterator List::rbegin() const { return data.rbegin(); }
List::const_reverse_iterator List::rend() const { return data.rend(); }