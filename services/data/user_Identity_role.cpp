#include "user_Identity_role.hpp"

namespace suiUserIdentityRole
{
    const std::string UserIdentityRole::_cacheKeyPrefix = "user_Identity_role_redis_key";
    const std::string UserIdentityRole::_identityKey = "user_identity_key";
    const std::string UserIdentityRole::_roleKey = "user_role_key";
    const std::string UserIdentityRole::_userIdKey = "user_id_key";
    const int UserIdentityRole::_cache_expire_min = 3600;
    const int UserIdentityRole::_cache_expire_max = 7200;


    UserIdentityRole::UserIdentityRole(odb::database& db, sw::redis::Redis& redis, const suiRemoveCache::RemoveCache::ptr dataSync)
        : _db(db), _redis(redis), _dataSync(dataSync)
    {

    }
        
    //增
    void UserIdentityRole::insert(const std::string& user_id, suiDataSql::roleType role_type, suiDataSql::identityType identity_type)
    {
        //直接向数据库插入
        insertToDb(user_id, role_type, identity_type);
    }

    //删
    void UserIdentityRole::remove(const std::string& user_id)
    {
        //删除数据库
        removeToDb(user_id);
        //删除redis
        removeToRedis(user_id);
    }

    //查
    suiDataSql::suiUserIdIdentityRoleMeta::ptr UserIdentityRole::select(const std::string& user_id)
    {
        //从redis中查找
        auto ret = selectToRedis(user_id);
        if(ret)
        {
            INFO("redis中查询到存在");
            return ret;
        }
        INFO("redis中查询到不存在");
        //从数据库中查找
        ret = selectToDb(user_id);
        if(ret)
            insertToRedis(ret->getUserId(), ret->getRoleType(), ret->getIdentityType());
        return ret;
    }

    void UserIdentityRole::update(suiDataSql::suiUserIdIdentityRoleMeta::ptr updateData)
    {
        //先查找
        auto ret = selectToDb(updateData->getUserId());
        if(ret)
        {
            //存在目标
            ret->setUserId(updateData->getUserId());
            ret->setRoleType(updateData->getRoleType());
            ret->setIdentityType(updateData->getIdentityType());
            //更新数据库
            _db.update(*ret);
            //删除缓存
            publishDeleteMessage({getCacheKey(ret->getUserId())});
        }
    }

    //判断
    //判断是否拥有某个身份
    bool UserIdentityRole::hasIdentity(const std::string& user_id, suiDataSql::identityType identity_type)
    {
        auto ret = select(user_id);
        if(ret)
            return ret->getIdentityType() == identity_type;
        return false;
    }
    //判断是否拥有某个角色
    bool UserIdentityRole::hasRole(const std::string& user_id, suiDataSql::roleType role_type)
    {
        //先查找
        auto ret = select(user_id);
        if(ret)
            return ret->getRoleType() == role_type;
        return false;
    }

    bool UserIdentityRole::hasPermission(const std::string& user_id, suiDataSql::identityType identity_type, suiDataSql::roleType role_type)
    {
        //先查找
        auto ret = select(user_id);
        if(suiDataSql::suiRoleType::comparePermission(ret->getRoleType(), role_type) 
        && suiDataSql::suiIdentityType::comparePermission(ret->getIdentityType(), identity_type))
        {
            return true;
        }
        return false;
    }

    //数据库操作
    void UserIdentityRole::insertToDb(const std::string& user_id, suiDataSql::roleType role_type, suiDataSql::identityType identity_type)
    {
        //不使用try，异常由外部处理
        suiDataSql::suiUserIdIdentityRoleMeta curMeta;
        curMeta.setUserId(user_id);
        curMeta.setRoleType(role_type);
        curMeta.setIdentityType(identity_type);
        _db.persist(curMeta);
    }

    void UserIdentityRole::removeToDb(const std::string& user_id)
    {
        //不使用try，异常由外部处理
        _db.erase_query<suiDataSql::suiUserIdIdentityRoleMeta>(odb::query<suiDataSql::suiUserIdIdentityRoleMeta>::user_id == user_id);
    }

    //查询数据库
    suiDataSql::suiUserIdIdentityRoleMeta::ptr UserIdentityRole::selectToDb(const std::string& user_id)
    {
        auto ret = suiDataSql::suiUserIdIdentityRoleMeta::ptr(_db.query_one<suiDataSql::suiUserIdIdentityRoleMeta>(
            odb::query<suiDataSql::suiUserIdIdentityRoleMeta>::user_id == user_id
        ));
        return ret;
    }

    //缓存操作
    void UserIdentityRole::insertToRedis(const std::string& user_id, suiDataSql::roleType role_type, suiDataSql::identityType identity_type)
    {
        //不使用try，异常由外部处理
        std::string key = getCacheKey(user_id);
        std::unordered_map<std::string, std::string> curData;
        curData[_userIdKey] = user_id;
        curData[_identityKey] = suiDataSql::suiIdentityType::identityTypeToString(identity_type);
        curData[_roleKey] = suiDataSql::suiRoleType::roleTypeToString(role_type);
        //进行添加
        _redis.hmset(key, curData.begin(), curData.end());
        _redis.expire(key, std::chrono::seconds(getCacheExpire()));
    }
    void UserIdentityRole::removeToRedis(const std::string& user_id)
    {
        //进行删除
        std::string key = getCacheKey(user_id);
        publishDeleteMessage(std::vector<std::string>({key}));
    }

    suiDataSql::suiUserIdIdentityRoleMeta::ptr UserIdentityRole::selectToRedis(const std::string& user_id)
    {
        //从缓存机制中读取
        std::string key = getCacheKey(user_id);
        std::unordered_map<std::string, std::string> _data;
        _redis.hgetall(key, std::inserter(_data, _data.begin()));
        if(_data.empty())
            return nullptr; //不存在

        //遍历缓存数据
        //都为not_null 所以直接添加
        suiDataSql::suiUserIdIdentityRoleMeta::ptr out = std::make_shared<suiDataSql::suiUserIdIdentityRoleMeta>();
        out->setUserId(_data[_userIdKey]);
        out->setIdentityType(suiDataSql::suiIdentityType::stringToIdentityType(_data[_identityKey]));
        out->setRoleType(suiDataSql::suiRoleType::stringToRoleType(_data[_roleKey]));
        return out;
    }

    //发布缓存删除消息
    void UserIdentityRole::publishDeleteMessage(const std::vector<std::string>& curCacheKey)
    {
        if(curCacheKey.empty())
            return;
        //发布消息
        _dataSync->syncCache(curCacheKey);
    }

    std::string UserIdentityRole::getCacheKey(const std::string& user_id)
    {
        return  _cacheKeyPrefix + user_id;
    }

    int UserIdentityRole::getCacheExpire()
    {
        thread_local std::mt19937 gen(std::random_device{}());
        // 均匀分布
        std::uniform_int_distribution<int> distrib(_cache_expire_min, _cache_expire_max);
        return distrib(gen);
    }

}
