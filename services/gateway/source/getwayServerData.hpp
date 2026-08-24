#pragma once
#include <memory>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiodb.hpp>
#include "data.hpp"
#include "data-odb.hxx"
#include "session.hpp"
#include "user_Identity_role.hpp"
#include "optionalaAuthority.hpp"

namespace suiGetwayServer
{
    class getwayServerData
    {
    public:
        using ptr = std::shared_ptr<getwayServerData>;
        getwayServerData(const suiOdb::odbSetting &ms, const suiRedis::redisSettings &rs);

        //验证权限
        bool verificationPermission(const std::string& sessionid, const std::string& operation);
    
    private:
        //权限比较
        bool comparePermission(suiDataSql::roleType role1, suiDataSql::roleType role2);

    private:
        //操作句柄
        std::shared_ptr<odb::database> _mysql;
        std::shared_ptr<sw::redis::Redis> _redis;
    };

}