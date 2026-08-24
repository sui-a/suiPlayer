#include "videoServerData.hpp"

namespace suiVideoServer
{
    videoServerData::videoServerData(std::shared_ptr<odb::database> mysql,
                std::shared_ptr<sw::redis::Redis> redis, std::shared_ptr<suiRemoveCache::RemoveCache> cache_sync
            , suiVideoSubtitleTarget::videoSubtitleAsync::ptr videoSubtitleAsync
            , suiVideoSearch::videoSearch::ptr videoSearchHandle
            , suiCacheSync::CacheSyncClient::ptr fileRemove, suiCacheSync::CacheSyncClient::ptr transcode_operation_handle
            , suiVideoDataStatistics::VideoDataStatisticsAsync::ptr videoStaticsSync)
        : _mysql(mysql)
        , _redis(redis)
        , _cache_sync(cache_sync)
        , _videoSubtitleAsync(videoSubtitleAsync)
        , _videoSearchHandle(videoSearchHandle)
        , _fileRemove(fileRemove)
        , _transcode_operation_handle(transcode_operation_handle)
        , _videoStaticsSync(videoStaticsSync)
        
    {
        
    }

    void videoServerData::newVideo(const std::string& ssid, const suiApi::VideoInfo& videoInfo, int32_t& error_code, std::string& error_msg)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            {
                //提取当前上传用户id
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto curSession = sessionHandle.selectBySessionId(ssid);
                if(curSession == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(curSession->getUserId().null())
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_USER_NO_PERMISSION;
                    error_msg = "会话未登录";
                    return;
                }
                if(curSession->getUserId().get() != videoInfo.userid())
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_USER_MISMATCH;
                    error_msg = "上传用户与会话用户不匹配";
                    return;
                }
            }
            // {
            //     //验证用户有效性
            //     suiUserInformation::suiUserInformationOperation userInfoHandle(tx, rtx, _cache_sync);
            //     auto userInfo = userInfoHandle.getUserInfoById(videoInfo.userid());
            //     if(userInfo == nullptr)
            //     {
            //         error_code = suiErrorCodeDef::ERR_USER_SERVICE_GET_USER_INFO_FAILED;
            //         error_msg = "用户信息不存在";
            //         return;
            //     }
            // }
            {
                //构造视频信息，初始化为转码状态，并上传
                suiDataSql::suiVideoMeta videoMeta;
                videoMeta.setVideoId(createId());
                videoMeta.setVideoFileId(videoInfo.videofileid());
                videoMeta.setVideoCoverFileId(videoInfo.photofileid());
                videoMeta.setUploadUserId(videoInfo.userid());
                videoMeta.setVideoName(videoInfo.videotitle());
                if(!videoInfo.videodesc().empty())
                    videoMeta.setVideoDescription(videoInfo.videodesc());
                videoMeta.setVideoPlayCount(0);
                videoMeta.setVideoSize(videoInfo.videosize());
                videoMeta.setVideoDuration(videoInfo.videoduration());
                videoMeta.setVideoUploadTime(suiTime::suiTimeOperater::get_timestamp_sec());
                videoMeta.setVideoStatus(suiDataSql::videoStatus::videoStatusTranscoding);
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync);
                videoHandle.insert(videoMeta);
            }
            {
                //添加视频标签信息
                suiVideocatgoryTag::videocatgoryTag tagHandle(dbHandler);
                for(auto it : videoInfo.videotag())
                {
                    //判断标签是否存在
                    auto tag = tagHandle.selectTag(it);
                    if(tag != nullptr) //不为空插入视频标签关联，为空的过滤掉
                        tagHandle.addVideoTag(videoInfo.videoid(), tag->getTagId());
                }
            }
            {
                //发布转码消息
                _transcode_operation_handle->syncCache(videoInfo.videoid());
            }
            
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::deleteVideo(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userId;
            {
                //判断请求信息是否为空
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto curSession = sessionHandle.selectBySessionId(ssid);
                if(curSession == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(curSession->getUserId().null())
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_USER_NO_PERMISSION;
                    error_msg = "上传用户权限不足";
                    return;
                }
                userId = curSession->getUserId().get();
            }
            std::string videoUrl;
            std::string videoPhotoUrl;
            {
                //获取视频信息
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync);
                auto video = videoHandle.select(videoId); //优先从缓存中获取视频信息
                if(video == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_VIDEO_SERVICE_VIDEO_NOT_FOUND;
                    error_msg = "视频不存在";
                    return;
                }
                if(video->getUploadUserId() != userId)
                {
                    //视频上传用户与会话用户不匹配
                    error_code = suiErrorCodeDef::ERR_VIDEO_SERVICE_UPLOAD_USER_MISMATCH;
                    error_msg = "视频上传用户与会话用户不匹配";
                    return;
                }
                videoUrl = video->getVideoFileId();
                videoPhotoUrl = video->getVideoCoverFileId();
            }
            {
                //移除视频元信息
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync);
                videoHandle.remove(videoId);
            }
            {
                //移除视频分类标签信息
                suiVideocatgoryTag::videocatgoryTag tagHandle(dbHandler);
                tagHandle.removeVideoTag(videoId);
            }
            {
                //移除弹幕信息
                suiVideoSubtitleTarget::videoSubtitleTarget subtitleHandle(tx, rtx, _videoSubtitleAsync);
                subtitleHandle.removeByVideoid(videoId);
            }
            {
                //移除点赞信息
                suiVideoLike::videoLike likeHandle(dbHandler);
                likeHandle.removeByVideoId(videoId);
            }
            {
                //移除es中的视频信息
                _videoSearchHandle->removeVideo(videoId);
            }
            {
                //异步删除文件信息
                {
                    suiApi::DeleteFileMsg msg;
                    msg.add_fileid(videoUrl);
                    msg.add_fileid(videoPhotoUrl);
                    _fileRemove->syncCache(msg.SerializeAsString());
                }
            }
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::videoLikeJudge(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg, bool& isLike)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        isLike = false;
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userId;
            {
                //判断会话有效性
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto curSession = sessionHandle.selectBySessionId(ssid);
                if(curSession == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(curSession->getUserId().null())
                {
                    //无登录会话，直接当作没有点赞
                    isLike = false;
                    return;
                }
                //有登录会话，记录id
                userId = curSession->getUserId().get();
            }
            {
                //判断是否点赞
                suiVideoLike::videoLike likeHandle(dbHandler);
                isLike = likeHandle.isLike(userId, videoId);
            }

            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::videoLikeOperation(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userId;
            {
                //判断会话有效性
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto curSession = sessionHandle.selectBySessionId(ssid);
                if(curSession == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(curSession->getUserId().null())
                {
                    //无登录会话，直接当作没有点赞
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED;
                    error_msg = "用户未登录";
                    return;
                }
                //有登录会话，记录id
                userId = curSession->getUserId().get();
            }
            std::string videoUserId;
            {
                //验证视频有效性
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync);
                auto video = videoHandle.select(videoId); //优先从缓存中获取视频信息
                if(video == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_VIDEO_SERVICE_VIDEO_NOT_FOUND;
                    error_msg = "视频不存在";
                    return;
                }
                videoUserId = video->getUploadUserId();
            }
            bool isLike = false;
            {
                //判断点赞，然后进行反转
                suiVideoLike::videoLike likeHandle(dbHandler);
                isLike = likeHandle.isLike(userId, videoId);
                if(isLike)
                {
                    //已点赞，取消点赞
                    likeHandle.remove(videoId, userId);
                }
                else
                {
                    //未点赞，点赞
                    likeHandle.insert(videoId, userId);
                }
            }
            {
                //更新缓存中的视频点赞量
                suiVideoDataStatistics::VideoDataStatistics videoStaticsHandle(tx, rtx, _cache_sync, _videoStaticsSync);
                videoStaticsHandle.setVideoLikeCountByChange(videoId, (isLike? 1 : -1));
            }
            {
                //更新缓存中用户点赞量
                suiUserStatics::suiStatics userStaticsHandle(dbHandler, reHandler, _cache_sync, rtx);
                userStaticsHandle.setVideoLikeCountByChange(videoUserId, (isLike? 1 : -1));
            }
            rtx.exec();
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::videoPlayOperation(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            // {
            //     //判断会话有效性
            //     suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
            //     auto curSession = sessionHandle.selectBySessionId(ssid);
            //     if(curSession == nullptr)
            //     {
            //         error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
            //         error_msg = "会话无效";
            //         return;
            //     }
            // }
            std::string videoUserId;
            {
                //判断视频有效性
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync);
                auto video = videoHandle.select(videoId); //优先从缓存中获取视频信息
                if(video == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_VIDEO_SERVICE_VIDEO_NOT_FOUND;
                    error_msg = "视频不存在";
                    return;
                }
                videoUserId = video->getUploadUserId();
            }
            {
                //添加视频播放量
                suiVideoDataStatistics::VideoDataStatistics videoStaticsHandle(tx, rtx, _cache_sync, _videoStaticsSync);
                videoStaticsHandle.setVideoPlayCountByChange(videoId, 1);
            }
            {
                //新增用户统计数据播放量
                suiUserStatics::suiStatics userStaticsHandle(dbHandler, reHandler, _cache_sync, rtx);
                userStaticsHandle.setVideoPlayCountByChange(videoUserId, 1);
            }
            rtx.exec();
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::addVideoSubtitle(const std::string& ssid, const suiApi::VideoSubtitleInfo& videoSubtitleInfo, int32_t& error_code, std::string& error_msg)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            {
                //会话用户与请求用户是否匹配
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto curSession = sessionHandle.selectBySessionId(ssid);
                if(curSession == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(curSession->getUserId().null())
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_USER_NO_PERMISSION;
                    error_msg = "会话未登录";
                    return;
                }
                if(curSession->getUserId().get() != videoSubtitleInfo.userid())
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_USER_MISMATCH;
                    error_msg = "用户id不匹配";
                    return;
                }
            }
            {
                //判断视频有效性
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync);
                auto video = videoHandle.select(videoSubtitleInfo.videoid()); //优先从缓存中获取视频信息
                if(video == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_VIDEO_SERVICE_VIDEO_NOT_FOUND;
                    error_msg = "视频不存在";
                    return;
                }
            }
            {
                //直接新增弹幕
                suiDataSql::suiVideoSubtitleTarget::ptr data = std::make_shared<suiDataSql::suiVideoSubtitleTarget>();
                data->setVideoId(videoSubtitleInfo.videoid());
                data->setUserId(videoSubtitleInfo.userid());
                data->setBulletchatId(createId());
                data->setBulletchat(videoSubtitleInfo.subtitle());
                data->setSendByVideoTime(videoSubtitleInfo.subtitletime());
                data->setCreateTime();
                suiVideoSubtitleTarget::videoSubtitleTarget subtitleTarget(tx, rtx, _videoSubtitleAsync);
                subtitleTarget.insert(data);
            }
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::getVideoSubtitle(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg, suiApi::getVideoSubtitleResult& result)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            // {
            //     //判断请求信息是否为空
            //     suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
            //     auto curSession = sessionHandle.selectBySessionId(ssid);
            //     if(curSession == nullptr)
            //     {
            //         error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
            //         error_msg = "会话无效";
            //         return;
            //     }
            // }
            {
                //开始获取弹幕列表
                suiVideoSubtitleTarget::videoSubtitleTarget subtitleTarget(tx, rtx, _videoSubtitleAsync);
                auto retList = subtitleTarget.select(videoId);
                for(auto& it : retList->list)
                {
                    suiApi::VideoSubtitleInfo* curInfo = result.add_videosubtitlelist();
                    curInfo->set_subtitleid(it.getBulletchatId());
                    curInfo->set_videoid(videoId);
                    curInfo->set_userid(it.getUserId());
                    curInfo->set_subtitle(it.getBulletchat());
                    curInfo->set_subtitletime(it.getSendByVideoTime());
                }
            }
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::checkVideo(const std::string& ssid, const std::string& videoId, bool isPass, int32_t& error_code, std::string& error_msg)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string checkUserId;
            {
                //验证会话
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto curSession = sessionHandle.selectBySessionId(ssid);
                if(curSession == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(curSession->getUserId().null())
                {
                    //无登录会话，直接当作没有点赞
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED;
                    error_msg = "用户未登录";
                    return;
                }
                //有登录会话，记录id
                checkUserId = curSession->getUserId().get();
            }
            {
                //验证是否为管理员
                suiUserIdentityRole::UserIdentityRole userRoleHandle(dbHandler, reHandler, _cache_sync);
                auto roleRet = userRoleHandle.select(checkUserId);
                if(roleRet->getIdentityType() != suiDataSql::identityType::identityTypeAdmin)
                {
                    //只判断是否为B端用户，只要B端用户，无论权限大小都可以操作
                    //直接返回错误
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID;
                    error_msg = "用户权限不足";
                    return;
                }
            }
            suiDataSql::suiVideoMeta::ptr videoRet = nullptr;
            {
                //验证视频是否存在，并修改状态
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync);
                videoRet = videoHandle.selectToDbForce(videoId); //强制从数据库中获取
                if(videoRet == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_VIDEO_SERVICE_VIDEO_NOT_FOUND;
                    error_msg = "视频不存在";
                    return;
                }
                if(videoRet->getVideoStatus() != suiDataSql::videoStatus::videoStatusPendingReview)
                {
                    error_code = suiErrorCodeDef::ERR_VIDEO_SERVICE_VIDEO_STATUS_INVALID;
                    error_msg = "视频状态异常";
                    return;
                }
                //直接修改
                videoRet->setVideoStatus(isPass ? suiDataSql::videoStatus::videoStatusApproved : suiDataSql::videoStatus::videoStatusReject);
                videoRet->setReviewUserId(checkUserId);
                videoHandle.updateToDbForce(*videoRet);
            }
            {
                //如果审核通过，更新es
                if(videoRet->getVideoStatus() == suiDataSql::videoStatus::videoStatusApproved)
                {
                    //更新es
                    _videoSearchHandle->insertVideo(videoRet->getVideoId(), suiDataSql::videoStatus::videoStatusApproved, videoRet->getVideoName()
                                                , (videoRet->getVideoDescription().null() ? "" : videoRet->getVideoDescription().get()), videoRet->getVideoUploadTime());
                }
            }
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::onlineVideo(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string checkUserId;
            {
                //验证会话
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto curSession = sessionHandle.selectBySessionId(ssid);
                if(curSession == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(curSession->getUserId().null())
                {
                    //无登录会话，直接当作没有点赞
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED;
                    error_msg = "用户未登录";
                    return;
                }
                //有登录会话，记录id
                checkUserId = curSession->getUserId().get();
            }
            {
                //验证是否为管理员
                suiUserIdentityRole::UserIdentityRole userRoleHandle(dbHandler, reHandler, _cache_sync);
                auto roleRet = userRoleHandle.select(checkUserId);
                if(roleRet->getIdentityType() != suiDataSql::identityType::identityTypeAdmin)
                {
                    //只判断是否为B端用户，只要B端用户，无论权限大小都可以操作
                    //直接返回错误
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID;
                    error_msg = "用户权限不足";
                    return;
                }
            }
            suiDataSql::suiVideoMeta::ptr videoRet = nullptr;
            {
                //获取视频信息
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync);
                videoRet = videoHandle.selectToDbForce(videoId); //强制从数据库中获取
                if(videoRet == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_VIDEO_SERVICE_VIDEO_NOT_FOUND;
                    error_msg = "视频不存在";
                    return;
                }
                if(videoRet->getVideoStatus() != suiDataSql::videoStatus::videoStatusRemove)
                {
                    //只有下架状态才可以上架，否则走审核流程
                    error_code = suiErrorCodeDef::ERR_VIDEO_SERVICE_VIDEO_STATUS_INVALID;
                    error_msg = "视频状态异常";
                    return;
                }
                //直接修改
                videoRet->setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoHandle.updateToDbForce(*videoRet);
            }
            {
                //更新es
                _videoSearchHandle->updateVideo(videoRet->getVideoId(), suiDataSql::videoStatus::videoStatusApproved);
            }
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::offlineVideo(const std::string& ssid, const std::string& videoId, int32_t& error_code, std::string& error_msg)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string checkUserId;
            {
                //验证会话
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto curSession = sessionHandle.selectBySessionId(ssid);
                if(curSession == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(curSession->getUserId().null())
                {
                    //无登录会话，直接当作没有点赞
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED;
                    error_msg = "用户未登录";
                    return;
                }
                //有登录会话，记录id
                checkUserId = curSession->getUserId().get();
            }
            {
                //验证是否为管理员
                suiUserIdentityRole::UserIdentityRole userRoleHandle(dbHandler, reHandler, _cache_sync);
                auto roleRet = userRoleHandle.select(checkUserId);
                if(roleRet == nullptr || roleRet->getIdentityType() != suiDataSql::identityType::identityTypeAdmin)
                {
                    //只判断是否为B端用户，只要B端用户，无论权限大小都可以操作
                    //直接返回错误
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID;
                    error_msg = "用户权限不足";
                    return;
                }
            }
            suiDataSql::suiVideoMeta::ptr videoRet = nullptr;
            {
                //获取视频信息
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync);
                videoRet = videoHandle.selectToDbForce(videoId); //强制从数据库中获取
                if(videoRet == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_VIDEO_SERVICE_VIDEO_NOT_FOUND;
                    error_msg = "视频不存在";
                    return;
                }
                if(videoRet->getVideoStatus() != suiDataSql::videoStatus::videoStatusApproved)
                {
                    //只有已审核状态才可以下架，否则走审核流程，或者直接打死视频
                    error_code = suiErrorCodeDef::ERR_VIDEO_SERVICE_VIDEO_STATUS_INVALID;
                    error_msg = "视频状态异常";
                    return;
                }
                //直接修改
                videoRet->setVideoStatus(suiDataSql::videoStatus::videoStatusRemove);
                videoHandle.updateToDbForce(*videoRet);
            }
            {
                //更新es
                _videoSearchHandle->updateVideo(videoRet->getVideoId(), suiDataSql::videoStatus::videoStatusRemove);
            }
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::getUserVideoList(const std::string& ssid, const std::string& targetUserId, int32_t pageIndex, int32_t pageCount
            , int32_t& error_code, std::string& error_msg, suiApi::getUserVideoListResult& result)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            // {
            //     //验证会话
            //     suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
            //     auto curSession = sessionHandle.selectBySessionId(ssid);
            //     if(curSession == nullptr)
            //     {
            //         error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
            //         error_msg = "会话无效";
            //         return;
            //     }
            // }
            suiDataSql::suiUsrMeta::ptr userInfo = nullptr;
            {
                //获取用户信息
                suiUserInformation::suiUserInformationOperation userInfoHandle(tx, rtx, _cache_sync);
                userInfo = userInfoHandle.getUserInfoById(targetUserId);
                if(userInfo == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND;
                    error_msg = "用户不存在";
                    return;
                }
                if(userInfo->getUserStatus() != suiDataSql::userStatus::userStatusEnable)
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_EMAIL_DISABLE;
                    error_msg = "用户已被禁用";
                    return;
                }
            }
            std::vector<std::string> videoIdList;
            size_t videoTotal = 0;
            {
                //获取用户视频列表
                suiVideoListOperational::videoListOperational videoListOperational(tx, rtx, _cache_sync);
                videoIdList = videoListOperational.getUserVideoMainList(targetUserId, pageIndex, pageCount);
                videoTotal = videoListOperational.getUserVideoTotal(targetUserId);
            }
            {
                //加载信息
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync); //视频信息获取句柄
                suiVideocatgoryTag::videocatgoryTag videoTagHandle(dbHandler); //视频分类标签获取句柄
                suiVideoLike::videoLike videoLikeHandle(dbHandler); //视频点赞获取句柄
                suiUserInformation::suiUserInformationOperation userInfoHandle(tx, rtx, _cache_sync); //用户信息获取句柄
                for(auto& it : videoIdList)
                {
                    auto videoMeta = videoHandle.select(it);
                    auto videoTagList = videoTagHandle.selectVideoTag(it);
                    if(videoMeta->getReviewUserId().null())
                        continue;
                    auto checkerInfo = userInfoHandle.getUserInfoById(videoMeta->getReviewUserId().get());
                    if(checkerInfo == nullptr || videoMeta == nullptr)
                        continue;
                    size_t likeCount = videoLikeHandle.countByVideoId(it);
                    auto ret = result.add_videolist();
                    ret->set_videoid(it);
                    ret->set_userid(targetUserId);
                    if(!userInfo->getHeadImageFileId().null())
                        ret->set_useravatarid(userInfo->getHeadImageFileId().get()); 
                    ret->set_nickname(userInfo->getUserName());
                    for(auto& tag : videoTagList)
                        ret->add_videotag(tag);
                    ret->set_videofileid(videoMeta->getVideoFileId());
                    ret->set_photofileid(videoMeta->getVideoCoverFileId()); //封面一定存在
                    ret->set_likecount(likeCount);
                    ret->set_playcount(videoMeta->getVideoPlayCount());
                    ret->set_videosize(videoMeta->getVideoSize());
                    if(!videoMeta->getVideoDescription().null())
                        ret->set_videodesc(videoMeta->getVideoDescription().get());
                    ret->set_videotitle(videoMeta->getVideoName());
                    ret->set_videoduration(videoMeta->getVideoDuration());
                    ret->set_videouptime(videoMeta->getVideoUploadTime());
                    ret->set_checkerid(videoMeta->getReviewUserId().get());
                    if(!checkerInfo->getHeadImageFileId().null())
                        ret->set_checkeravatar(checkerInfo->getHeadImageFileId().get());
                    ret->set_status(suiApi::videoStatus::videoStatusApproved); //这里获得的视频统一是被认可的上架状态
                }
                //设置视频总数
                result.set_videototal(videoTotal);
            }
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::getVideoListByStatus(const std::string& ssid, suiApi::videoStatus status, int32_t pageIndex, int32_t pageCount
            , int32_t& error_code, std::string& error_msg, suiApi::getStatusVideoListResult& result)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userId;
            {
                //验证会话
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto curSession = sessionHandle.selectBySessionId(ssid);
                if(curSession == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(curSession->getUserId().null())
                {
                    //无登录会话，直接当作没有点赞
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED;
                    error_msg = "用户未登录";
                    return;
                }
                //有登录会话，记录id
                userId = curSession->getUserId().get();
            }
            {
                //验证是否为管理员
                suiUserIdentityRole::UserIdentityRole userRoleHandle(dbHandler, reHandler, _cache_sync);
                auto roleRet = userRoleHandle.select(userId);
                if(roleRet->getIdentityType() != suiDataSql::identityType::identityTypeAdmin)
                {
                    //只判断是否为B端用户，只要B端用户，无论权限大小都可以操作
                    //直接返回错误
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID;
                    error_msg = "用户权限不足";
                    return;
                }
            }
            std::vector<std::string> videoIdList;
            size_t videoTotal = 0;
            {
                //获取状态视频列表与总数
                suiVideoListOperational::videoListOperational videoListOperational(tx, rtx, _cache_sync);
                videoIdList = videoListOperational.getVideoStatusList(videoStatusMap(status), pageIndex, pageCount);
                videoTotal = videoListOperational.getVideoStatusTotal(videoStatusMap(status));
            }
            {
                //加载信息
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync); //视频信息获取句柄
                suiVideocatgoryTag::videocatgoryTag videoTagHandle(dbHandler); //视频分类标签获取句柄
                suiVideoLike::videoLike videoLikeHandle(dbHandler); //视频点赞获取句柄
                suiUserInformation::suiUserInformationOperation userInfoHandle(tx, rtx, _cache_sync); //用户信息获取句柄
                for(auto& it : videoIdList)
                {
                    auto videoMeta = videoHandle.select(it); //视频信息
                    auto videoTagList = videoTagHandle.selectVideoTag(it); //获取视频标签
                    if(videoMeta == nullptr)
                        continue;
                    auto videoUserInfo = userInfoHandle.getUserInfoById(videoMeta->getUploadUserId()); //获取上传者用户信息
                    if(videoUserInfo == nullptr)
                        continue;
                    size_t likeCount = videoLikeHandle.countByVideoId(it); //视频被点赞总数
                    suiDataSql::suiUsrMeta::ptr checkerInfo = nullptr; //审核该视频管理员用户信息
                    if(!videoMeta->getReviewUserId().null()) //如果有被审核则查询
                        checkerInfo = userInfoHandle.getUserInfoById(videoMeta->getReviewUserId().get());

                    {
                        auto ret = result.add_videolist();
                        ret->set_videoid(it); //设置视频id
                        ret->set_userid(videoMeta->getUploadUserId()); //设置视频上传用户id
                        if(!videoUserInfo->getHeadImageFileId().null()) //如果用户头像存在
                            ret->set_useravatarid(videoUserInfo->getHeadImageFileId().get()); 
                        ret->set_nickname(videoUserInfo->getUserName()); //设置用户昵称
                        for(auto& tag : videoTagList)
                            ret->add_videotag(tag); //添加视频标签
                        ret->set_videofileid(videoMeta->getVideoFileId()); //设置视频文件id
                        ret->set_photofileid(videoMeta->getVideoCoverFileId()); //封面一定存在
                        ret->set_likecount(likeCount); //设置视频被点赞总数
                        ret->set_playcount(videoMeta->getVideoPlayCount()); //设置视频播放次数
                        ret->set_videosize(videoMeta->getVideoSize()); //设置视频大小
                        if(!videoMeta->getVideoDescription().null())
                            ret->set_videodesc(videoMeta->getVideoDescription().get()); //如果存在则设置视频描述
                        ret->set_videotitle(videoMeta->getVideoName()); //设置视频名称
                        ret->set_videoduration(videoMeta->getVideoDuration()); //设置视频时长
                        ret->set_videouptime(videoMeta->getVideoUploadTime()); //设置视频上传时间
                        if(checkerInfo) //如果审核者存在，设置审核者id
                            ret->set_checkerid(checkerInfo->getUserId());
                        if(checkerInfo && !checkerInfo->getHeadImageFileId().null()) //如果审核者头像存在
                            ret->set_checkeravatar(checkerInfo->getHeadImageFileId().get());
                        ret->set_status(status);
                    }
                }
                //设置视频总数
                result.set_totalcount(videoTotal);
            }

            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::getMainVideoList(const std::string& ssid, int32_t pageIndex, int32_t pageCount
            , int32_t& error_code, std::string& error_msg, suiApi::getHomeRandomVideoListResult& result)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            // {
            //     //验证会话id的有效性
            //     suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
            //     auto curSession = sessionHandle.selectBySessionId(ssid);
            //     if(curSession == nullptr)
            //     {
            //         error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
            //         error_msg = "会话无效";
            //         return;
            //     }
            // }
            std::vector<std::string> videoIdList;
            {
                //获取状态视频列表与总数
                suiVideoListOperational::videoListOperational videoListOperational(tx, rtx, _cache_sync);
                videoIdList = videoListOperational.getVideoMainList(pageIndex, pageCount);
            }
            {
                //加载视频信息
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync); //视频信息获取句柄
                suiVideocatgoryTag::videocatgoryTag videoTagHandle(dbHandler); //视频分类标签获取句柄
                suiVideoLike::videoLike videoLikeHandle(dbHandler); //视频点赞获取句柄
                suiUserInformation::suiUserInformationOperation userInfoHandle(tx, rtx, _cache_sync); //用户信息获取句柄
                for(auto& it : videoIdList)
                {
                    auto videoMeta = videoHandle.select(it); //视频信息
                    auto videoTagList = videoTagHandle.selectVideoTag(it); //获取视频标签
                    if(videoMeta == nullptr)
                        continue;
                    auto videoUserInfo = userInfoHandle.getUserInfoById(videoMeta->getUploadUserId()); //获取上传者用户信息
                    if(videoUserInfo == nullptr)
                        continue;
                    size_t likeCount = videoLikeHandle.countByVideoId(it); //视频被点赞总数
                    suiDataSql::suiUsrMeta::ptr checkerInfo = nullptr; //审核该视频管理员用户信息
                    if(!videoMeta->getReviewUserId().null()) //如果有被审核则查询
                        checkerInfo = userInfoHandle.getUserInfoById(videoMeta->getReviewUserId().get());

                    {
                        auto ret = result.add_videolist();
                        ret->set_videoid(it); //设置视频id
                        ret->set_userid(videoMeta->getUploadUserId()); //设置视频上传用户id
                        if(!videoUserInfo->getHeadImageFileId().null()) //如果用户头像存在
                            ret->set_useravatarid(videoUserInfo->getHeadImageFileId().get()); 
                        ret->set_nickname(videoUserInfo->getUserName()); //设置用户昵称
                        for(auto& tag : videoTagList)
                            ret->add_videotag(tag); //添加视频标签
                        ret->set_videofileid(videoMeta->getVideoFileId()); //设置视频文件id
                        ret->set_photofileid(videoMeta->getVideoCoverFileId()); //封面一定存在
                        ret->set_likecount(likeCount); //设置视频被点赞总数
                        ret->set_playcount(videoMeta->getVideoPlayCount()); //设置视频播放次数
                        ret->set_videosize(videoMeta->getVideoSize()); //设置视频大小
                        if(!videoMeta->getVideoDescription().null())
                            ret->set_videodesc(videoMeta->getVideoDescription().get()); //如果存在则设置视频描述
                        ret->set_videotitle(videoMeta->getVideoName()); //设置视频名称
                        ret->set_videoduration(videoMeta->getVideoDuration()); //设置视频时长
                        ret->set_videouptime(videoMeta->getVideoUploadTime()); //设置视频上传时间
                        if(checkerInfo) //如果审核者存在，设置审核者id
                            ret->set_checkerid(checkerInfo->getUserId());
                        if(checkerInfo && !checkerInfo->getHeadImageFileId().null()) //如果审核者头像存在
                            ret->set_checkeravatar(checkerInfo->getHeadImageFileId().get());
                        ret->set_status(suiApi::videoStatus::videoStatusApproved); //这里获得的视频统一是被认可的上架状态
                    }
                }
            }
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::getVideoListByTag(const std::string& ssid, int32_t tagid, int32_t pageIndex, int32_t pageCount
            , int32_t& error_code, std::string& error_msg, suiApi::getTagVideoListResult& result)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            // {
            //     //验证会话id的有效性
            //     suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
            //     auto curSession = sessionHandle.selectBySessionId(ssid);
            //     if(curSession == nullptr)
            //     {
            //         error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
            //         error_msg = "会话无效";
            //         return;
            //     }
            // }
            std::vector<std::string> videoIdList;
            size_t totalCount = 0;
            {
                //获取状态视频列表与总数
                suiVideoListOperational::videoListOperational videoListOperational(tx, rtx, _cache_sync);
                videoIdList = videoListOperational.getCategoryVideoList(tagid, pageIndex, pageCount);
                totalCount = videoListOperational.getCategoryVideoTotal(tagid);
            }
            {
                //加载视频信息
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync); //视频信息获取句柄
                suiVideocatgoryTag::videocatgoryTag videoTagHandle(dbHandler); //视频分类标签获取句柄
                suiVideoLike::videoLike videoLikeHandle(dbHandler); //视频点赞获取句柄
                suiUserInformation::suiUserInformationOperation userInfoHandle(tx, rtx, _cache_sync); //用户信息获取句柄
                for(auto& it : videoIdList)
                {
                    auto videoMeta = videoHandle.select(it); //视频信息
                    auto videoTagList = videoTagHandle.selectVideoTag(it); //获取视频标签
                    if(videoMeta == nullptr)
                        continue;
                    auto videoUserInfo = userInfoHandle.getUserInfoById(videoMeta->getUploadUserId()); //获取上传者用户信息
                    if(videoUserInfo == nullptr)
                        continue;
                    size_t likeCount = videoLikeHandle.countByVideoId(it); //视频被点赞总数
                    suiDataSql::suiUsrMeta::ptr checkerInfo = nullptr; //审核该视频管理员用户信息
                    if(!videoMeta->getReviewUserId().null()) //如果有被审核则查询
                        checkerInfo = userInfoHandle.getUserInfoById(videoMeta->getReviewUserId().get());
                    {
                        auto ret = result.add_videolist();
                        ret->set_videoid(it); //设置视频id
                        ret->set_userid(videoMeta->getUploadUserId()); //设置视频上传用户id
                        if(!videoUserInfo->getHeadImageFileId().null()) //如果用户头像存在
                            ret->set_useravatarid(videoUserInfo->getHeadImageFileId().get()); 
                        ret->set_nickname(videoUserInfo->getUserName()); //设置用户昵称
                        for(auto& tag : videoTagList)
                            ret->add_videotag(tag); //添加视频标签
                        ret->set_videofileid(videoMeta->getVideoFileId()); //设置视频文件id
                        ret->set_photofileid(videoMeta->getVideoCoverFileId()); //封面一定存在
                        ret->set_likecount(likeCount); //设置视频被点赞总数
                        ret->set_playcount(videoMeta->getVideoPlayCount()); //设置视频播放次数
                        ret->set_videosize(videoMeta->getVideoSize()); //设置视频大小
                        if(!videoMeta->getVideoDescription().null())
                            ret->set_videodesc(videoMeta->getVideoDescription().get()); //如果存在则设置视频描述
                        ret->set_videotitle(videoMeta->getVideoName()); //设置视频名称
                        ret->set_videoduration(videoMeta->getVideoDuration()); //设置视频时长
                        ret->set_videouptime(videoMeta->getVideoUploadTime()); //设置视频上传时间
                        if(checkerInfo) //如果审核者存在，设置审核者id
                            ret->set_checkerid(checkerInfo->getUserId());
                        if(checkerInfo && !checkerInfo->getHeadImageFileId().null()) //如果审核者头像存在
                            ret->set_checkeravatar(checkerInfo->getHeadImageFileId().get());
                        ret->set_status(suiApi::videoStatus::videoStatusApproved); //这里获得的视频统一是被认可的上架状态
                    }
                }
                result.set_totalcount(totalCount); //设置总视频数
            }

            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::getVideoListBySearch(const std::string& ssid, const std::string& keyword, int32_t pageIndex, int32_t pageCount
            , int32_t& error_code, std::string& error_msg, suiApi::searchVideoListResult& result)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            // {
            //     //验证会话id的有效性
            //     suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
            //     auto curSession = sessionHandle.selectBySessionId(ssid);
            //     if(curSession == nullptr)
            //     {
            //         error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
            //         error_msg = "会话无效";
            //         return;
            //     }
            // }
            std::vector<std::string> videoIdList;
            size_t totalCount = 0;
            {
                //获取视频id
                videoIdList = _videoSearchHandle->searchVideo(keyword, pageIndex, pageCount, totalCount);
            }
            {
                //加载视频信息
                suiVideoOperation::videoOperation videoHandle(dbHandler, reHandler, _cache_sync); //视频信息获取句柄
                suiVideocatgoryTag::videocatgoryTag videoTagHandle(dbHandler); //视频分类标签获取句柄
                suiVideoLike::videoLike videoLikeHandle(dbHandler); //视频点赞获取句柄
                suiUserInformation::suiUserInformationOperation userInfoHandle(tx, rtx, _cache_sync); //用户信息获取句柄
                for(auto& it : videoIdList)
                {
                    auto videoMeta = videoHandle.select(it); //视频信息
                    auto videoTagList = videoTagHandle.selectVideoTag(it); //获取视频标签
                    if(videoMeta == nullptr)
                        continue;
                    auto videoUserInfo = userInfoHandle.getUserInfoById(videoMeta->getUploadUserId()); //获取上传者用户信息
                    if(videoUserInfo == nullptr)
                        continue;
                    size_t likeCount = videoLikeHandle.countByVideoId(it); //视频被点赞总数
                    suiDataSql::suiUsrMeta::ptr checkerInfo = nullptr; //审核该视频管理员用户信息
                    if(!videoMeta->getReviewUserId().null()) //如果有被审核则查询
                        checkerInfo = userInfoHandle.getUserInfoById(videoMeta->getReviewUserId().get());
                    {
                        auto ret = result.add_videolist();
                        ret->set_videoid(it); //设置视频id
                        ret->set_userid(videoMeta->getUploadUserId()); //设置视频上传用户id
                        if(!videoUserInfo->getHeadImageFileId().null()) //如果用户头像存在
                            ret->set_useravatarid(videoUserInfo->getHeadImageFileId().get()); 
                        ret->set_nickname(videoUserInfo->getUserName()); //设置用户昵称
                        for(auto& tag : videoTagList)
                            ret->add_videotag(tag); //添加视频标签
                        ret->set_videofileid(videoMeta->getVideoFileId()); //设置视频文件id
                        ret->set_photofileid(videoMeta->getVideoCoverFileId()); //封面一定存在
                        ret->set_likecount(likeCount); //设置视频被点赞总数
                        ret->set_playcount(videoMeta->getVideoPlayCount()); //设置视频播放次数
                        ret->set_videosize(videoMeta->getVideoSize()); //设置视频大小
                        if(!videoMeta->getVideoDescription().null())
                            ret->set_videodesc(videoMeta->getVideoDescription().get()); //如果存在则设置视频描述
                        ret->set_videotitle(videoMeta->getVideoName()); //设置视频名称
                        ret->set_videoduration(videoMeta->getVideoDuration()); //设置视频时长
                        ret->set_videouptime(videoMeta->getVideoUploadTime()); //设置视频上传时间
                        if(checkerInfo) //如果审核者存在，设置审核者id
                            ret->set_checkerid(checkerInfo->getUserId());
                        if(checkerInfo && !checkerInfo->getHeadImageFileId().null()) //如果审核者头像存在
                            ret->set_checkeravatar(checkerInfo->getHeadImageFileId().get());
                        ret->set_status(suiApi::videoStatus::videoStatusApproved); //这里获得的视频统一是被认可的上架状态
                    }
                }
                result.set_totalcount(totalCount); //设置总视频数
            }
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        //执行到这里由于异常退出，导致事务回滚，直接返回失败
        //异常由于服务端异常，返回未知错误码
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }

    void videoServerData::getTagList(int32_t& error_code, std::string& error_msg, suiApi::getTagInfoRsp& result)
    {
        error_code = suiErrorCodeDef::SUCCESS; //默认成功
        try
        {
            odb::transaction tx(_mysql->begin());
            auto& dbHandler = tx.database();
            {
                //查询所有标签
                suiVideocatgoryTag::videocatgoryTag videoTagHandle(dbHandler);
                auto tagList = videoTagHandle.selectAllTag();
                for(auto& tag : tagList.list)
                {
                    auto data = result.add_result();
                    data->set_videotagid(tag.first);
                    data->set_videotagname(tag.second);
                }
            }
            tx.commit();
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        error_code = suiErrorCodeDef::ERR_SERVER;
        error_msg = "未知错误";
        return;
    }


    std::string videoServerData::createId()
    {
        return ::suiRandom::RandomUtil::uuid(suiRandom::RandomUtil::UuidType::ALL);
    }

    suiDataSql::videoStatus videoServerData::videoStatusMap(suiApi::videoStatus status)
    {
        switch (status) 
        {
            case suiApi::videoStatusUnknow:
                return suiDataSql::videoStatus::videoStatusUnknow;
            case suiApi::videoStatusUpload:
                return suiDataSql::videoStatus::videoStatusUpload;
            case suiApi::videoStatusPendingReview:
                return suiDataSql::videoStatus::videoStatusPendingReview;
            case suiApi::videoStatusApproved:
                return suiDataSql::videoStatus::videoStatusApproved;
            case suiApi::videoStatusReject:
                return suiDataSql::videoStatus::videoStatusReject;
            case suiApi::videoStatusRemove:
                return suiDataSql::videoStatus::videoStatusRemove;
            case suiApi::videoStatusTranscoding:
                return suiDataSql::videoStatus::videoStatusTranscoding;
            default:
                return suiDataSql::videoStatus::videoStatusUnknow;
        }
    }

    suiApi::videoStatus videoServerData::videoStatusMap(suiDataSql::videoStatus status)
    {
        switch (status) 
        {
            case suiDataSql::videoStatus::videoStatusUnknow:
                return suiApi::videoStatusUnknow;
            case suiDataSql::videoStatus::videoStatusUpload:
                return suiApi::videoStatusUpload;
            case suiDataSql::videoStatus::videoStatusTranscoding:
                return suiApi::videoStatusTranscoding;
            case suiDataSql::videoStatus::videoStatusPendingReview:
                return suiApi::videoStatusPendingReview;
            case suiDataSql::videoStatus::videoStatusApproved:
                return suiApi::videoStatusApproved;
            case suiDataSql::videoStatus::videoStatusReject:
                return suiApi::videoStatusReject;
            case suiDataSql::videoStatus::videoStatusRemove:
                return suiApi::videoStatusRemove;
            default:
                return suiApi::videoStatusUnknow;
        }
    }
}
