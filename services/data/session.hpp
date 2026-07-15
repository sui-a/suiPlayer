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

namespace suiSession
{
    class sessionData
    {
    public:
        sessionData(odb::database& db, sw::redis::Redis& redis, suiRemoveCache::RemoveCache::ptr dataSync);
        ~sessionData() = default;
        
        //新增会话
        void insert(suiDataSql::suiSessionMeta& session);
        //更新会话
        void update(suiDataSql::suiSessionMeta& session);
        //删除会话
        void removeBySessionId(std::string sessionId);
        void removeByUserId(std::string userId);
        //根据会话ID查询会话信息
        suiDataSql::suiSessionMeta::ptr selectBySessionId(const std::string& sessionId);
    private:
        //数据库操作
        //新增会话
        void insertToDb(suiDataSql::suiSessionMeta& session);
        //更新会话
        void updateToDb(suiDataSql::suiSessionMeta& session);
        //删除会话
        void removeBysessionIdToDb(std::string sessionId);
        void removeByUserIdToDb(std::string userId);
        //查询会话
        suiDataSql::suiSessionMeta::ptr selectToDb(const std::string& sessionId);

        //缓存操作
        //向redis缓存会话
        void insertToRedis(suiDataSql::suiSessionMeta& session);
        //更新redis会话
        void updateToRedis(suiDataSql::suiSessionMeta& session);
        //删除redis会话
        void removeBySessionIdToRedis(std::string sessionId);
        //根据会话ID查询会话
        suiDataSql::suiSessionMeta::ptr selectToRedis(const std::string& sessionId);

        //发布会话延迟删除消息
        void publishDeleteMessage(const std::vector<std::string>& sessionIdList);
        //通过会话id获取缓存key
        std::string getCacheKey(std::string sessionId);

        //获取用户所有会话id
        std::shared_ptr<std::vector<suiDataSql::suiSessionMeta::ptr>> getDataListByUserId(std::string userId);


    private:
        static const std::string _cache_prefix;  //缓存前缀
        static const int _cache_expire;  //缓存过期时间，单位秒
        static const std::string _field_session_id;  //会话id字段
        static const std::string _field_user_id;  //用户id字段
        static const std::string _field_upload_time;  //上传时间字段

        odb::database& _db;
        sw::redis::Redis& _redis;
        suiRemoveCache::RemoveCache::ptr _dataSync;
    };
}