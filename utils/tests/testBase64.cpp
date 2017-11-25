//MIT License
//Copyright 2017 Patrick Laughrea
#include "utils/base64.hpp"
#include "catch.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "various/stringBuilder.hpp"

using namespace std;
using namespace various;
using namespace webss;

//this has to be done to avoid a compiler error about putting chars >= 128 in a string
string makeString(vector<int> s)
{
	string out;
	for (decltype(s.size()) i = 0; i < s.size(); ++i)
		out += (char)s[i];
	return out;
}

string addSplit(string split, string a)
{
	*reinterpret_cast<int32_t*>(const_cast<char*>(split.data())) += *reinterpret_cast<int32_t*>(const_cast<char*>(a.data()));
	return split;
}

string readSmartIterator(SmartIterator it)
{
	StringBuilder sb;
	for (; it; ++it)
		sb += *it;
	return sb;
}

static string stringToBits(const string& s)
{
	string bits;
	for (char c : s)
		for (unsigned char mask = 0x80; mask > 0; mask >>= 1)
			bits += (c & mask) == 0 ? '0' : '1';
	return bits;
}

static bool checkBits(const string& s1, const string& s2, int numBits)
{
	auto bits1 = stringToBits(s1), bits2 = stringToBits(s2);
	for (int i = 0; i < numBits; ++i)
		if (bits1[i] != bits2[i])
			return false;
	return true;
}

TEST_CASE("Encode 1 char", "[base64-char1]")
{
	//0xe1 => 11100001 -> 00111000 0001xxxx => 56 16 -> "4Q"
	SmartIterator it(makeString({0xe1}));
	auto out = encodeBase64(it);
	REQUIRE(out.length() == 2);
	REQUIRE(checkBits(out, "4Q", 12));
}

TEST_CASE("Encode 2 chars", "[base64-chars2]")
{
	//0xe1 0x1e => 11100001 00011110 -> 00111000 00010001 001110xx => 56 17 56 -> "4R4"
	SmartIterator it(makeString({0xe1, 0x1e}));
	auto out = encodeBase64(it);
	REQUIRE(out.length() == 3);
	REQUIRE(checkBits(out, "4R4", 22));
}

TEST_CASE("Encode 3 chars", "[base64-chars3]")
{
	//0xe1 0x1e 0xb4 => 11100001 00011110 10110100 -> 00111000 00010001 00111010 00110100 => 56 17 58 52 -> "4R60"
	SmartIterator it(makeString({0xe1, 0x1e, 0xb4}));
	auto out = encodeBase64(it);
	REQUIRE(out.length() == 4);
	REQUIRE(checkBits(out, "4R60", 32));
}


TEST_CASE("Decode 1 char", "[base64-char1]")
{
	SmartIterator it(makeString({'m', ')'}));
	REQUIRE_THROWS_AS(decodeBase64(it), runtime_error);
}

TEST_CASE("Decode 2 chars", "[base64-chars2]")
{
	//"mQ" -> 38 16 => 00100110 00010000 -> 10011001 => "\x99"
	SmartIterator it(makeString({'m', 'Q', ')'}));
	auto out = decodeBase64(it);
	REQUIRE(out.length() == 1);
	REQUIRE(checkBits(out, "\x99", 8));
}

TEST_CASE("Decode 3 chars", "[base64-chars3]")
{
	//"mQ8" -> 38 16 60 => 00100110 00010000 00111100 -> 10011001 00001111 => "\x99\x0f"
	SmartIterator it(makeString({'m', 'Q', '8', ')'}));
	auto out = decodeBase64(it);
	REQUIRE(out.length() == 2);
	REQUIRE(checkBits(out, "\x99\x0f", 16));
}

TEST_CASE("Decode 4 chars", "[base64-chars4]")
{
	//"mQ8/" -> 38 16 60 63 => 00100110 00010000 00111100 00111111 -> 10011001 00001111 00111111 => "\x99\x0f\x3f"
	SmartIterator it(makeString({'m', 'Q', '8', '/', ')'}));
	auto out = decodeBase64(it);
	REQUIRE(out.length() == 3);
	REQUIRE(checkBits(out, "\x99\x0f\x3f", 16));
}