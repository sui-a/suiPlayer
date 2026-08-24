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
        OptionalaAuthority(odb::database& db, sw::redis::Redis& redis);

        //只提供查询接口
        suiDataSql::suiRoleOperationMeta::ptr select(const std::string& operation_url);
    private:
        //数据库查询
        suiDataSql::suiRoleOperationMeta::ptr selectFromDb(const std::string& operation_url);
        //添加进缓存 
        void addToCache(suiDataSql::suiRoleOperationMeta::ptr operationMeta);
        //缓存查询
        suiDataSql::suiRoleOperationMeta::ptr selectFromCache(const std::string& operation_url);

        std::string getCacheKey(const std::string& operation_url);

    private:
        //数据库操作句柄
        odb::database& _db;
        //redis操作句柄
        sw::redis::Redis& _redis;

        //缓存键前缀
        static const std::string _cacheKeyPrefix;
        //权限字段
        static const std::string _cacheKeyField;
        //默认缓存过期时间
        static const int _defaultCacheExpire;
    };
}