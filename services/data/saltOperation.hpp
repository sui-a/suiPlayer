#pragma once
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/log.h>
#include <odb/query.hxx>
#include <odb/database.hxx>
#include <odb/result.hxx>
#include "data.hpp"
#include "data-odb.hxx"
#include "removeCache.hpp"

namespace suiSalt
{
    class suiSaltOperation
    {
    public:
        suiSaltOperation(odb::database& db, suiRemoveCache::RemoveCache::ptr dataSync);

        //插入新数据
        void insert(suiDataSql::suiSaltMeta& salt);
        void insert(const std::string& userId, const std::string& salt);

        //更新
        void update(suiDataSql::suiSaltMeta& salt);

        //索引
        suiDataSql::suiSaltMeta::ptr selectByUserId(const std::string& userId);

        //删除
        void removeByUserId(const std::string& userId);
    
    private:
        //数据库操作
        void insertToDb(suiDataSql::suiSaltMeta& salt);
        void updateToDb(suiDataSql::suiSaltMeta& salt);
        suiDataSql::suiSaltMeta::ptr selectByUserIdToDb(const std::string& userId);
        void removeByUserIdToDb(const std::string& userId);

    private:
        odb::database& _db;
        suiRemoveCache::RemoveCache::ptr _dataSync;
    };




}


