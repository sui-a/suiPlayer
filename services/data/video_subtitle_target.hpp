#pragma once
#include <memory>
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/log.h>
#include <suiScaffold/suiJson.hpp>
#include <odb/query.hxx>
#include <odb/database.hxx>
#include <odb/result.hxx>
#include "data.hpp"
#include "data-odb.hxx"
#include "removeCache.hpp"
#include "cache_sync.hpp"

namespace suiVideoSubtitleTarget
{
    //缓存key前缀
    extern const std::string _cacheKeyPrefix;
    //map映射字段
    extern const std::string _map_video_id;
    extern const std::string _map_user_id;
    extern const std::string _map_subtitle_id;
    extern const std::string _map_content;
    extern const std::string _map_video_time;
    extern const std::string _map_create_time;
    extern const int _cache_expire;

    //异步删除缓存视频弹幕
    extern const std::string async_delete_cache_by_video_id;
    //异步向数据库新增
    extern const std::string async_insert_db_by_subtitle;
    //异步向数据库删除
    extern const std::string async_delete_db_by_subtitleid;
    extern const std::string async_delete_db_by_video_id;

    //视频弹幕异步维护类
    class videoSubtitleAsync
    {
    public:
        using ptr = std::shared_ptr<videoSubtitleAsync>;
        videoSubtitleAsync(std::shared_ptr<odb::database> mysql, std::shared_ptr<sw::redis::Redis> redis, suiQueue::MQClient::ptr MqClientPtr);

        void async(const std::string& body);

    private:
        bool callback(std::string body);

        //数据库增删接口
        void insertBySubtitleidToDb(const Json::Value& list);
        void removeBySubtitleidToDb(const Json::Value& list);
        //数据库全量删除
        void removeByVideoidToDb(const Json::Value& list);
        //缓存全量删除
        void removeByVideoidToCache(const Json::Value& list);
        
    private:
        //缓存同步队列
        suiCacheSync::CacheSyncClient::ptr _bulletChatSync;
        std::shared_ptr<odb::database> _mysql;
        std::shared_ptr<sw::redis::Redis> _redis;
    };

    //视频弹幕处理
    class videoSubtitleTarget
    {
    public:
        using ptr = std::shared_ptr<videoSubtitleTarget>;

        //参数 普通字段删除队列， 弹幕增删队列
        videoSubtitleTarget(odb::transaction& sql_t,  sw::redis::Transaction& cache_tx
            , videoSubtitleAsync::ptr bulletChatSync);

        //新增单个弹幕
        void insert(suiDataSql::suiVideoSubtitleTarget::ptr subtitle);
        //发送者id，视频id，弹幕id，内容，视频偏移时间
        void insert(const std::string& userid, const std::string& video_id, const std::string& subtitleid, const std::string& content, long long time);

        //查
        //查询某个视频中的弹幕
        suiDataSql::videoBulletChatList::ptr select(const std::string& video_id);

        //删
        //删除某个视频中的弹幕
        void removeByVideoid(const std::string& video_id);
        //删除某个弹幕
        void removeBySubtitleid(const std::string& video_id, const std::string& subtitleid);
        void removeBySubtitle(suiDataSql::suiVideoSubtitleTarget::ptr subtitle);

    private:
        friend class videoSubtitleAsync;

        //同步接口
        //缓存增删查
        //对某个已存在的进行增
        void insertToCache(suiDataSql::suiVideoSubtitleTarget::ptr subtitle);
        //增加整个视频的弹幕缓存
        void insertToCache(suiDataSql::videoBulletChatList::ptr bulletChatList);
        //缓存查询
        suiDataSql::videoBulletChatList::ptr selectFromCache(const std::string& video_id);
        //删除视频中某个弹幕缓存 -- 全量删除提供异步接口执行
        void removeToCache(const std::string& video_id, const std::string& subtitleid);
        //数据库查
        suiDataSql::videoBulletChatList::ptr selectFromDb(const std::string& video_id);
        

        //异步接口
        //缓存全量删除
        void removeToCache(const std::string& video_id);
        //数据库增
        void insertToDb(suiDataSql::suiVideoSubtitleTarget::ptr subtitle);
        //数据库删
        void removeBySubtitleidToDb(const std::string& subtitleid);
        void removeByVideoidToDb(const std::string& video_id);

        std::string getKey(const std::string& video_id);

    private:
        //数据库事务
        odb::transaction& _mysqltx;
        sw::redis::Transaction& _cacheTx;
        //弹幕异步队列
        videoSubtitleAsync::ptr _bulletChatSync;
    };
}





