#include "user_data_statistics.hpp"

namespace suiUserStatics
{

    const std::string suiStatics::cacheKeyPrefix = "user_basic_data_";
    const std::string suiStatics::mapKeyFansCount = "followers_count";
    const std::string suiStatics::mapKeyFollowCount = "follow_count";
    const std::string suiStatics::mapKeyVideoLikeCount = "video_like_count";
    const std::string suiStatics::mapKeyVideoPlayCount = "video_play_count";
    const int suiStatics::_cache_expire_min = 1800;
    const int suiStatics::_cache_expire_max = 3600;
 

    suiStatics::suiStatics(odb::database& db, sw::redis::Redis& redis, suiRemoveCache::RemoveCache::ptr dataSync
            , sw::redis::Transaction& rtx)   
        : _db(db), _redis(redis), _rtx(rtx), _dataSync(dataSync)
    {

    }

    suiDataSql::UserHomepageBasicData::ptr suiStatics::getUserBasicData(const std::string& userId)
    {
        //缓存中查询用户基础数据
        auto ret = getUserBasicDataToRedis(userId);
        if(ret)
        {
            //更新时间
            setCacheDataTime(userId);
            return ret;
        }
        //缓存中不存在
        //从数据库中查询
        ret = getUserBasicDataToDb(userId);
        if(ret)
        {
            //设置缓存
            insertUserBasicDataToRedis(userId, ret);
        }
        return ret;
    }

    void suiStatics::setVideoPlayCountByChange(const std::string& userId, int playCountChange)
    {
        //通过变化异步修改数据库中的数据
        //_setUserStatistics->setVideoChangeSyncCache(userId, playCountChange);

        //本地直接更新缓存
        updateCacheField(userId, mapKeyVideoPlayCount, playCountChange);
    }

    void suiStatics::setVideoLikeCountByChange(const std::string& userId, int likeCountChange)
    {
        updateCacheField(userId, mapKeyVideoLikeCount, likeCountChange);
    }

    void suiStatics::setUserFansCountByChange(const std::string& userId, int fansCountChange)
    {
        updateCacheField(userId, mapKeyFansCount, fansCountChange);
    }

    void suiStatics::setUserFollowCountByChange(const std::string& userId, int followCountChange)
    {
        updateCacheField(userId, mapKeyFollowCount, followCountChange);
    }

    suiDataSql::UserHomepageBasicData::ptr suiStatics::getUserBasicDataToDb(const std::string& userId)
    {
        //从数据库获取粉丝数
        auto funCount = _db.query_one<suiDataSql::suiUserFollowCountView>(odb::query<suiDataSql::suiUserFollowCountView>::follow_user_id == userId);
        //从数据库获取关注数
        auto followCount = _db.query_one<suiDataSql::suiUserFollowCountView>(odb::query<suiDataSql::suiUserFollowCountView>::user_id == userId);
        //从数据库获取用户的视频播放总数
        auto videoPlayCount = _db.query_one<suiDataSql::suiVideoPlayCountView>(odb::query<suiDataSql::suiVideoPlayCountView>::upload_user_id == userId);
        //从数据库获取点赞总量
        auto likeCount = _db.query_one<suiDataSql::suiUserLikeCountView>(odb::query<suiDataSql::suiUserLikeCountView>::suiVideoMeta::upload_user_id == userId);

        //构造
        std::shared_ptr<suiDataSql::UserHomepageBasicData> ret = std::make_shared<suiDataSql::UserHomepageBasicData>();
    
        // 使用三目运算符安全赋值：如果指针不为空就取 count，为空就给 0
        ret->fansCount = funCount ? funCount->count : 0;
        ret->followCount = followCount ? followCount->count : 0;
        ret->videoPlayCount = videoPlayCount ? videoPlayCount->count : 0;
        ret->videoLikeCount = likeCount ? likeCount->count : 0;
        return ret;
    }

    void suiStatics::insertUserBasicDataToRedis(const std::string& userId, const suiDataSql::UserHomepageBasicData::ptr& data)
    {
        std::string key = buildCacheKey(userId);

        std::unordered_map<std::string, std::string> _data;
        _data[mapKeyFansCount] = std::to_string(data->fansCount);
        _data[mapKeyFollowCount] = std::to_string(data->followCount);
        _data[mapKeyVideoLikeCount] = std::to_string(data->videoLikeCount);
        _data[mapKeyVideoPlayCount] = std::to_string(data->videoPlayCount);
        
        //进行插入
        _redis.hmset(key, _data.begin(), _data.end());
        //设置时间
        setCacheDataTime(userId);
    }


    suiDataSql::UserHomepageBasicData::ptr suiStatics::getUserBasicDataToRedis(const std::string& userId)
    {
        //构造key
        std::string key = buildCacheKey(userId);
        //获取缓存数据
        std::unordered_map<std::string, std::string> _data;

        _redis.hgetall(key, std::inserter(_data, _data.begin()));
        if(_data.empty())
            return nullptr;

        //存在，直接构造
        std::shared_ptr<suiDataSql::UserHomepageBasicData> ret = std::make_shared<suiDataSql::UserHomepageBasicData>();
        ret->fansCount = std::stoll(_data[mapKeyFansCount]);
        ret->followCount = std::stoll(_data[mapKeyFollowCount]);
        ret->videoLikeCount =  std::stoll(_data[mapKeyVideoLikeCount]);
        ret->videoPlayCount =  std::stoll(_data[mapKeyVideoPlayCount]);
        return ret;
    }

    
    void suiStatics::removeUserBaseDataToRedis(const std::string& userId)
    {
        //删除缓存
        publishRemoveMessage({buildCacheKey(userId)});
    }

    void suiStatics::publishRemoveMessage(const std::vector<std::string>& curCacheKey)
    {
        //发布缓存删除消息队列
        if(curCacheKey.empty())
            return;
        //发布消息
        _dataSync->syncCache(curCacheKey);
    }

    void suiStatics::setCacheDataTime(const std::string& userId)
    {
        std::string key = buildCacheKey(userId);
        //过期返回false，此时不处理，直接当作失效
        _redis.expire(key, std::chrono::seconds(getCacheExpireTime()));
    }

    std::string suiStatics::buildCacheKey(const std::string& userId)
    {
        return cacheKeyPrefix + userId;
    }

    int suiStatics::getCacheExpireTime()
    {
        thread_local std::mt19937 gen(std::random_device{}());
        // 均匀分布
        std::uniform_int_distribution<int> distrib(_cache_expire_min, _cache_expire_max);
        return distrib(gen);
    }

    void suiStatics::updateCacheField(const std::string& userId, const std::string& mapkey, size_t value)
    {
        //更新缓存
        std::string rediskey = buildCacheKey(userId);
        bool isExit = false; //是否退出循环
        while (1) 
        {
            try 
            {
                //监听对象
                _redis.watch(rediskey);
                while(1)
                {
                    bool ret = _redis.exists(rediskey);
                    //获取原数据后，无论对象是否存在，redis中都进行更新
                    if(!ret)
                    {
                        //当前缓存不存在用户信息
                        _redis.unwatch();//取消监视
                        auto data = getUserBasicData(userId);
                        if(!data)
                        {
                            ERROR("不存在该用户：{} 的信息", userId);
                            isExit = true;
                            break;
                        }
                        _redis.watch(rediskey); //成功获取到数据，开启监视
                    }
                    else
                    {
                        break;
                    }
                }
                if(isExit)
                    break;
                
                //此时缓存中存在用户信息
                //获取缓存用户信息
                std::unordered_map<std::string, std::string> _data;
                _redis.hgetall(rediskey, std::inserter(_data, _data.begin()));
                
                INFO("开始更新, 原来的值： {}， 修改为了 {}", std::stoll(_data[mapkey]), std::stoll(_data[mapkey]) + value);
                _data[mapkey] = std::to_string(std::stoll(_data[mapkey]) + value);
                //开始更新
                _rtx.hmset(rediskey, _data.begin(), _data.end());
                //更新成功
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
}