//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <istream>
#include <sstream>
#include <string>

namespace webss
{
#define This SmartIterator
	class This
	{
	public:
		This(This&& it) : in(std::move(it.in)), c1(it.c1), c2(it.c2), isValid(it.isValid), hasPeek(it.hasPeek), line(it.line), charCount(it.charCount) {}
		This(const This& it)
		{
			this->in << it.in.rdbuf();
			readStart();
		}
		This(std::istream&& in)
		{
			this->in << in.rdbuf();
			readStart();
		}
		This(const std::istream& in)
		{
			this->in << in.rdbuf();
			readStart();
		}
		This(std::stringstream&& in) : in(std::move(in))
		{
			readStart();
		}
		This(const std::stringstream& in)
		{
			this->in << in.rdbuf();
			readStart();
		}

		This(std::string&& in) : in(std::move(in))
		{
			readStart();
		}
		This(const std::string& in) : in(in)
		{
			readStart();
		}

		This& operator=(This&& o)
		{
			in = std::move(o.in);
			c1 = o.c1;
			c2 = o.c2;
			isValid = o.isValid;
			hasPeek = o.hasPeek;
			line = o.line;
			charCount = o.charCount;
			return *this;
		}
		This& operator=(const This& o)
		{
			if (this != &o)
			{
				in << o.in.rdbuf();
				readStart();
			}
			return *this;
		}

		This& operator++() //prefix
		{
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
		This& incTwo()
		{
			checkChar(c1);
			checkChar(c2);
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

		int getLine() { return line; }
		int getCharCount() { return charCount; }
	private:
		std::stringstream in;
		char c1, c2;
		bool isValid = true, hasPeek = true;
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
