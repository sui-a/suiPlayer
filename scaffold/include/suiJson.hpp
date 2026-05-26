#pragma once
#include <jsoncpp/json/json.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <optional>

namespace suiUtil
{
    class suiJson
    {
    public:
        static std::optional<std::string> serialize(const Json::Value& val);
        static std::optional<Json::Value> unserialize(const std::string& inp);
    private:
    };
}