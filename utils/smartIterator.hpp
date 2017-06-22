//MIT License
//Copyright 2017 Patrick Laughrea
/*
	SmartIterator: wraps a stringstream object, providing a common interface to
	iterate both streams and strings.

	The class throws the exception ios_base::failure in case of a non-eof error.
*/
#pragma once

#include <sstream>
#include <string>

namespace webss
{
	class SmartIterator
	{
	public:
		SmartIterator(SmartIterator&& it);
		SmartIterator(const SmartIterator& it) = delete;
		SmartIterator(std::stringstream&& in);
		SmartIterator(const std::stringstream& in) = delete;
		SmartIterator(std::string in);
		SmartIterator(std::streambuf* sb);

		SmartIterator& operator=(SmartIterator&& o);
		SmartIterator& operator=(const SmartIterator& o) = delete;

		//the iterator must be good
		SmartIterator& operator++();

		//the iterator and peek must be good
		SmartIterator& incTwo();

		//the iterator must be good
		char operator*() const;

		bool good() const;
		bool end() const;

		//returns good()
		operator bool() const;
		bool operator!() const;

		//returns true if the iterator is good and it points to c, else false
		bool operator==(char c) const;
		bool operator!=(char c) const;

		//peek must be good
		char peek() const;

		bool peekGood() const;
		bool peekEnd() const;

		int getLine() const; //the current line (based on '\n'), 1-based index
		int getCharCount() const; //1-based index of currently pointed char on the current line
	private:
		std::stringstream in;
		char c1, c2;
		bool isValid = true, hasPeek = true;
		int line = 1, charCount = 1;

		void addLine(); //increases line and resets charCount to 1
		void checkChar(char c); //if c is '\n', addLine(), else increases charCount
		
		void readStart(); //safely reads the next two chars on the stream
		void getPeek(); //safely reads the next char on the stream and assign peek
	};
}
