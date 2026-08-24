#include "video_operation_meta.hpp"

namespace suiVideoOperation
{
    //前缀
    const std::string videoOperation::_cachePrefix = "video_meta_";
    //映射字段
    const std::string videoOperation::_map_video_id = "video_meta_id";
    const std::string videoOperation::_map_video_file_id = "video_file_id";
    const std::string videoOperation::_map_video_cover_file_id = "video_cover_file_id";
    const std::string videoOperation::_map_upload_user_id = "upload_user_id";
    const std::string videoOperation::_map_review_user_id = "review_user_id";
    const std::string videoOperation::_map_video_name = "video_name";
    const std::string videoOperation::_map_video_description = "video_description";
    const std::string videoOperation::_map_video_play_count = "video_play_count";
    const std::string videoOperation::_map_video_size = "video_size";
    const std::string videoOperation::_map_video_duration = "video_duration";
    const std::string videoOperation::_map_video_upload_time = "video_upload_time";
    const std::string videoOperation::_map_video_status = "video_status";
    //缓存过期时间
    const int videoOperation::_cache_expire = 60 * 60 * 2; //缓存过期时间，单位秒，2小时

    videoOperation::videoOperation(odb::database& db, sw::redis::Redis& redis, suiRemoveCache::RemoveCache::ptr dataSync)
        : _db(db), _redis(redis), _dataSync(dataSync)
    {

    }

    void videoOperation::insert(suiDataSql::suiVideoMeta& video)
    {
        insertToDb(video);
    }

    void videoOperation::remove(const std::string& videoId)
    {
        removeToDb(videoId);
        removeToCache(videoId);
    }

    suiDataSql::suiVideoMeta::ptr videoOperation::select(const std::string& videoId)
    {
        //优先搜索缓存
        auto ret =  selectToCache(videoId);
        if(ret)
            return ret;
        ret = selectToDb(videoId);
        if(ret)
        {
            //添加进缓存
            insertToCache(*ret);
        }
        return ret;
    }

    suiDataSql::suiVideoMeta::ptr videoOperation::selectToDbForce(const std::string& videoId)
    {
        return selectToDb(videoId);
    }

    void videoOperation::update(suiDataSql::suiVideoMeta& video)
    {
        //优先进行搜索
        auto ret = selectToDb(video.getVideoId());
        if(ret)
        {
            ret->setReviewUserId(video.getReviewUserId()); //审核用户
            ret->setVideoName(video.getVideoName()); //视频名称
            ret->setVideoDescription(video.getVideoDescription()); //视频描述
            ret->setVideoPlayCount(video.getVideoPlayCount()); //视频播放次数
            ret->setVideoStatus(video.getVideoStatus()); //视频状态
            updateToDb(*ret);
            removeToCache(ret->getVideoId());
        }
    }

    void videoOperation::updateToDbForce(suiDataSql::suiVideoMeta& video)
    {
        //强制直接使用数据库更新
        updateToDb(video);
    }

    suiDataSql::suiVideoMetaList::ptr videoOperation::selectByUserId(const std::string& userId, size_t page, size_t pageSize)
    {
        return selectByUserIdToDb(userId, page, pageSize);
    }

    void videoOperation::addPlayCount(const std::string& videoId, int playCountChange)
    {
        //不删除缓存，直接更新数据库，想要播放数据视频统计中获取，这里允许短期不同步
        addPlayCountToDb(videoId, playCountChange);
    }


    void videoOperation::insertToDb(suiDataSql::suiVideoMeta& video)
    {
        _db.persist(video);
    }

    suiDataSql::suiVideoMeta::ptr videoOperation::selectToDb(const std::string& videoId)
    {
        auto ret = suiDataSql::suiVideoMeta::ptr(_db.query_one<suiDataSql::suiVideoMeta>(odb::query<suiDataSql::suiVideoMeta>::video_id == videoId));
        return ret;
    }

    void videoOperation::updateToDb(suiDataSql::suiVideoMeta& video)
    {
        //本处默认从数据库获取后更新的字段
        _db.update(video);
    }

    void videoOperation::removeToDb(const std::string& videoId)
    {
        _db.erase_query<suiDataSql::suiVideoMeta>(odb::query<suiDataSql::suiVideoMeta>::video_id == videoId);
    }

    suiDataSql::suiVideoMetaList::ptr videoOperation::selectByUserIdToDb(const std::string& userId, size_t page, size_t pageSize)
    {
        auto q = odb::query<suiDataSql::suiVideoMeta>::upload_user_id == userId;
        //获取视频总数
        auto total = _db.query_one<suiDataSql::suiVideoCountView>(q);
        //新增条件
        q += " " + suiDataSql::SqlPaginationUtil::buildPageClause(pageSize, page);
        //获取视频列表
        auto ret = _db.query<suiDataSql::suiVideoMeta>(q);
        auto out = std::make_shared<suiDataSql::suiVideoMetaList>();
        for(auto& itemPtr : ret)
            out->list.push_back(itemPtr);
        out->total = total->count;
        return out;
    }

    void videoOperation::addPlayCountToDb(const std::string& videoId, int playCountChange)
    {
        auto ret = selectToDb(videoId);
        if(ret)
        {
            ret->setVideoPlayCount(ret->getVideoPlayCount() + playCountChange);
            updateToDb(*ret);
        }
    }

    void videoOperation::insertToCache(suiDataSql::suiVideoMeta& video)
    {
        std::unordered_map<std::string, std::string> _data;
        {
            _data[_map_video_id] = video.getVideoId();
            _data[_map_video_file_id] = video.getVideoFileId();
            _data[_map_video_cover_file_id] = video.getVideoCoverFileId();
            _data[_map_upload_user_id] = video.getUploadUserId();
            if(!video.getReviewUserId().null())
                _data[_map_review_user_id] = video.getReviewUserId().get();
            _data[_map_video_name] = video.getVideoName();
            if(!video.getVideoDescription().null())
                _data[_map_video_description] = video.getVideoDescription().get();
            _data[_map_video_play_count] = std::to_string(video.getVideoPlayCount());
            _data[_map_video_size] = std::to_string(video.getVideoSize());
            _data[_map_video_duration] = std::to_string(video.getVideoDuration());
            _data[_map_video_upload_time] = std::to_string(video.getVideoUploadTime());
            _data[_map_video_status] = suiDataSql::videoStatusUtil::toStringFromEnum(video.getVideoStatus());
        }
        //进行添加
        std::string key = getCacheKey(video.getVideoId());
        _redis.hmset(key, _data.begin(), _data.end());
        _redis.expire(key, std::chrono::seconds(_cache_expire));
    }

    suiDataSql::suiVideoMeta::ptr videoOperation::selectToCache(const std::string& videoId)
    {
        std::string key = getCacheKey(videoId);
        std::unordered_map<std::string, std::string> _data;
        _redis.hgetall(key, std::inserter(_data, _data.begin()));
        if(_data.empty())
            return nullptr;
        auto out = std::make_shared<suiDataSql::suiVideoMeta>();
        {
            //进行设置
            out->setVideoId(_data[_map_video_id]);
            out->setVideoFileId(_data[_map_video_file_id]);
            out->setVideoCoverFileId(_data[_map_video_cover_file_id]);
            out->setUploadUserId(_data[_map_upload_user_id]);
            out->setReviewUserId(_data[_map_review_user_id]);
            out->setVideoName(_data[_map_video_name]);
            if(_data.find(_map_video_description) != _data.end())
                out->setVideoDescription(_data[_map_video_description]);
            out->setVideoPlayCount(std::stoul(_data[_map_video_play_count]));
            out->setVideoSize(std::stoull(_data[_map_video_size]));
            out->setVideoDuration(std::stoull(_data[_map_video_duration]));
            out->setVideoUploadTime(std::stoull(_data[_map_video_upload_time]));
            out->setVideoStatus(suiDataSql::videoStatusUtil::getEnumFromString(_data[_map_video_status]));
        }
        return out;
    }

    void videoOperation::removeToCache(const std::string& videoId)
    {
        _dataSync->syncCache({getCacheKey(videoId)});
    }

    std::string videoOperation::getCacheKey(const std::string& videoId)
    {
        return _cachePrefix + videoId;
    }

    videoOperationDb::videoOperationDb(odb::database& db)
        :_db(db)
    {

    }

    suiDataSql::suiVideoMeta::ptr videoOperationDb::select(const std::string& videoId)
    {
        auto ret = suiDataSql::suiVideoMeta::ptr(_db.query_one<suiDataSql::suiVideoMeta>(odb::query<suiDataSql::suiVideoMeta>::video_id == videoId));
        return ret;
    }

    void videoOperationDb::update(suiDataSql::suiVideoMeta& video)
    {
        _db.update(video);
    }
}
