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

namespace suiVideoListOperational
{
    //列表缓存以及同步更新
    class videoListOperational
    {
    public:
        videoListOperational(odb::transaction& tx, sw::redis::Transaction& rtx, suiRemoveCache::RemoveCache::ptr dataSync);

        //获取主页视频id列表
        std::vector<std::string> getVideoMainList(int page, int size);
        //主页同步处理接口
        void syncVideoMainList(const std::string& videoId);

        //获取用户视频id列表
        std::vector<std::string> getUserVideoMainList(const std::string& userId, int page, int size);
        //获取用户视频总数
        int32_t getUserVideoTotal(const std::string& userId);
        //同步处理用户视频接口
        void syncUserVideoList(const std::string& userId);

        //获取分类视频id列表
        std::vector<std::string> getCategoryVideoList(long long categoryId, int page, int size);
        //获取分类视频总数
        int32_t getCategoryVideoTotal(long long categoryId);
        //视频同步处理接口
        void syncCategoryVideoList(long long categoryId, const std::string& videoId);

        //获取视频状态列表
        std::vector<std::string> getVideoStatusList(suiDataSql::videoStatus status, int page, int size);
        //获取视频状态总数
        int32_t getVideoStatusTotal(suiDataSql::videoStatus status);

    private:
        //从数据库获取主页视频列表
        std::vector<suiDataSql::VideoIdList> getVideoMainListFromDb(int page, int size);
        //将主页视频列表插入缓存
        void insertVideoMainListToCache(std::vector<suiDataSql::VideoIdList> dbList);
        //从缓存中获取主页视频列表
        std::shared_ptr<std::vector<std::string>> getVideoMainListFromCache(int page, int size);

        //从数据库中获取用户视频id列表
        std::vector<suiDataSql::VideoIdList> getUserVideoListFromDb(const std::string& userId);
        //把用户视频id列表插入缓存
        void insertUserVideoListToCache(const std::string& userId, std::vector<suiDataSql::VideoIdList>& videoList);
        //从缓存中获取用户视频id列表
        std::shared_ptr<std::vector<std::string>>  getUserVideoListFromCache(const std::string& userId, int page, int size);

        //从数据库中获取对应标签的视频id列表
        std::vector<suiDataSql::TaggedVideoItem> getCategoryVideoListFromDb(long long categoryId, int page, int size);
        //把分类视频id列表插入缓存
        void insertCategoryVideoListToCache(long long categoryId, std::vector<suiDataSql::TaggedVideoItem>& videoList);
        //从缓存中获取分类视频id列表
        std::shared_ptr<std::vector<std::string>> getCategoryVideoListFromCache(long long categoryId, int page, int size);

        //删除缓存，并延迟发布删除消息
        void syncDeleteCache(const std::string& cacheKey);
        //获取主页视频缓存key
        const std::string& getVideoMainListCacheKey();
        //获取用户视频缓存key
        std::string getUserVideoListCacheKey(const std::string& userId);
        //获取分类页面key
        std::string getVideoCategoryListCacheKey(long long categoryId);
        //获取状态key
        std::string getVideoStatusListCacheKey(suiDataSql::videoStatus status);

    private:
        odb::transaction& _tx;
        sw::redis::Transaction& _rtx;
        suiRemoveCache::RemoveCache::ptr _dataSync;
        odb::transaction::database_type& _mysql;
        sw::redis::Redis _redis;

        //主页视频缓存key
        static const std::string _videoMainListKey;
        //用户缓存key前缀
        static const std::string _userVideoListKeyPrefix;
        //分类缓存key前缀
        static const std::string _videoCategoryListKeyPrefix;
        //状态前缀
        static const std::string _videoStatusListKeyPrefix;
        //默认单key缓存的视频数量
        static const int _defaultCacheSize;
        //默认缓存过期时间
        static const int _defaultCacheExpire;
    };





}