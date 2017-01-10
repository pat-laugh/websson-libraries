//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <istream>

#define GET_LINE

namespace webss
{
#define This SmartIterator
	class This
	{
	public:
		virtual This& operator++() = 0; //prefix; no postfix because of C++ design limitations forcing the usage of CRTP idiom, which forces usage of a template, which is annoying
		virtual This& readTwo() = 0;
		virtual char operator*() const = 0;

		virtual bool good() const { return !end(); }
		virtual bool end() const = 0;

		virtual operator bool() const { return good(); }
		virtual bool operator!() const { return end(); }

		virtual bool operator==(char c) const { return good() && c == operator*(); }
		virtual bool operator!=(char c) const { return !operator==(c); }

		virtual char peek() const = 0;
		virtual bool peekGood() const { return !peekEnd(); }
		virtual bool peekEnd() const = 0;

#ifdef GET_LINE
		int getLine() { return line; }
		int getCharCol() { return col; }
	protected:
		int line = 1;
		int col = 0;

		void addLine()
		{
			++line;
			col = 0;
		}

		void checkChar(char c)
		{
			if (c == '\n')
				addLine();
			else
				++col;
		}
#endif
	};
#undef This

#define This SmartStreamIterator
	class This : public SmartIterator
	{
	public:
		This(std::istream& in) : in(in)
		{
#ifdef GET_LINE
			c2 = 0;
#endif
			readTwo();
		}
		~This() {}

		This& operator++() override
		{
			c1 = c2;
			s1 = s2;
#ifdef GET_LINE
			if (good())
				checkChar(c1);
#endif
			c2 = in.get();
			s2 = in.rdstate();
			return *this;
		} //prefix

		This& readTwo() override
		{
			c1 = in.get();
			s1 = in.rdstate();
#ifdef GET_LINE
			if (peekGood())
				checkChar(c2);
			if (good())
				checkChar(c1);
#endif
			c2 = in.get();
			s2 = in.rdstate();
			return *this;
		}

		char operator*() const override { return c1; }
		bool end() const override { return s1 != 0; }
		char peek() const override { return c2; }
		bool peekEnd() const override { return s2 != 0; }
	private:
		char c1, c2;
		std::istream& in;
		std::ios_base::iostate s1, s2;
	};
#undef This

#define This SmartStringIterator
	class This : public SmartIterator
	{
	public:
		This(const std::string& in) : in(in), it(in.begin())
		{
#ifdef GET_LINE
			if (good())
				checkChar(*it);
#endif
		}
		~This() {}

		This& operator++() override
		{
			++it;
#ifdef GET_LINE
			if (good())
				checkChar(*it);
#endif
			return *this;
		} //prefix

		This& readTwo() override
		{
			operator++();
			operator++();
			return *this;
		}

		char operator*() const override { return *it; }
		bool end() const override { return it == in.end(); }
		char peek() const override { return *(it + 1); }
		bool peekEnd() const override { return (it + 1) == in.end(); }
	private:
		const std::string& in;
		std::string::const_iterator it;
	};
#undef This

	using It = SmartIterator;
}