#include "suiIp.hpp"

namespace suiIp
{
    std::optional<std::string> suiIper::GetLocalIP()
    {
        struct ifaddrs* ifaddr = nullptr;

        if (getifaddrs(&ifaddr) == -1)
        {
            return std::nullopt;
        }

        for (struct ifaddrs* ifa = ifaddr;
            ifa != nullptr;
            ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == nullptr)
            {
                continue;
            }

            // 只处理 IPv4
            if (ifa->ifa_addr->sa_family != AF_INET)
            {
                continue;
            }

            sockaddr_in* addr =
                reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);

            char ip[INET_ADDRSTRLEN] = {0};

            inet_ntop(
                AF_INET,
                &addr->sin_addr,
                ip,
                sizeof(ip));

            std::string ipStr(ip);

            // 跳过回环地址
            if (ipStr != "127.0.0.1")
            {
                freeifaddrs(ifaddr);
                return ipStr;
            }
        }

        freeifaddrs(ifaddr);

        return std::nullopt;
    }




}