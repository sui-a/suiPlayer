#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/log.h>
#include "removeCache.hpp"
#include "data.hpp"
#include "data-odb.hxx"

namespace suiRolePermission
{
    class rolePermission
    {
    public:
        rolePermission(odb::database& db, sw::redis::Redis& redis, suiRemoveCache::RemoveCache::ptr dataSync);

        //增
        void insert(suiDataSql::suiRoleOperationMeta& rolePermission);
        void insert(suiDataSql::roleType roleId, const std::string& operationUrl);
        //删
        void remove(const std::string& operationUrl);
        void remove(suiDataSql::suiRoleOperationMeta& rolePermission);

        //查
        //通过角色权限查询
        std::vector<suiDataSql::suiRoleOperationMeta> getPermissionsByRole(suiDataSql::roleType roleId);
        //通过操作id查询
        suiDataSql::suiRoleOperationMeta::ptr getPermissionsByOperation(const std::string& operationUrl);
        
        //判断操作在这个权限中是否可以执行
        bool checkPermission(suiDataSql::roleType roleId, const std::string& operationUrl);

    private:
        //从数据库增
        void insertToDb(suiDataSql::suiRoleOperationMeta& rolePermission);
        //从数据库删
        void removeToDb(const std::string& operationUrl);

        //从数据库查
        std::vector<suiDataSql::suiRoleOperationMeta> getPermissionsToDb(suiDataSql::roleType roleId);
        suiDataSql::suiRoleOperationMeta::ptr getPermissionsToDb(const std::string& operationUrl);

        //对缓存
        //从缓存增
        void insertToRedis(suiDataSql::roleType roleId, const std::string& operationUrl); //根据roleId和operationUrl插入缓存 key由operationUrl组成 value为roleId字符串
        //从缓存删
        void removeToRedis(const std::string& operationUrl); //删除前缀+operationUrl组成的key
        //
        suiDataSql::suiRoleOperationMeta::ptr getPermissionsToRedis(const std::string& operationUrl); //只查找操作id，对角色权限进行枚举

        //发布缓存删除消息队列
        void publishDeleteMessage(const std::vector<std::string>& curCacheKey);

        //根据roleId和operationUrl获取缓存key
        std::string getCacheKey(const std::string& operationUrl);

        //获取随机超时时间
        int getCacheExpire();
    private:
        //数据库操作句柄
        odb::database& _db;
        //redis操作句柄
        sw::redis::Redis& _redis;
        //缓存操作客户端
        suiRemoveCache::RemoveCache::ptr _dataSync;

        //缓存key前缀
        const std::string _cacheKeyPrefix = "sui_role_permission_";
        const int _cache_expire_min = 3600;
        const int _cache_expire_max = 7200;
    };




}