//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <istream>
#include <sstream>
#include <string>

#define GET_LINE
#ifdef GET_LINE
#define TAB_LENGTH 4
#endif

namespace webss
{
#define This SmartIterator
	class This
	{
	public:
		This(const std::istream& in)
		{
			this->in << in.rdbuf();
			readStart();
		}
		This(const std::stringstream& in)
		{
			this->in << in.rdbuf();
			readStart();
		}
		This(const std::string& in) : in(in)
		{
			readStart();
		}
		~This() {}

		This& operator++() //prefix
		{
#ifdef GET_LINE
			checkChar(c1);
#endif
			if (hasPeek)
			{
				c1 = c2;
				getPeek();
			}
			else
				isValid = false;
			return *this;
		}
		This& readTwo()
		{
#ifdef GET_LINE
			checkChar(c1);
			checkChar(c2);
#endif
			readStart();
			return *this;
		}
		char operator*() const { return c1; }

		bool good() const { return isValid; }
		bool end() const { return !good(); }

		operator bool() const { return good(); }
		bool operator!() const { return end(); }

		bool operator==(char c) const { return good() && c == operator*(); }
		bool operator!=(char c) const { return !operator==(c); }

		char peek() { return c2; }
		bool peekGood() { return hasPeek; }
		bool peekEnd() { return !peekGood(); }

#ifdef GET_LINE
		int getLine() { return line; }
		int getCol() { return col; }
#endif
	private:
		std::stringstream in;
		char c1, c2;
		bool isValid, hasPeek;
#ifdef GET_LINE
		int line = 1, col = 1;

		void addLine()
		{
			++line;
			col = 1;
		}

		void checkChar(char c)
		{
			if (c == '\n')
				addLine();
			else if (c == '\t')
				col += TAB_LENGTH;
			else
				++col;
		}
#endif

		void readStart()
		{
			c1 = in.get();
			isValid = in.rdstate() == 0;
			getPeek();
		}

		void getPeek()
		{
			c2 = in.get();
			hasPeek = in.rdstate() == 0;
		}
	};
#undef This

	using It = SmartIterator;
}

#ifdef GET_LINE
#undef TAB_LENGTH
#undef GET_LINE
#endif