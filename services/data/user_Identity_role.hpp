#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/log.h>
#include "removeCache.hpp"
#include "data.hpp"
#include "data-odb.hxx"

namespace suiUserIdentityRole
{
    class UserIdentityRole
    {
    public:
        using ptr = std::shared_ptr<UserIdentityRole>;

        UserIdentityRole(odb::database& db, sw::redis::Redis& redis, const suiRemoveCache::RemoveCache::ptr dataSync);
        
        //增
        void insert(const std::string& user_id, suiDataSql::roleType role_type, suiDataSql::identityType identity_type);

        //删
        void remove(const std::string& user_id);

        //查
        suiDataSql::suiUserIdIdentityRoleMeta::ptr select(const std::string& user_id);

        //改  不需要更新接口， 由于一个用户可能存在多个身份，不能直接根据某个字段来判断更新什么，所以更新操作用删增完成
        //bool update(const std::string& user_id, suiDataSql::roleType role_type, suiDataSql::identityType identity_type);
        //bool update(suiDataSql::suiUserIdIdentityRoleMeta::ptr updateData);

        //判断
        //判断是否拥有某个身份
        bool hasIdentity(const std::string& user_id, suiDataSql::identityType identity_type);
        //判断是否拥有某个角色
        bool hasRole(const std::string& user_id, suiDataSql::roleType role_type);

    private:
        //数据库操作
        void insertToDb(const std::string& user_id, suiDataSql::roleType role_type, suiDataSql::identityType identity_type);
        void removeToDb(const std::string& user_id);
        //查询数据库
        suiDataSql::suiUserIdIdentityRoleMeta::ptr selectToDb(const std::string& user_id);

        //缓存操作
        void insertToRedis(const std::string& user_id, suiDataSql::roleType role_type, suiDataSql::identityType identity_type);
        void removeToRedis(const std::string& user_id);
        suiDataSql::suiUserIdIdentityRoleMeta::ptr selectToRedis(const std::string& user_id);

        //通过用户id获取缓存key
        std::string getCacheKey(const std::string& user_id);

        //获取随机缓存过期时间
        int getCacheExpire();

        //发布缓存删除消息
        void publishDeleteMessage(const std::vector<std::string>& curCacheKey);

    public:
        //数据库操作句柄
        odb::database& _db;
        //redis操作句柄
        sw::redis::Redis& _redis;
        //缓存操作客户端
        suiRemoveCache::RemoveCache::ptr _dataSync;


        //缓存key前缀
        static const std::string _cacheKeyPrefix;
        //用户id的key
        static const std::string _userIdKey;
        //身份key
        static const std::string _identityKey;
        //角色key
        static const std::string _roleKey;
        //缓存过期时间范围
        static const int _cache_expire_min;
        static const int _cache_expire_max;
    };





}
