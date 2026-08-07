#pragma once
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/log.h>
#include "removeCache.hpp"
#include "data.hpp"
#include "data-odb.hxx"

namespace suiOptionalaAuthority
{
    class OptionalaAuthority
    {
    public:
        OptionalaAuthority(odb::database& db, sw::redis::Redis& redis, suiRemoveCache::RemoveCache::ptr dataSync);

        //不需要提供添加接口
        


    private:

    private:
        //数据库操作句柄
        odb::database& _db;
        //redis操作句柄
        sw::redis::Redis& _redis;
        //缓存操作客户端
        suiRemoveCache::RemoveCache::ptr _dataSync;
    };
}