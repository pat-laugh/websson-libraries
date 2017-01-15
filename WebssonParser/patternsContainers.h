//MIT License
//Copyright(c) 2016 Patrick Laughrea

#define CaseKeyValue [&](std::string&& key, webss::Webss&& value)
#define CaseKeyOnly [&](std::string&& key)
#define CaseValueOnly [&](webss::Webss&& value)
#define CaseAbstractEntity [&](const webss::Entity& abstractEntity)

#define ErrorKeyValue(Message) [](std::string&& key, webss::Webss&& value) { throw std::runtime_error(Message); }
#define ErrorKeyOnly(Message) [](std::string&& key) { throw std::runtime_error(Message); }
#define ErrorValueOnly(Message) [](webss::Webss&& value) { throw std::runtime_error(Message); }
#define ErrorAbstractEntity(Message) [](const webss::Entity& abstractEntity) { throw std::runtime_error(Message); }