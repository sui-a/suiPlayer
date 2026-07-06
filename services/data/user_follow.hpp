#pragma once
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/log.h>
#include "removeCache.hpp"
#include "data.hpp"
#include "data-odb.hxx"

namespace suiUserFollow
{
    class suiFollow
    {
    public:
        using ptr = std::shared_ptr<suiFollow>;

        suiFollow(odb::database& db);

        //增
        void insert(const std::string& user_id, const std::string& follow_user_id);

        //删
        void remove(const std::string& user_id, const std::string& follow_user_id);

        //查
        bool judgment(const std::string& user_id, const std::string& follow_user_id);

        //查
        suiDataSql::suiUserFollowMeta::ptr selectToDb(const std::string& user_id, const std::string& follow_user_id);
        std::vector<suiDataSql::suiUserFollowMeta> selectToDbByUserId(const std::string& user_id);
        std::vector<suiDataSql::suiUserFollowMeta> selectToDbByFollowUserId(const std::string& follow_user_id);

    private:
        //数据库操作句柄
        odb::database& _db;
    };



}