#include "role_permission.hpp"

namespace suiRolePermission
{
    rolePermission::rolePermission(odb::database& db, sw::redis::Redis& redis, suiRemoveCache::RemoveCache::ptr dataSync)   
        : _db(db), _redis(redis), _dataSync(dataSync)
    {

    }

    //增
    void rolePermission::insert(suiDataSql::suiRoleOperationMeta& rolePermission)
    {
        //向数据库插入
        insertToDb(rolePermission);
    }
    void rolePermission::insert(suiDataSql::roleType roleId, const std::string& operationUrl)
    {
        //向数据库插入
        suiDataSql::suiRoleOperationMeta curMeta;
        curMeta.setRoleType(roleId);
        curMeta.setOperationUrl(operationUrl);
        insertToDb(curMeta);
    }

    //删
    void rolePermission::remove(const std::string& operationUrl)
    {
        //删除数据库
        removeToDb(operationUrl);
        //删除缓存
        removeToRedis(operationUrl);
    }

    void rolePermission::remove(suiDataSql::suiRoleOperationMeta& rolePermission)
    {
        //删除数据库
        removeToDb(rolePermission.getOperationUrl());
        //删除缓存
        removeToRedis(rolePermission.getOperationUrl());
    }

    //查
    //通过角色权限查询
    std::vector<suiDataSql::suiRoleOperationMeta> rolePermission::getPermissionsByRole(suiDataSql::roleType roleId)
    {
        //从数据库中查找
        return getPermissionsToDb(roleId);
    }

    //通过操作id查询
    suiDataSql::suiRoleOperationMeta::ptr rolePermission::getPermissionsByOperation(const std::string& operationUrl)
    {
        //从缓存中查找
        auto ret = getPermissionsToRedis(operationUrl);
        if(ret)
        {
            INFO("redis中查询到存在");
            return ret;
        }
        //从数据库中查找
        INFO("redis中查询到不存在");
        ret = getPermissionsToDb(operationUrl);
        if(ret)
            insertToRedis(ret->getRoleType(), ret->getOperationUrl());

        return ret;
    }
    
    //判断操作在这个权限中是否可以执行
    bool rolePermission::checkPermission(suiDataSql::roleType roleId, const std::string& operationUrl)
    {
        //从缓存中查找
        auto ret = getPermissionsByOperation(operationUrl);
        if(ret)
        {
            return suiDataSql::suiRoleType::getWeight(roleId) >= suiDataSql::suiRoleType::getWeight(ret->getRoleType());
        }
        return false;
    }

    //从数据库增
    void rolePermission::insertToDb(suiDataSql::suiRoleOperationMeta& rolePermission)
    {
        //不使用try，异常由外部处理
        _db.persist(rolePermission);
    }
    //从数据库删
    void rolePermission::removeToDb(const std::string& operationUrl)
    {
        //删除数据库
        _db.erase_query<suiDataSql::suiRoleOperationMeta>(odb::query<suiDataSql::suiRoleOperationMeta>::operation_url == operationUrl);
    }

    //从数据库查
    std::vector<suiDataSql::suiRoleOperationMeta> rolePermission::getPermissionsToDb(suiDataSql::roleType roleId)
    {
        odb::result<suiDataSql::suiRoleOperationMeta> res(_db.query<suiDataSql::suiRoleOperationMeta>(odb::query<suiDataSql::suiRoleOperationMeta>::role_type == roleId));
        std::vector<suiDataSql::suiRoleOperationMeta> ret;
        for(auto it = res.begin(); it != res.end(); it++)
            ret.push_back(*it);
        return ret;
    }
    suiDataSql::suiRoleOperationMeta::ptr rolePermission::getPermissionsToDb(const std::string& operationUrl)
    {
        suiDataSql::suiRoleOperationMeta::ptr out(_db.query_one<suiDataSql::suiRoleOperationMeta>(odb::query<suiDataSql::suiRoleOperationMeta>::operation_url == operationUrl));
        return out;
    }

    //对缓存
    void rolePermission::insertToRedis(suiDataSql::roleType roleId, const std::string& operationUrl)
    {
        //向缓存插入
        auto key = getCacheKey(operationUrl);
        _redis.set(key, suiDataSql::suiRoleType::roleTypeToString(roleId));
        //设置超时时间
        _redis.expire(key, std::chrono::seconds(getCacheExpire()));
    }
    void rolePermission::removeToRedis(const std::string& operationUrl)
    {
        //删除缓存
        publishDeleteMessage({getCacheKey(operationUrl)});
    }
    suiDataSql::suiRoleOperationMeta::ptr rolePermission::getPermissionsToRedis(const std::string& operationUrl)
    {
        //根据key查询
        auto key = getCacheKey(operationUrl);
        auto ret = _redis.get(key);
        if(ret)
        {
            auto curMeta = std::make_shared<suiDataSql::suiRoleOperationMeta>();
            curMeta->setRoleType(suiDataSql::suiRoleType::stringToRoleType(ret.value()));
            curMeta->setOperationUrl(operationUrl);
            return curMeta;
        }
        return nullptr;
    }

    void rolePermission::publishDeleteMessage(const std::vector<std::string>& curCacheKey)
    {
        //发布缓存删除消息队列
        if(curCacheKey.empty())
            return;
        //发布消息
        _dataSync->syncCache(curCacheKey);
    }

    int rolePermission::getCacheExpire()
    {
        thread_local std::mt19937 gen(std::random_device{}());
        // 均匀分布
        std::uniform_int_distribution<int> distrib(_cache_expire_min, _cache_expire_max);
        return distrib(gen);
    }

    std::string rolePermission::getCacheKey(const std::string& operationUrl)
    {
        return  _cacheKeyPrefix + operationUrl;
    }
}