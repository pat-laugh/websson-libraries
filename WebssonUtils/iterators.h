//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <istream>
#include <sstream>
#include <string>

#define webss_GET_LINE

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
#ifdef webss_GET_LINE
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
		This& incTwo()
		{
#ifdef webss_GET_LINE
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

#ifdef webss_GET_LINE
		int getLine() { return line; }
		int getCharCount() { return charCount; }
#endif
	private:
		std::stringstream in;
		char c1, c2;
		bool isValid = true, hasPeek = true;
#ifdef webss_GET_LINE
		int line = 1, charCount = 1;

		void addLine()
		{
			++line;
			charCount = 1;
		}

		void checkChar(char c)
		{
			if (c == '\n')
				addLine();
			else
				++charCount;
		}
#endif

		void readStart()
		{
			c1 = in.get();
			if (in.good())
				getPeek();
			else
			{
				isValid = false;
				hasPeek = false;
				if (!in.eof())
					throw std::ios_base::failure("io error");
			}
		}

		void getPeek()
		{
			c2 = in.get();
			if (!in.good())
			{
				hasPeek = false;
				if (!in.eof())
					throw std::ios_base::failure("io error");
			}
		}
	};
#undef This
}