#include "suiRedis.hpp"

namespace suiRedis
{
    std::shared_ptr<sw::redis::Redis> RedisFactory::create(const redisSettings& setting)
    {
        sw::redis::ConnectionOptions conn_opts;
        conn_opts.host = setting._host; // 提示：原图此处未填写完整，结合上下文一般从 settings 获取
        conn_opts.port = setting._port;
        conn_opts.user = setting._user;
        conn_opts.password = setting._password; // 提示：原图此处未填写完整
        conn_opts.db = setting._db; // 默认0号库

        sw::redis::ConnectionPoolOptions pool_opts;
        pool_opts.size = setting._connection_pool_size;

        return std::make_shared<sw::redis::Redis>(conn_opts, pool_opts);
    }

}
