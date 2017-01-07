#pragma once

#include "stdafx.h"
#include "WebssonParser/parser.h"
#include "WebssonParser/utilsUsers.h"

const std::string schemaDate[] = { "year", "month", "day" };

class Date
{
public:
	int year;
	int month;
	int day;

	Date(int year, int month, int day) : year(year), month(month), day(day) {}

	Date(const webss::Tuple& tuple) :
		year(tuple[schemaDate[0]].getInt()),
		month(tuple[schemaDate[1]].getInt()),
		day(tuple[schemaDate[2]].getInt()) {}

	webss::Webss deserialize()
	{
		webss::Function f(Date::getSchema(), getDataTuple());
		f.functionType = webss::FunctionHead::FunctionType::BINARY;

		return f;
	}

	webss::Webss deserializeRegular()
	{
		return webss::Function(Date::getSchemaRegular(), getDataTuple());
	}

	static webss::Tuple* getSchema()
	{
		static webss::Tuple* schema = new webss::Tuple();
		static bool schemaDefined = false;
		if (!schemaDefined)
		{
			schema->add(schemaDate[0], webss::makeBinaryDefault(webss::Keyword::INT4, 2016));
			schema->add(schemaDate[1], webss::makeBinary(webss::Keyword::INT4));
			schema->add(schemaDate[2], webss::makeBinary(webss::Keyword::INT4));

			schemaDefined = true;
		}
		return schema;
	}



private:
	static webss::Tuple* getSchemaRegular()
	{
		static webss::Tuple* schema = new webss::Tuple();
		static bool schemaDefined = false;
		if (!schemaDefined)
		{
			schema->add(schemaDate[0], 2016);
			schema->add(schemaDate[1], webss::WebssType::NONE);
			schema->add(schemaDate[2], webss::WebssType::NONE);
			schemaDefined = true;
		}
		return schema;
	}

	webss::Tuple getDataTuple()
	{
		webss::Tuple tuple;
		if (year == 2016)
			tuple.add(webss::WebssType::DEFAULT);
		else
			tuple.add(year);
		tuple.add(month);
		tuple.add(day);
		return tuple;
	}
};