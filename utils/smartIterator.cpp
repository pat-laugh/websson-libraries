//MIT License
//Copyright 2017 Patrick Laughrea
#include "smartIterator.hpp"

#include <cassert>

using namespace std;
using namespace webss;

SmartIterator::SmartIterator(SmartIterator&& it) : in(move(it.in)), c1(it.c1), c2(it.c2), isValid(it.isValid), hasPeek(it.hasPeek), line(it.line), charCount(it.charCount) {}
SmartIterator::SmartIterator(stringstream&& in) : in(move(in)) { readStart(); }
SmartIterator::SmartIterator(string in) : in(move(in)) { readStart(); }

SmartIterator& SmartIterator::operator=(SmartIterator&& o)
{
	in = move(o.in);
	c1 = o.c1;
	c2 = o.c2;
	isValid = o.isValid;
	hasPeek = o.hasPeek;
	line = o.line;
	charCount = o.charCount;
	return *this;
}

SmartIterator& SmartIterator::operator++()
{
	assert(good());
	checkChar(c1);
	if (hasPeek)
	{
		c1 = c2;
		getPeek();
	}
	else
		isValid = false;
	return *this;
}

SmartIterator& SmartIterator::incTwo()
{
	assert(good() && peekGood());
	checkChar(c1);
	checkChar(c2);
	readStart();
	return *this;
}

char SmartIterator::operator*() const { assert(good()); return c1; }

bool SmartIterator::good() const { return isValid; }
bool SmartIterator::end() const { return !good(); }

SmartIterator::operator bool() const { return good(); }
bool SmartIterator::operator!() const { return end(); }

bool SmartIterator::operator==(char c) const { return good() && c == operator*(); }
bool SmartIterator::operator!=(char c) const { return !operator==(c); }

char SmartIterator::peek() const { assert(peekGood()); return c2; }

bool SmartIterator::peekGood() const { return hasPeek; }
bool SmartIterator::peekEnd() const { return !peekGood(); }

int SmartIterator::getLine() const { return line; }
int SmartIterator::getCharCount() const { return charCount; }

void SmartIterator::addLine()
{
	++line;
	charCount = 1;
}

void SmartIterator::checkChar(char c)
{
	if (c == '\n')
		addLine();
	else
		++charCount;
}

void SmartIterator::readStart()
{
	c1 = in.get(); //in.good() refers to last get, so have to get first
	if (in.good())
		getPeek();
	else
	{
		isValid = false;
		hasPeek = false;
		if (!in.eof())
			throw ios_base::failure("io error");
	}
}

void SmartIterator::getPeek()
{
	c2 = in.get();
	if (!in.good())
	{
		hasPeek = false;
		if (!in.eof())
			throw ios_base::failure("io error");
	}
}