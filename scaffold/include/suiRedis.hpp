#pragma once
#include "log.h"
#include <string>
#include <optional>
#include <memory>
#include <sstream>
#include <sw/redis++/redis++.h>
#include <sw/redis++/queued_redis.h>

namespace suiRedis
{
    struct redisSettings
    {
        std::string _host;
        int _port = 6379;
        std::string _password;
        int _db = 0;
        std::string _user = "default";
        int _connection_pool_size = 3;
    };

    class RedisFactory
    {
    public:
        static std::shared_ptr<sw::redis::Redis> create(const redisSettings& setting);
    };

    
}