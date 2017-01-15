#pragma once

#include "stdafx.h"
#include "WebssonParser/parser.h"
#include "WebssonParser/utilsUsers.h"

using namespace webss;
const std::string schemaAddress[] = { "streetAddress", "city", "state", "postalCode" };
const std::string schemaPhone[] = { "type", "number" };
const std::string schemaPerson[] = { "firstName", "lastName", "isAlive", "age", "address", "phoneNumbers", "children", "spouse" };

class Person
{
public:
	using Entity = BasicEntity<Webss>;
	struct Address
	{
	public:


		Address(const Tuple& tuple) :
			streetAddress(tuple[schemaAddress[0]].getString()),
			city(tuple[schemaAddress[1]].getString()),
			state(tuple[schemaAddress[2]].getString()),
			postalCode(tuple[schemaAddress[3]].getString()) {}

		Webss deserialize()
		{
			Tuple tuple;
			tuple.add(schemaAddress[0], streetAddress);
			tuple.add(schemaAddress[1], city);
			tuple.add(schemaAddress[2], state);
			tuple.add(schemaAddress[3], postalCode);
			return tuple;
		}

		static Tuple* getSchema()
		{
			static Tuple* schema = new Tuple(true);
			static bool schemaDefined = false;
			if (!schemaDefined)
			{
				schema->add(schemaAddress[0], WebssType::NONE);
				schema->add(schemaAddress[1], WebssType::NONE);
				schema->add(schemaAddress[2], WebssType::NONE);
				schema->add(schemaAddress[3], WebssType::NONE);
				schemaDefined = true;
			}
			return schema;
		}
	private:
		std::string streetAddress;
		std::string city;
		std::string state;
		std::string postalCode;
	};
	struct PhoneNumber
	{
	public:
		PhoneNumber(const Tuple& tuple) :
			type(tuple[schemaPhone[0]].getString()),
			number(tuple[schemaPhone[1]].getString()) {}

		Webss deserialize()
		{
			Tuple tuple;
			tuple.add(schemaPhone[0], type);
			tuple.add(schemaPhone[1], number);
			return tuple;
		}

		static Tuple* getSchema()
		{
			static Tuple* schema = new Tuple(true);
			static bool schemaDefined = false;
			if (!schemaDefined)
			{
				schema->add(schemaPhone[0], WebssType::NONE);
				schema->add(schemaPhone[1], WebssType::NONE);
				schemaDefined = true;
			}
			return schema;
		}
	private:
		std::string type;
		std::string number;
	};

	Person(const Tuple& tuple) :
		firstName(tuple[schemaPerson[0]].getString()),
		lastName(tuple[schemaPerson[1]].getString()),
		isAlive(tuple[schemaPerson[2]].getBool()),
		age(tuple[schemaPerson[3]].getInt()),
		address(tuple[schemaPerson[4]].getTuple())
	{
		const auto& numbersList = tuple[schemaPerson[5]].getList();
		for (int i = 0; i < numbersList.size(); ++i)
			phoneNumbers.push_back(PhoneNumber(numbersList[i].getTuple()));

		const auto& childrenList = tuple[schemaPerson[6]].getList();
		for (int i = 0; i < childrenList.size(); ++i)
			children.push_back(new Person(childrenList[i].getTuple()));

		spouse = tuple[schemaPerson[7]].isNull() ? nullptr : new Person(tuple[schemaPerson[7]].getTuple());
	}

	Webss deserialize()
	{
		Tuple tuple;
		tuple.add("firstName", firstName);
		tuple.add("lastName", lastName);
		tuple.add("isAlive", isAlive);
		tuple.add("age", age);
		tuple.add("address", address.deserialize());
		{
			List list;
			for (int i = 0; i < phoneNumbers.size(); ++i)
				list.add(phoneNumbers[i].deserialize());
			tuple.add("phoneNumbers", list);
		}
		{
			List list;
			for (int i = 0; i < children.size(); ++i)
				list.add(children[i]->deserialize());
			tuple.add("children", list);
		}
		if (spouse == nullptr)
			tuple.add("spouse", WebssType::PRIMITIVE_NULL);
		else
			tuple.add("spouse", spouse->deserialize());
		return tuple;
	}
	Webss deserialize(bool schema)
	{
		Tuple tuple;
		tuple.add(schemaPerson[0], firstName);
		tuple.add(schemaPerson[1], lastName);
		tuple.add(schemaPerson[2], isAlive);
		tuple.add(schemaPerson[3], age);
		tuple.add(schemaPerson[4], address.deserialize().getTuple());

		{
			List list;
			for (int i = 0; i < phoneNumbers.size(); ++i)
				list.add(phoneNumbers[i].deserialize());
			tuple.add(schemaPerson[5], std::move(list));
		}
		{
			if (children.empty())
				tuple.add(schemaPerson[6], WebssType::DEFAULT);
			else
			{
				List list;
				for (int i = 0; i < children.size(); ++i)
					list.add(children[i]->deserialize());
				tuple.add(schemaPerson[6], list);
			}
		}
		if (spouse == nullptr)
			tuple.add(schemaPerson[7], WebssType::DEFAULT);
		else
			tuple.add(schemaPerson[7], spouse->deserialize());
		return Function(Person::getSchema(), std::move(tuple));
	}

	Webss deserialize(const std::shared_ptr<Entity>& ent)
	{
		Tuple tuple;
		tuple.add(firstName);
		tuple.add(lastName);
		tuple.add(isAlive);
		tuple.add(age);
		tuple.add(address.deserialize().getTuple());

		{
			List list;
			for (int i = 0; i < phoneNumbers.size(); ++i)
				list.add(phoneNumbers[i].deserialize());
			tuple.add(std::move(list));
		}
		{
			if (children.empty())
				tuple.add(WebssType::DEFAULT);
			else
			{
				List list;
				for (int i = 0; i < children.size(); ++i)
					list.add(children[i]->deserialize());
				tuple.add(list);
			}
		}
		if (spouse == nullptr)
			tuple.add(WebssType::DEFAULT);
		else
			tuple.add(spouse->deserialize());
		return Function(ent, std::move(tuple));
	}

	static Tuple* getSchema()
	{
		static Tuple* schema = new Tuple();
		static bool schemaDefined = false;
		if (!schemaDefined)
		{
			schema->add(schemaPerson[0], WebssType::NONE);
			schema->add(schemaPerson[1], WebssType::NONE);
			schema->add(schemaPerson[2], WebssType::NONE);
			schema->add(schemaPerson[3], WebssType::NONE);
			schema->add(schemaPerson[4], FunctionHead(Address::getSchema()));
			schema->add(schemaPerson[5], FunctionHead(PhoneNumber::getSchema()));
			schema->add(schemaPerson[6], List());
			schema->add(schemaPerson[7], WebssType::PRIMITIVE_NULL);
			schemaDefined = true;
		}
		return schema;
	}
private:
	std::string firstName;
	std::string lastName;
	bool isAlive;
	int age;
	Address address;
	std::vector<PhoneNumber> phoneNumbers;
	std::vector<Person*> children;
	Person* spouse;
};