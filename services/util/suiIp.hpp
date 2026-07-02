#pragma once
#include <optional>
#include <string>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace suiIp
{
    class suiIper
    {
    public:
        static std::optional<std::string> GetLocalIP();

    };
}