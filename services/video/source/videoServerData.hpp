#pragma once
#include <string>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiRandom.hpp>
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiQueue.hpp>
#include "base.pb.h"
#include "video.pb.h"
#include "data.hpp"
#include "data-odb.hxx"
#include "cache_sync.hpp"
#include "removeCache.hpp"
#include "user_information_operation.hpp"
#include "session.hpp"
#include "video_operation_meta.hpp"
#include "video_catgory_tag.hpp"
#include "video_subtitle_target.hpp"
#include "user_data_statistics.hpp"
#include "video_data_statistics.hpp"
#include "user_data_statistics.hpp"
#include "videoListOperational.hpp"
#include "user_Identity_role.hpp"
#include "video_like.hpp"
#include "video_search.hpp"
#include "error.proto.hpp"
#include "suiTime.hpp"

namespace suiVideoServer
{
    class videoServerData
    {
    public:
        using ptr = std::shared_ptr<videoServerData>;

        videoServerData(std::shared_ptr<odb::database> mysql,
                std::shared_ptr<sw::redis::Redis> redis, std::shared_ptr<suiRemoveCache::RemoveCache> cache_sync
            , suiVideoSubtitleTarget::videoSubtitleAsync::ptr videoSubtitleAsync
            , suiVideoSearch::videoSearch::ptr videoSearchHandle
            , suiCacheSync::CacheSyncClient::ptr fileRemove, suiCacheSync::CacheSyncClient::ptr transcode_operation_handle
            , suiVideoDataStatistics::VideoDataStatisticsAsync::ptr videoStaticsSync);
        
        //新增视频信息
        void newVideo(const std::string& ssid, const suiApi::VideoInfo& videoInfo, int32_t& error_code, std::string& error_msg);
        //删除视频信息
        void deleteVideo(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg);
        //点赞判断
        void videoLikeJudge(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg, bool& isLike);
        //点赞/取消点赞操作
        void videoLikeOperation(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg);
        //视频播放接口
        void videoPlayOperation(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg);
        //新增视频弹幕
        void addVideoSubtitle(const std::string& ssid, const suiApi::VideoSubtitleInfo& videoSubtitleInfo, int32_t& error_code, std::string& error_msg);
        //获取弹幕列表
        void getVideoSubtitle(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg, suiApi::getVideoSubtitleResult& result);
        //视频审核
        void checkVideo(const std::string& ssid, const std::string& videoId, bool isPass, int32_t& error_code, std::string& error_msg);
        //上架视频
        void onlineVideo(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg);
        //下架视频
        void offlineVideo(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg);
        //获取用户视频列表
        void getUserVideoList(const std::string& ssid, const std::string& targetUserId, int32_t pageIndex, int32_t pageCount
            , int32_t& error_code, std::string& error_msg, suiApi::getUserVideoListResult& result);
        //管理员获取状态视频列表
        void getVideoListByStatus(const std::string& ssid, suiApi::videoStatus status, int32_t pageIndex, int32_t pageCount
            , int32_t& error_code, std::string& error_msg, suiApi::getStatusVideoListResult& result);
        //获取主页视频列表
        void getMainVideoList(const std::string& ssid, int32_t pageIndex, int32_t pageCount
            , int32_t& error_code, std::string& error_msg, suiApi::getHomeRandomVideoListResult& result);
        //根据分类标签获取视频列表
        void getVideoListByTag(const std::string& ssid, int32_t tagid, int32_t pageIndex, int32_t pageCount
            , int32_t& error_code, std::string& error_msg, suiApi::getTagVideoListResult& result);
        //根据搜索关键词获取视频列表
        void getVideoListBySearch(const std::string& ssid, const std::string& keyword, int32_t pageIndex, int32_t pageCount
            , int32_t& error_code, std::string& error_msg, suiApi::searchVideoListResult& result);
        //获取分类标签列表
        void getTagList(int32_t& error_code, std::string& error_msg, suiApi::getTagInfoRsp& result);

    private:
        //构造视频id
        std::string createId();


        //api接口的视频状态与数据库状态映射
        suiDataSql::videoStatus videoStatusMap(suiApi::videoStatus status);
        suiApi::videoStatus videoStatusMap(suiDataSql::videoStatus status);



    private:
        //操作句柄
        std::shared_ptr<odb::database> _mysql;
        std::shared_ptr<sw::redis::Redis> _redis;
        //普通缓存删除同步句柄
        suiRemoveCache::RemoveCache::ptr _cache_sync;
        //异步视频弹幕维护类
        suiVideoSubtitleTarget::videoSubtitleAsync::ptr _videoSubtitleAsync;
        //es操作句柄
        suiVideoSearch::videoSearch::ptr _videoSearchHandle;
        //文件异步删除
        suiCacheSync::CacheSyncClient::ptr _fileRemove;
        //转码消息发布句柄
        suiCacheSync::CacheSyncClient::ptr _transcode_operation_handle;
        //视频异步同步类
        suiVideoDataStatistics::VideoDataStatisticsAsync::ptr _videoStaticsSync;
    };


}