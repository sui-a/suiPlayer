#include "getwayServerData.hpp"

namespace suiGetwayServer
{
    getwayServerData::getwayServerData(const suiOdb::odbSetting &ms, const suiRedis::redisSettings &rs)
    {
        //创建操作句柄
        _redis = suiRedis::RedisFactory::create(rs);
        _mysql = suiOdb::dbFactory::create(ms);
    }

    bool getwayServerData::verificationPermission(const std::string& sessionid, const std::string& operation)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userid;
            suiDataSql::roleType role = suiDataSql::roleType::roleTypeUnknown; //默认未知角色
            {
                //获取用户id以及权限
                suiSession::sessionData sessionHandle(dbHandler, reHandler, nullptr); //不允许使用修改等操作
                auto sessionMeta = sessionHandle.selectBySessionId(sessionid);
                if(sessionMeta && !sessionMeta->getUserId().null())
                {
                    userid = sessionMeta->getUserId().get();
                }
            }
            if(!userid.empty())
            {
                //如果不为空，则获取用户实际权限
                suiUserIdentityRole::UserIdentityRole roleHandle(dbHandler, reHandler, nullptr); //不允许使用修改等操作
                auto roleMeta = roleHandle.select(userid);
                if(roleMeta)
                    role = roleMeta->getRoleType();
            }
            {
                //获取操作实际权限
                suiOptionalaAuthority::OptionalaAuthority authorityHandle(dbHandler, reHandler); //不允许使用修改等操作
                auto authorityMeta = authorityHandle.select(operation);
                if(authorityMeta == nullptr)
                {
                    //不存在操作，直接返回无权限
                    return false;
                }
                //比较权限
                if(comparePermission(role, authorityMeta->getRoleType()) == false)
                {
                    //说明权限不够
                    return false;
                }
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        return false;
    }

    bool getwayServerData::comparePermission(suiDataSql::roleType role1, suiDataSql::roleType role2)
    {
        return suiDataSql::suiRoleType::getWeight(role1) >= suiDataSql::suiRoleType::getWeight(role2);
    }



}