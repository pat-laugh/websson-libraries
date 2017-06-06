//MIT License
//Copyright 2017 Patrick Laughrea
#include "dictionary.hpp"

#include "utils.hpp"

using namespace std;
using namespace webss;

bool Dictionary::empty() const { return data.empty(); }
Dictionary::size_type Dictionary::size() const { return data.size(); }

void Dictionary::add(std::string key, Webss value) { containerAddUnsafe(data, std::move(key), std::move(value)); }
void Dictionary::addSafe(std::string key, Webss value) { containerAddSafe(data, std::move(key), std::move(value)); }

bool Dictionary::has(const std::string& key) const { return data.find(key) != data.end(); }

bool Dictionary::operator==(const Dictionary& o) const { return data == o.data; }
bool Dictionary::operator!=(const Dictionary& o) const { return !(*this == o); }

Webss& Dictionary::operator[](const std::string& key) { return accessKeyUnsafe<Data, Webss>(data, key); }
const Webss& Dictionary::operator[](const std::string& key) const { return accessKeyUnsafe<Data, Webss>(data, key); }
Webss& Dictionary::at(const std::string& key) { return accessKeySafe<Data, Webss>(data, key); }
const Webss& Dictionary::at(const std::string& key) const { return accessKeySafe<Data, Webss>(data, key); }

Dictionary::iterator Dictionary::begin() { return data.begin(); }
Dictionary::iterator Dictionary::end() { return data.end(); }
Dictionary::const_iterator Dictionary::begin() const { return data.begin(); }
Dictionary::const_iterator Dictionary::end() const { return data.end(); }