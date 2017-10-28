//MIT License
//Copyright 2017 Patrick Laughrea
#include "utils/base64.hpp"
#include "catch.hpp"

#include <cstdint>
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

//Decimal conversions:
//0-9 -> '0'-'9'
//10-35 -> 'A'-'Z'
//36-61 -> 'a'-'z'
//62 -> '-'
//63 -> '='

#define ALL_ZEROES '_'
#define ALL_ONES '+'
#define AN '0'
#define AU 'A' - 10
#define AL 'a' - 36

TEST_CASE("Encode 1 char not 0 nor -1", "[base64-char1-not0or1]")
{
	//11100001 -> 00100001 00000000 00000000 00110000 -> 33 00 00 48
	SmartIterator it(makeString({0xe1})); 
	REQUIRE(encodeBase64(it) == addSplit({33, 0, 0, 48}, {AU, AN, AN, AL}));
}

TEST_CASE("Encode 2 chars not 0 nor -1", "[base64-chars2-not0or1]")
{
	//11100001 00011110 -> 00100001 00011110 00000000 00110000 -> 33 30 00 48
	SmartIterator it(makeString({0xe1, 0x1e})); 
	REQUIRE(encodeBase64(it) == addSplit({33, 30, 0, 48}, {AU, AU, AN, AL}));
}

TEST_CASE("Encode 3 chars not 0 nor -1", "[base64-chars3-not0or1]")
{
	//11100001 00011110 10110100 -> 00100001 00011110 00110100 00110010 -> 33 30 52 50
	SmartIterator it(makeString({0xe1, 0x1e, 0xb4})); 
	REQUIRE(encodeBase64(it) == addSplit({33, 30, 52, 50}, {AU, AU, AL, AL}));
}

TEST_CASE("Encode 1 char 0", "[base64-char1-0]")
{
	//00000000 -> '_'
	SmartIterator it(makeString({0})); 
	REQUIRE(encodeBase64(it) == makeString({ALL_ZEROES}));
}

TEST_CASE("Encode 2 chars 0", "[base64-chars2-0]")
{
	//00000000 00000000 -> '_'
	SmartIterator it(makeString({0, 0})); 
	REQUIRE(encodeBase64(it) == makeString({ALL_ZEROES}));
}

TEST_CASE("Encode 3 chars 0", "[base64-chars3-0]")
{
	//00000000 00000000 00000000 -> '_'
	SmartIterator it(makeString({0, 0, 0})); 
	REQUIRE(encodeBase64(it) == makeString({ALL_ZEROES}));
}

TEST_CASE("Encode 1 char -1", "[base64-char1-1]")
{
	//11111111 -> '+'
	SmartIterator it(makeString({-1})); 
	REQUIRE(encodeBase64(it) == makeString({ALL_ONES}));
}

TEST_CASE("Encode 2 chars -1", "[base64-chars2-1]")
{
	//11111111 11111111 -> '+'
	SmartIterator it(makeString({-1, -1})); 
	REQUIRE(encodeBase64(it) == makeString({ALL_ONES}));
}

TEST_CASE("Encode 3 chars -1", "[base64-chars3-1]")
{
	//11111111 11111111 11111111 -> '+'
	SmartIterator it(makeString({-1, -1, -1})); 
	REQUIRE(encodeBase64(it) == makeString({ALL_ONES}));
}


TEST_CASE("Decode 1 char not 0 nor -1", "[base64-char1-not0or1]")
{
	SmartIterator it(makeString({'m', ')'}));
	REQUIRE_THROWS_AS(decodeBase64(it), runtime_error);
}

TEST_CASE("Decode 2 chars not 0 nor -1", "[base64-chars2-not0or1]")
{
	SmartIterator it(makeString({'m', 'Q', ')'}));
	REQUIRE_THROWS_AS(decodeBase64(it), runtime_error);
}

TEST_CASE("Decode 3 chars not 0 nor -1", "[base64-chars3-not0or1]")
{
	SmartIterator it(makeString({'m', 'Q', '8', ')'}));
	REQUIRE_THROWS_AS(decodeBase64(it), runtime_error);
}

TEST_CASE("Decode 1 char 0", "[base64-char1-0]")
{
	SmartIterator it(makeString({ALL_ZEROES, ')'})); 
	REQUIRE(decodeBase64(it) == makeString({0, 0, 0}));
}

TEST_CASE("Decode 1 char -1", "[base64-char1-1]")
{
	SmartIterator it(makeString({ALL_ONES, ')'})); 
	REQUIRE(decodeBase64(it) == makeString({-1, -1, -1}));
}

TEST_CASE("Decode 4 chars not 0 nor -1", "[base64-chars3-not0or1]")
{
	SmartIterator it(makeString({'m', 'Q', '8', '=', ')'}));
	//109 81 56 61 -> (- 'a' + 36) (- 'A' + 10) (- '0') (63)
	//48 26 8 63 -> 00110000 00011010 00001000 00111111
	//11110000 11011010 11001000 -> 240 218 200
	REQUIRE(decodeBase64(it) == makeString({240, 218, 200}));
}