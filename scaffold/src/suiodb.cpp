#include "suiodb.hpp"
#include "log.h"

namespace suiOdb
{
    std::shared_ptr<odb::database> dbFactory::create(const odbSetting& settings)
    {
        //构造连接池
        std::unique_ptr<odb::mysql::connection_factory> pool(new odb::mysql::connection_pool_factory(settings._connection_pool_size));
        //构造数据库操作句柄
        auto handler = std::make_shared<odb::mysql::database>(settings._user, settings._password
                                                            , settings._database, settings._host
                                                            , settings._port, ""
                                                            , settings._charset, 0, std::move(pool));
        return handler;
    }
}
