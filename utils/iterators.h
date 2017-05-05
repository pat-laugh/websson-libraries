//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <istream>
#include <sstream>
#include <string>

namespace webss
{
	class SmartIterator
	{
	public:
		SmartIterator(SmartIterator&& it);
		SmartIterator(const SmartIterator& it);
		SmartIterator(std::istream&& in);
		SmartIterator(const std::istream& in);
		SmartIterator(std::stringstream&& in);
		SmartIterator(const std::stringstream& in);

		SmartIterator(std::string&& in);
		SmartIterator(const std::string& in);

		SmartIterator& operator=(SmartIterator&& o);
		SmartIterator& operator=(const SmartIterator& o);

		SmartIterator& operator++(); //prefix
		SmartIterator& incTwo();
		char operator*() const;

		bool good() const;
		bool end() const;

		operator bool() const;
		bool operator!() const;

		bool operator==(char c) const;
		bool operator!=(char c) const;

		char peek();
		bool peekGood();
		bool peekEnd();

		int getLine();
		int getCharCount();
	private:
		std::stringstream in;
		char c1, c2;
		bool isValid = true, hasPeek = true;
		int line = 1, charCount = 1;

		void addLine();

		void checkChar(char c);

		void readStart();

		void getPeek();
	};
}
