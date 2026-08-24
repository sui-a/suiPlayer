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
#include "video_operation_meta.hpp"
#include "cache_sync.hpp"

namespace suiVideoDataStatistics
{
    //信息聚合结构体
    struct videoStatsAggregate
    {
        using ptr = std::shared_ptr<videoStatsAggregate>;
        unsigned int playCount;
        unsigned int likeCount;
    };

    //数据库异步类
    class VideoDataStatisticsAsync
    {
    public:
        using ptr = std::shared_ptr<VideoDataStatisticsAsync>;
        VideoDataStatisticsAsync(std::shared_ptr<odb::database> mysql, std::shared_ptr<sw::redis::Redis> redis, suiQueue::MQClient::ptr MqClientPtr, suiRemoveCache::RemoveCache::ptr dataSync);

        void async(const std::string& body);
    private:
        bool callback(std::string body);

    private:
        //缓存同步队列
        suiCacheSync::CacheSyncClient::ptr _bulletChatSync;
        std::shared_ptr<odb::database> _mysql;
        std::shared_ptr<sw::redis::Redis> _redis;
        suiRemoveCache::RemoveCache::ptr _dataSync;
    };

    //视频数据统计
    class VideoDataStatistics
    {
    public:
        //传入事务
        VideoDataStatistics(odb::transaction& tx, sw::redis::Transaction& rtx, suiRemoveCache::RemoveCache::ptr dataSync, VideoDataStatisticsAsync::ptr dbasyncPtr);

        //获取视频数据
        videoStatsAggregate::ptr getVideoStatistics(const std::string& videoId);

        //数据修改接口
        void setVideoPlayCountByChange(const std::string& videoId, int playCountChange);
        void setVideoLikeCountByChange(const std::string& videoId, int likeCountChange);


    private:
        //数据库操作
        //从数据库中获取视频点赞量
        unsigned int getVideoLikeCountFromDb(const std::string& videoId);
        //从数据库中获取视频播放量
        unsigned int getVideoPlayCountFromDb(const std::string& videoId);

        //缓存操作类
        //添加视频数据统计缓存
        void insertCache(const std::string& videoId, int playCount, long long likeCount);
        //获取视频数据统计缓存
        videoStatsAggregate::ptr getVideoStatisticsFromCache(const std::string& videoId);
        //修改缓存视频播放量与视频点赞量
        void updateCachePlayCount(const std::string& videoId, int playCountChange = 0, int likeCountChange = 0);
        //删除缓存
        void removeCache(const std::string& videoId);

        //构建key
        std::string buildCacheKey(const std::string& videoId);

    private:
        odb::transaction& _tx;
        sw::redis::Transaction& _rtx;
        suiRemoveCache::RemoveCache::ptr _dataSync;
        VideoDataStatisticsAsync::ptr _dbasyncPtr;

        //缓存key前缀
        static const std::string _cacheKeyPrefix;
        //缓存超时时间
        static const int _cache_expire;
        //缓存映射字段
        static const std::string _map_play_count;
        static const std::string _map_like_count;
    };



}