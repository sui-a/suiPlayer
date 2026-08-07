#pragma once
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/log.h>
#include "removeCache.hpp"
#include "data.hpp"
#include "data-odb.hxx"

namespace suiUserStatics
{
    //用户基础信息操作
    class suiStatics
    {
    public:
        suiStatics(odb::database& db, sw::redis::Redis& redis, suiRemoveCache::RemoveCache::ptr dataSync
            , sw::redis::Transaction& rtx);

        //获取用户基础数据
        suiDataSql::UserHomepageBasicData::ptr getUserBasicData(const std::string& userId);

        //更新用户数据操作
        //更新视频播放量
        void setVideoPlayCountByChange(const std::string& userId, int playCountChange);

        //更新缓存中的视频点赞数量
        void setVideoLikeCountByChange(const std::string& userId, int likeCountChange);

        //更新缓存中的用户粉丝数
        void setUserFansCountByChange(const std::string& userId, int fansCountChange);

        //更新缓存中的用户关注数
        void setUserFollowCountByChange(const std::string& userId, int followCountChange);
    private:
        //从数据库查询用户基础信息
        suiDataSql::UserHomepageBasicData::ptr getUserBasicDataToDb(const std::string& userId);

        //从redis查询用户基础信息
        suiDataSql::UserHomepageBasicData::ptr getUserBasicDataToRedis(const std::string& userId);
        //redis添加信息
        void insertUserBasicDataToRedis(const std::string& userId, const suiDataSql::UserHomepageBasicData::ptr& data);
        //移除缓存
        void removeUserBaseDataToRedis(const std::string& userId);

        //发布移除消息
        void publishRemoveMessage(const std::vector<std::string>& curCacheKey);

        //设置缓存数据时间
        void setCacheDataTime(const std::string& userId);

        //构建缓存key
        std::string buildCacheKey(const std::string& userId);

        //获取范围内随机缓存过期时间
        int getCacheExpireTime();

        //更新缓存中某个字段的值
        void updateCacheField(const std::string& rediskey, const std::string& mapkey, size_t value);

    private:
        //数据库操作句柄
        odb::database& _db;  //绑定好事务的数据库句柄
        //redis操作句柄（绑定链接的）
        sw::redis::Redis& _redis;
        //redis事务对象
        sw::redis::Transaction& _rtx;
        //缓存操作客户端
        suiRemoveCache::RemoveCache::ptr _dataSync;
        


        //缓存键的前缀
        static const std::string cacheKeyPrefix;
        //map粉丝数字段
        static const std::string mapKeyFansCount;
        //map关注数字段
        static const std::string mapKeyFollowCount;
        //map视频点赞总量字段
        static const std::string mapKeyVideoLikeCount;
        //map视频播放量字段
        static const std::string mapKeyVideoPlayCount;
        //缓存过期时间
        static const int _cache_expire_min;
        static const int _cache_expire_max;
        
    };
}