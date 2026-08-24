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

namespace suiVideoOperation
{
    class videoOperation
    {
    public:
        videoOperation(odb::database& db, sw::redis::Redis& redis, suiRemoveCache::RemoveCache::ptr dataSync);
        
        //新增视频元数据
        void insert(suiDataSql::suiVideoMeta& video);
        //删除视频信息
        void remove(const std::string& videoId);
        //通过视频id获取视频元数据
        suiDataSql::suiVideoMeta::ptr select(const std::string& videoId);
        //强制从数据库中获取
        suiDataSql::suiVideoMeta::ptr selectToDbForce(const std::string& videoId);
        //更新视频信息
        void update(suiDataSql::suiVideoMeta& video);
        //不进行搜索直接更新 配合上述直接从数据库强制获取元数据的接口，因为视频元数据过多，所以暴露给外部调用
        void updateToDbForce(suiDataSql::suiVideoMeta& video);
        //通过用户id获取视频元数据列表 参数：用户id，页数，页大小
        suiDataSql::suiVideoMetaList::ptr selectByUserId(const std::string& userId, size_t page, size_t pageSize);
        //增加播放量
        void addPlayCount(const std::string& videoId, int playCountChange = 1);

    private:
        //向数据库新增视频信息
        void insertToDb(suiDataSql::suiVideoMeta& video);
        //从数据库中获取视频信息
        suiDataSql::suiVideoMeta::ptr selectToDb(const std::string& videoId);
        //修改数据库视频信息
        void updateToDb(suiDataSql::suiVideoMeta& video);
        //删除数据库视频信息
        void removeToDb(const std::string& videoId);
        //获取列表
        suiDataSql::suiVideoMetaList::ptr selectByUserIdToDb(const std::string& userId, size_t page, size_t pageSize);
        //向数据库增加播放量
        void addPlayCountToDb(const std::string& videoId, int playCountChange = 1);

        //缓存相关
        //添加视频信息缓存
        void insertToCache(suiDataSql::suiVideoMeta& video);
        //搜索
        suiDataSql::suiVideoMeta::ptr selectToCache(const std::string& videoId);
        //删除
        void removeToCache(const std::string& videoId);

        std::string getCacheKey(const std::string& videoId);

    private:
        odb::database& _db;
        sw::redis::Redis& _redis;
        suiRemoveCache::RemoveCache::ptr _dataSync;


        //缓存前缀字段
        static const std::string _cachePrefix;
        //map映射字段
        static const std::string _map_video_id;
        static const std::string _map_video_file_id;
        static const std::string _map_video_cover_file_id;
        static const std::string _map_upload_user_id;
        static const std::string _map_review_user_id;
        static const std::string _map_video_name;
        static const std::string _map_video_description;
        static const std::string _map_video_play_count;
        static const std::string _map_video_size;
        static const std::string _map_video_duration;
        static const std::string _map_video_upload_time;
        static const std::string _map_video_status;
        //缓存过期时间
        static const int _cache_expire;
    };

    class videoOperationDb
    {
    public:
        videoOperationDb(odb::database& db);
        suiDataSql::suiVideoMeta::ptr select(const std::string& videoId);
        void update(suiDataSql::suiVideoMeta& video);


    private:
        odb::database& _db;
    };
}