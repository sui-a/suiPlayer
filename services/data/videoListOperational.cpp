#include "videoListOperational.hpp"

namespace suiVideoListOperational
{
    const std::string videoListOperational::_videoMainListKey = "videoListOperational_videoMainList";
    const std::string videoListOperational::_userVideoListKeyPrefix = "videoListOperational_userVideoList_";
    const std::string videoListOperational::_videoCategoryListKeyPrefix = "videoListOperational_videoCategoryList_";
    const std::string videoListOperational::_videoStatusListKeyPrefix = "videoListOperational_videoStatusList_";
    const int videoListOperational::_defaultCacheSize = 10;
    const int videoListOperational::_defaultCacheExpire = 60 * 15;
 
    videoListOperational::videoListOperational(odb::transaction& tx, sw::redis::Transaction& rtx, suiRemoveCache::RemoveCache::ptr dataSync)
        : _tx(tx), _rtx(rtx), _dataSync(dataSync), _mysql(tx.database()), _redis(rtx.redis())
    {

    }

    std::vector<std::string> videoListOperational::getVideoMainList(int page, int size)
    {
        //判断是否在缓存范围内
        if((page + 1) * size <= _defaultCacheSize)
        {
            //先向缓存中获取
            auto cacheList = getVideoMainListFromCache(page, size);
            if(cacheList != nullptr)
                return *cacheList; //不为0说明缓存命中，直接返回即可
            else
            {
                //缓存没命中
                //先添加缓存
                std::vector<std::string> out;
                auto dbList = getVideoMainListFromDb(0, _defaultCacheSize);
                //如果数据再缓存范围内, 或者读到末尾(dbList.size() <= _defaultCacheSize)，直接返回有效部分
                //直接把dbList中对应的输出出去
                insertVideoMainListToCache(dbList); //直接插入缓存
                for(size_t i = page * size; i < (size_t)(page * size + size) && i < dbList.size(); i++)
                    out.push_back(dbList[i].video_id);
                return out;
            }
        }
        //到这里没有返回，说明数据再1000条范围外，直接从数据库中再读取一次
        std::vector<std::string> out;
        auto dbList = getVideoMainListFromDb(page, size);
        for(auto& it : dbList)
            out.push_back(it.video_id); 
        return out;
    }

    void videoListOperational::syncVideoMainList(const std::string& videoId)
    {
        //判断状态改变的视频是否存在缓存中
        auto score = _redis.zscore(getVideoMainListCacheKey(), videoId);
        if(score)
        {
            //存在 直接进行同步
            return syncDeleteCache(getVideoMainListCacheKey());
        }
        //视频不存在缓存中，判断缓存数量
        auto size = _redis.zcard(getVideoMainListCacheKey());
        if(size < _defaultCacheSize) //进行同步
            return syncDeleteCache(getVideoMainListCacheKey());
    }

    std::vector<suiDataSql::VideoIdList> videoListOperational::getVideoMainListFromDb(int page, int size)
    {
        //从数据库中获取主页视频列表
        std::vector<suiDataSql::VideoIdList> out;
        odb::query<suiDataSql::suiVideoMeta> q(odb::query<suiDataSql::suiVideoMeta>::video_status == suiDataSql::videoStatus::videoStatusApproved);
        q += " " + suiDataSql::SqlPaginationUtil::buildOrderByClause("video_upload_time", true) + " " + suiDataSql::SqlPaginationUtil::buildPageClause(size, page);
        auto ret = _mysql.query<suiDataSql::suiVideoMeta>(q);
        for(auto& it : ret)
        {
            out.push_back(suiDataSql::VideoIdList());
            out.back().video_id = it.getVideoId();
            out.back().order_field = it.getVideoUploadTime();
        }
        return out;
    }


    void videoListOperational::insertVideoMainListToCache(std::vector<suiDataSql::VideoIdList> dbList)
    {
        auto& key = getVideoMainListCacheKey();
        if(dbList.empty())
        {
            //为空，插入空位符，防止击穿
            _redis.zadd(key, "__EMPTY_PLACEHOLDER__", 0);
            _redis.expire(key, std::chrono::seconds(_defaultCacheExpire / 2)); // 空缓存设置较短过期时间（如 60 秒）
            return;
        }
        //组织数据
        std::unordered_map<std::string, long long> data;
        for(auto& it : dbList)
            data[it.video_id] = it.order_field;
        _redis.zadd(key, data.begin(), data.end());
        _redis.expire(key, std::chrono::seconds(_defaultCacheExpire));
    }

    std::shared_ptr<std::vector<std::string>> videoListOperational::getVideoMainListFromCache(int page, int size)
    {
        std::shared_ptr<std::vector<std::string>> out = nullptr;
        if(_redis.exists(getVideoMainListCacheKey()))
        {
            //从缓存中获取
            out = std::make_shared<std::vector<std::string>>();
            std::vector<std::string> temp;
            _redis.zrevrange(getVideoMainListCacheKey(), page * size, (page + 1) * size - 1, std::back_inserter(temp));
            for (auto& id : temp)
            {
                if (id != "__EMPTY_PLACEHOLDER__")
                {
                    out->push_back(std::move(id));
                }
            }
        }
        return out;
    }

    void videoListOperational::syncDeleteCache(const std::string& cacheKey)
    {
        _dataSync->syncCache({cacheKey});
    }

    const std::string& videoListOperational::getVideoMainListCacheKey()
    {
        return _videoMainListKey;
    }

    std::string videoListOperational::getUserVideoListCacheKey(const std::string& userId)
    {
        return _userVideoListKeyPrefix + userId;
    }

    std::string videoListOperational::getVideoCategoryListCacheKey(long long categoryId)
    {
        return _videoCategoryListKeyPrefix + std::to_string(categoryId);
    }

    std::string videoListOperational::getVideoStatusListCacheKey(suiDataSql::videoStatus status)
    {
        return _videoStatusListKeyPrefix + suiDataSql::videoStatusUtil::toStringFromEnum(status);
    }

    std::vector<std::string> videoListOperational::getUserVideoMainList(const std::string& userId, int page, int size)
    {
        //先从缓存中获取
        auto curlist = getUserVideoListFromCache(userId, page, size); //返回智能指针
        if(curlist == nullptr) //缓存未命中
        {
            //缓存没命中
            //先添加缓存
            std::vector<std::string> out;
            auto dbList = getUserVideoListFromDb(userId);  //对于用户视频，默认量少，直接全量读取
            insertUserVideoListToCache(userId, dbList);
            //进行筛选
            for(size_t i = page * size; i < (size_t)(page * size + size) && i < dbList.size(); i++)
                out.push_back(dbList[i].video_id);
            return out;
        }
        else
        {
            return *curlist;
        }
    }

    int32_t videoListOperational::getUserVideoTotal(const std::string& userId)
    {
        odb::query<suiDataSql::suiVideoCountView> q(odb::query<suiDataSql::suiVideoCountView>::upload_user_id == userId
                                        && odb::query<suiDataSql::TaggedVideoCount>::suiVideoMeta::video_status == suiDataSql::videoStatus::videoStatusApproved);
        auto ret = _mysql.query_one<suiDataSql::suiVideoCountView>(q);
        return ret->count;
    }

    void videoListOperational::syncUserVideoList(const std::string& userId)
    {
        //不用在乎视频是哪个，缓存对于用户时全量同步的，所以直接删除缓存
        return syncDeleteCache(getUserVideoListCacheKey(userId));
    }

    std::vector<suiDataSql::VideoIdList> videoListOperational::getUserVideoListFromDb(const std::string& userId)
    {
        //从数据库中获取用户视频列表
        std::vector<suiDataSql::VideoIdList> out;
        odb::query<suiDataSql::VideoIdList> q(odb::query<suiDataSql::VideoIdList>::upload_user_id == userId 
                                            && odb::query<suiDataSql::VideoIdList>::video_status == suiDataSql::videoStatus::videoStatusApproved);
        q += " " + suiDataSql::SqlPaginationUtil::buildOrderByClause("video_upload_time", true);
        auto ret = _mysql.query<suiDataSql::VideoIdList>(q);
        for(auto& it : ret)
        {
            out.push_back(suiDataSql::VideoIdList());
            out.back().video_id = it.video_id;
            out.back().order_field = it.order_field;
        }
        return out;
    }

    void videoListOperational::insertUserVideoListToCache(const std::string& userId, std::vector<suiDataSql::VideoIdList>& videoList)
    {
        //将主页视频列表插入缓存
        auto key = getUserVideoListCacheKey(userId);
        if (videoList.empty())
        {
            //为空时，插入一个占位符，防止报错
            _redis.zadd(key, "__EMPTY_PLACEHOLDER__", 0);
            _redis.expire(key, std::chrono::seconds(_defaultCacheExpire / 2)); // 空缓存设置较短过期时间（如 60 秒）
            return;
        }
        //组织数据
        std::unordered_map<std::string, long long> data;
        for(auto& it : videoList)
            data[it.video_id] = it.order_field;
        _redis.zadd(key, data.begin(), data.end());
        _redis.expire(key, std::chrono::seconds(_defaultCacheExpire));
    }

    std::shared_ptr<std::vector<std::string>> videoListOperational::getUserVideoListFromCache(const std::string& userId, int page, int size)
    {
        //从缓存中获取用户视频列表
        auto key = getUserVideoListCacheKey(userId);
        //判断字段是否存在
        std::shared_ptr<std::vector<std::string>> out = nullptr;
        if(_redis.exists(key))
        {
            //存在
            out = std::make_shared<std::vector<std::string>>();
            std::vector<std::string> temp;
            _redis.zrevrange(key, page * size, (page + 1) * size - 1, std::back_inserter(temp));
            for (auto& id : temp)
            {
                if (id != "__EMPTY_PLACEHOLDER__")
                {
                    out->push_back(std::move(id));
                }
            }
        }
        return out;
    }


    std::vector<std::string> videoListOperational::getCategoryVideoList(long long categoryId, int page, int size)
    {
        if((page*size) + size <= _defaultCacheSize)
        {
            //优先从缓存中获取
            auto curlist = getCategoryVideoListFromCache(categoryId, page, size); //返回智能指针
            if(curlist == nullptr)
            {
                //从数据库中读取，插入缓存
                auto dbList = getCategoryVideoListFromDb(categoryId, 0, _defaultCacheSize);
                insertCategoryVideoListToCache(categoryId, dbList);
                //筛选出需要的视频
                std::vector<std::string> out;
                for(size_t i = page * size; i < (size_t)(page * size + size) && i < dbList.size(); i++)
                    out.push_back(dbList[i].video_id);
                return out;
            }
            else
            {
                //此时读取成功，直接返回
                return *curlist;
            }
        }
        //不在区间内，直接读数据库
        std::vector<std::string> out;
        auto dbList = getCategoryVideoListFromDb(categoryId, page, size);
        //不需要筛选，直接添加到输出变量中
        for(auto& it : dbList)
            out.push_back(it.video_id);
        return out;
    }

    int32_t videoListOperational::getCategoryVideoTotal(long long categoryId)
    {
        odb::query<suiDataSql::TaggedVideoCount> q(odb::query<suiDataSql::TaggedVideoCount>::suiVideoTagMeta::tag_id == categoryId
                                        && odb::query<suiDataSql::TaggedVideoCount>::suiVideoMeta::video_status == suiDataSql::videoStatus::videoStatusApproved);
        auto ret = _mysql.query_one<suiDataSql::TaggedVideoCount>(q);
        return ret->count;
    }

    void videoListOperational::syncCategoryVideoList(long long categoryId, const std::string& videoId)
    {
        auto key = getVideoCategoryListCacheKey(categoryId);
        auto score = _redis.zscore(key, videoId);
        if(score)
        {
            //存在 直接进行同步
            return syncDeleteCache(key);
        }
        //视频不存在缓存中，判断缓存数量
        auto size = _redis.zcard(key);
        if(size < _defaultCacheSize) //进行同步
            return syncDeleteCache(key);
    }

    std::vector<suiDataSql::TaggedVideoItem> videoListOperational::getCategoryVideoListFromDb(long long categoryId, int page, int size)
    {
        //从数据库中读
        std::vector<suiDataSql::TaggedVideoItem> out;
        odb::query<suiDataSql::TaggedVideoItem> q(odb::query<suiDataSql::TaggedVideoItem>::suiVideoTagMeta::tag_id == categoryId
                                        && odb::query<suiDataSql::TaggedVideoItem>::suiVideoMeta::video_status == suiDataSql::videoStatus::videoStatusApproved);
        q += " " + suiDataSql::SqlPaginationUtil::buildOrderByClause("video_upload_time", true) + " " + suiDataSql::SqlPaginationUtil::buildPageClause(size, page);
        auto ret = _mysql.query<suiDataSql::TaggedVideoItem>(q);
        for(auto& it : ret)
        {
            out.push_back(suiDataSql::TaggedVideoItem());
            out.back().video_id = it.video_id;
            out.back().order_field = it.order_field;
        }
        return out;
    }

    void videoListOperational::insertCategoryVideoListToCache(long long categoryId, std::vector<suiDataSql::TaggedVideoItem>& videoList)
    {
        auto key = getVideoCategoryListCacheKey(categoryId);
        if (videoList.empty())
        {
            _redis.zadd(key, "__EMPTY_PLACEHOLDER__", 0);
            _redis.expire(key, std::chrono::seconds(_defaultCacheExpire / 2)); // 空缓存设置较短过期时间（如 60 秒）
            return;
        }
        std::unordered_map<std::string, long long> data;
        for(auto& it : videoList)
            data[it.video_id] = it.order_field;
        _redis.zadd(key, data.begin(), data.end());
        _redis.expire(key, std::chrono::seconds(_defaultCacheExpire));
    }

    std::shared_ptr<std::vector<std::string>> videoListOperational::getCategoryVideoListFromCache(long long categoryId, int page, int size)
    {
        auto key = getVideoCategoryListCacheKey(categoryId);
        std::shared_ptr<std::vector<std::string>> out = nullptr;
        if(_redis.exists(key))
        {
            out = std::make_shared<std::vector<std::string>>();
            std::vector<std::string> temp;
            _redis.zrevrange(key, page * size, (page + 1) * size - 1, std::back_inserter(temp));
            for (auto& id : temp)
            {
                if (id != "__EMPTY_PLACEHOLDER__")
                {
                    out->push_back(std::move(id));
                }
            }
        }
        return out;
    }

    std::vector<std::string> videoListOperational::getVideoStatusList(suiDataSql::videoStatus status, int page, int size)
    {
        //直接从数据库获取，不进行缓存
        std::vector<std::string> result;
        odb::query<suiDataSql::VideoIdList> q(odb::query<suiDataSql::VideoIdList>::video_status == status);
        q += " " + suiDataSql::SqlPaginationUtil::buildOrderByClause("video_upload_time", true) + " " + suiDataSql::SqlPaginationUtil::buildPageClause(size, page);
        auto ret = _mysql.query<suiDataSql::VideoIdList>(q);
        for(auto& it : ret)
            result.push_back(it.video_id);
        return result;
    }

    int32_t videoListOperational::getVideoStatusTotal(suiDataSql::videoStatus status)
    {
        odb::query<suiDataSql::suiVideoCountView> q(odb::query<suiDataSql::TaggedVideoCount>::suiVideoMeta::video_status == status);
        auto ret = _mysql.query_one<suiDataSql::suiVideoCountView>(q);
        return ret->count;
    }
    

    
}