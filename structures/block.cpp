//MIT License
//Copyright 2017 Patrick Laughrea
#include "block.hpp"

using namespace std;
using namespace webss;

BlockHead::BlockHead() {}
BlockHead::BlockHead(const Entity& ent) : hasEnt(true), ent(ent) {}

bool BlockHead::hasEntity() const { return hasEnt; }
const Entity& BlockHead::getEntity() const { return ent; }

Block::Block(Webss&& value) : Head(), value(std::move(value)) {}
Block::Block(const Webss& value) : Head(), value(value) {}
Block::Block(const Entity& ent, Webss&& value) : Head(ent), value(std::move(value)) {}
Block::Block(const Entity& ent, const Webss& value) : Head(ent), value(value) {}
Block::Block(Head&& head, Webss&& value) : Head(std::move(head)), value(std::move(value)) {}
Block::Block(const Head& head, const Webss& value) : Head(head), value(value) {}

const Webss& Block::getValue() const { return value; }