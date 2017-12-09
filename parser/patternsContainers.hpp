//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "errors.hpp"

#define CaseKeyValue [&](std::string&& key, webss::Webss&& value)
#define CaseKeyOnly [&](std::string&& key)
#define CaseValueOnly [&](webss::Webss&& value)
#define CaseAbstractEntity [&](const webss::Entity& abstractEntity)

#define ErrorKeyValue(Message) [](std::string&&, webss::Webss&&) { throw std::runtime_error(WEBSSON_EXCEPTION(Message)); }
#define ErrorKeyOnly(Message) [](std::string&&) { throw std::runtime_error(WEBSSON_EXCEPTION(Message)); }
#define ErrorValueOnly(Message) [](webss::Webss&&) { throw std::runtime_error(WEBSSON_EXCEPTION(Message)); }
#define ErrorAbstractEntity(Message) [](const webss::Entity&) { throw std::runtime_error(WEBSSON_EXCEPTION(Message)); }