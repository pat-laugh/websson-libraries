//MIT License
//Copyright 2017 Patrick Laughrea
#include "webss.hpp"

using namespace std;
using namespace webss;

Enum::Enum(std::string name) : nspace(std::move(name)) {}
Enum::Enum(std::string name, const Namespace& previousNspace) : nspace(std::move(name), previousNspace) {}

bool Enum::empty() const { return nspace.empty(); }
Enum::size_type Enum::size() const { return nspace.size(); }

void Enum::add(std::string key) { nspace.add(Entity(std::move(key), Webss(size()))); }
void Enum::addSafe(std::string key) { nspace.addSafe(Entity(std::move(key), Webss(size()))); }

bool Enum::has(const std::string& key) const { return nspace.has(key); }

bool Enum::operator==(const Enum& o) const { return nspace == o.nspace; }
bool Enum::operator!=(const Enum& o) const { return !(*this == o); }

Entity& Enum::operator[](const std::string& key) { return nspace[key]; }
const Entity& Enum::operator[](const std::string& key) const { return nspace[key]; }
Entity& Enum::at(const std::string& key) { return nspace.at(key); }
const Entity& Enum::at(const std::string& key) const { return nspace.at(key); }

const std::string& Enum::getName() const { return nspace.getName(); }
const Enum::Namespaces& Enum::getNamespaces() const { return nspace.getNamespaces(); }
const Enum::PtrThis& Enum::getPointer() const { return nspace.getPointer(); }

Enum::iterator Enum::begin() { return nspace.begin(); }
Enum::iterator Enum::end() { return nspace.end(); }
Enum::const_iterator Enum::begin() const { return nspace.begin(); }
Enum::const_iterator Enum::end() const { return nspace.end(); }