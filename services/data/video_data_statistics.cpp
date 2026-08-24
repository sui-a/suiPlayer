#include "video_data_statistics.hpp"

namespace suiVideoDataStatistics
{
    const std::string VideoDataStatistics::_cacheKeyPrefix = "video_data_statistics_";
    const int VideoDataStatistics::_cache_expire = 60 * 60 * 2;  //缓存过期时间，单位秒，2小时
    const std::string VideoDataStatistics::_map_play_count = "play_count";
    const std::string VideoDataStatistics::_map_like_count = "like_count";

    VideoDataStatisticsAsync::VideoDataStatisticsAsync(std::shared_ptr<odb::database> mysql, std::shared_ptr<sw::redis::Redis> redis
                            , suiQueue::MQClient::ptr MqClientPtr, suiRemoveCache::RemoveCache::ptr dataSync)
        : _mysql(mysql)
        , _redis(redis)
        , _dataSync(dataSync)
    {
        //构造异步对象
        suiQueue::queueSetting syncset;
        syncset.exchange = "video_data_statistics_process_exchange",
        syncset.exchangeType = "delayed",
        syncset.queue = "video_data_statistics_process_queue",
        syncset.bindKey = "video_data_statistics_process",
        syncset.ttl = 3000;
        _bulletChatSync = std::make_shared<suiCacheSync::CacheSyncClient>(MqClientPtr, syncset, std::bind(&VideoDataStatisticsAsync::callback, this, std::placeholders::_1));
    }

    void VideoDataStatisticsAsync::async(const std::string& body)
    {
        _bulletChatSync->syncCache(body);
    }

    bool VideoDataStatisticsAsync::callback(std::string videoId)
    {
        int retry_count = 0;
        const int MAX_RETRIES = 3; // 最大重试次数
        while(retry_count < MAX_RETRIES)
        {
            try
            {
                //创建事务
                odb::transaction tx(_mysql->begin());
                auto rtx = _redis->transaction(false, false);
                {
                    auto& dbHandle = tx.database();
                    auto rHandle = rtx.redis();
                    suiVideoOperation::videoOperation op(dbHandle, rHandle, _dataSync);
                    op.addPlayCount(videoId);
                }
                tx.commit();
                return true;
            }
            catch (const odb::exception& e)
            {
                // 捕获 ODB 数据库异常
                ERROR("odb数据库更新视频播放次数异常： {}", e.what());
            }
            catch (const sw::redis::Error& e) 
            {
                // 捕获 Redis 异常
                ERROR("redis数据库更新视频播放次数异常： {}", e.what());
            }
            catch (...)
            {
                ERROR("更新视频播放次数时未知异常");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            retry_count++;
        }
        return false;
    }

    VideoDataStatistics::VideoDataStatistics(odb::transaction& tx, sw::redis::Transaction& rtx, suiRemoveCache::RemoveCache::ptr dataSync, VideoDataStatisticsAsync::ptr dbasyncPtr)
        : _tx(tx), _rtx(rtx), _dataSync(dataSync), _dbasyncPtr(dbasyncPtr)
    {

    }

    videoStatsAggregate::ptr VideoDataStatistics::getVideoStatistics(const std::string& videoId)
    {
        //先从缓存中获取
        auto cache = getVideoStatisticsFromCache(videoId);
        if (cache != nullptr)
        {
            return cache;
        }
        //从数据库中获取
        auto likeCount = getVideoLikeCountFromDb(videoId);
        auto playCount = getVideoPlayCountFromDb(videoId);
        auto out = std::make_shared<videoStatsAggregate>();
        out->playCount = playCount;
        out->likeCount = likeCount;
        //添加缓存
        insertCache(videoId, playCount, likeCount);
        return out;
    }

    void VideoDataStatistics::setVideoPlayCountByChange(const std::string& videoId, int playCountChange)
    {
        //同步修改缓存，异步操作数据库
        updateCachePlayCount(videoId, playCountChange);
        _dbasyncPtr->async(videoId);
    }

    void VideoDataStatistics::setVideoLikeCountByChange(const std::string& videoId, int likeCountChange)
    {
        updateCachePlayCount(videoId, 0, likeCountChange);
    }

    unsigned int VideoDataStatistics::getVideoLikeCountFromDb(const std::string& videoId)
    {
        //直接从数据库中获取视频点赞量
        auto& handle = _tx.database();
        auto result = handle.query_one<suiDataSql::suiUserLikeCountView>(odb::query<suiDataSql::suiUserLikeCountView>::suiUserLikeMeta::video_id == videoId);
        return result->count;
    }

    unsigned int VideoDataStatistics::getVideoPlayCountFromDb(const std::string& videoId)
    {
        auto& handle = _tx.database();
        auto result = handle.query_one<suiDataSql::suiVideoMeta>(odb::query<suiDataSql::suiVideoMeta>::video_id == videoId);
        return result->getVideoPlayCount();
    }

    void VideoDataStatistics::insertCache(const std::string& videoId, int playCount, long long likeCount)
    {
        //添加缓存
        auto _redis = _rtx.redis();
        std::string rediskey = buildCacheKey(videoId);
        std::unordered_map<std::string, std::string> _data;
        {
            _data[_map_play_count] = std::to_string(playCount);
            _data[_map_like_count] = std::to_string(likeCount);
        }
        _redis.hmset(rediskey, _data.begin(), _data.end());
        _redis.expire(rediskey, std::chrono::seconds(_cache_expire));
    }

    videoStatsAggregate::ptr VideoDataStatistics::getVideoStatisticsFromCache(const std::string& videoId)
    {
        auto _redis = _rtx.redis();
        std::string rediskey = buildCacheKey(videoId);
        std::unordered_map<std::string, std::string> _data;
        _redis.hgetall(rediskey, std::inserter(_data, _data.begin()));
        if (_data.empty())
        {
            return nullptr;
        }
        auto result = std::make_shared<videoStatsAggregate>();
        result->playCount =  static_cast<unsigned int>(std::stoul(_data[_map_play_count]));
        result->likeCount =  static_cast<unsigned int>(std::stoul(_data[_map_like_count]));
        return result;
    }

    void VideoDataStatistics::updateCachePlayCount(const std::string& videoId, int playCountChange, int likeCountChange)
    {
        //更新缓存
        auto _redis = _rtx.redis();
        std::string rediskey = buildCacheKey(videoId);
        bool isExit = false; //是否退出循环
        while (1) 
        {
            try
            {
                //设置监听对象
                _redis.watch(rediskey);
                while(1)
                {
                    bool ret = _redis.exists(rediskey);
                    if (!ret)
                    {
                        //数据不存在，直接插入
                        auto likeCount = getVideoLikeCountFromDb(videoId);
                        auto playCount = getVideoPlayCountFromDb(videoId);
                        insertCache(videoId, playCount, likeCount);
                        isExit = true;
                        break;
                    }
                    else
                    {
                        break;
                    }
                }
                if(isExit)
                    break; //此时重新插入了一遍属于最新数据
                
                //原来存在，更新播放量以及点赞量
                //直接从缓存中获取，然后更新
                auto stats = getVideoStatisticsFromCache(videoId);
                stats->playCount += playCountChange;
                stats->likeCount += likeCountChange;
                std::unordered_map<std::string, std::string> _data;
                {
                    _data[_map_play_count] = std::to_string(stats->playCount);
                    _data[_map_like_count] = std::to_string(stats->likeCount);
                }
                _rtx.hmset(rediskey, _data.begin(), _data.end());
                _redis.expire(rediskey, std::chrono::seconds(_cache_expire));
                break;
            }
            catch (const sw::redis::WatchError& e) 
            {
                ERROR("被别的线程抢先修改了，事务被打断，随机休眠1-10秒后重试");
                std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 2));
                continue;
            }
        }
    }

    void VideoDataStatistics::removeCache(const std::string& videoId)
    {
        _dataSync->syncCache({buildCacheKey(videoId)});
    }
    
    std::string VideoDataStatistics::buildCacheKey(const std::string& videoId)
    {
        return _cacheKeyPrefix + videoId;
    }

}