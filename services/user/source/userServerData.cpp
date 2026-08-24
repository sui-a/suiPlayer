#include "userServerData.hpp"


namespace suiUser
{
    const std::string suiUserServerData::_emailHtmlTemplateFieldTitle = "{{title}}";
    const std::string suiUserServerData::_emailHtmlTemplateFieldCode = "{{ emailVerifyCode }}";
    const std::string suiUserServerData::_emailHtmlTemplateFieldTargetEmail = "{{targetEmail}}";
    const std::string suiUserServerData::_emailHtmlTemplateFieldEmailfrom = "{{emailfrom}}";

    suiUserServerData::suiUserServerData(const suiOdb::odbSetting &ms,
                const suiRedis::redisSettings &rs, suiQueue::MQClient::ptr mq, suiMail::suiMailClient::ptr mail, suiCacheSync::CacheSyncClient::ptr file_remove_syne)
        : _mail(mail)
        , _file_remove_syne(file_remove_syne)
    {
        //初始化随机数种子
        srand((unsigned int)time(NULL));
        //创建操作句柄
        _redis = suiRedis::RedisFactory::create(rs);
        _mysql = suiOdb::dbFactory::create(ms);
        _cache_sync = std::make_shared<suiRemoveCache::RemoveCache>(_redis, mq);
    }

    suiUserServerData::~suiUserServerData()
    {

    }

    void suiUserServerData::tempLogin(int32_t& error_code, std::string& error_msg, suiApi::tempLoginResult& result)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            //创建会话id
            auto session_id = createRandomId();
            {
                //向数据库添加会话
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                suiDataSql::suiSessionMeta session;
                session.setSessionId(session_id);
                sessionHandle.insert(session);
            }
            tx.commit();
            {
                result.set_id(session_id);
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::sessionLogin(int32_t& error_code, std::string& error_msg, const std::string& ssid, suiApi::sessionLoginResult& result)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            {
                //查询会话
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto session = sessionHandle.selectBySessionId(ssid);
                if (session == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                }
                else
                {
                    if(session->getUserId().null())
                    {
                        //说明是临时用户
                        result.set_isguest(true);
                    }
                    else
                    {
                        result.set_isguest(false);
                        result.set_userid(session->getUserId().get());
                    }
                }
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::addVerifyCode(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& email, suiApi::getEmailCodeResult& result)
    {
        try
        {
            //验证码id
            std::string codeId = createRandomId();
            //随机验证码
            std::string verifyCode = createVerifyCode();
            {
                //开始发送验证码
                if(_mail->send(email, getEmailHtmlContent("验证码", verifyCode, email, "2076354958@qq.com")) == false)
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_VERIFY_CODE_MISMATCH;
                    error_msg = "验证码获取失败";
                    return;
                }
            }
            auto rtx = _redis->transaction(false, false);
            auto reHandler = rtx.redis();
            {
                //添加验证码
                suiVerifyCode::suiVerifyCodeOperation verifyCodeHandle(reHandler, _cache_sync);
                verifyCodeHandle.insert(codeId, ssid, verifyCode);
            }
            {
                error_code = suiErrorCodeDef::SUCCESS;
                result.set_codeid(codeId);
            }
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

    void suiUserServerData::verifyCodeLogin(const suiApi::emailNumberLoginReq& request, suiApi::emailNumberLoginRsp& rsp)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string sessionId = request.sessionid();
            {
                //查询验证码
                suiVerifyCode::suiVerifyCodeOperation verifyCodeHandle(reHandler, _cache_sync);
                auto codeMeta = verifyCodeHandle.get(request.codeid());
                if(codeMeta == nullptr)
                {
                    rsp.set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_VERIFY_CODE_MISMATCH);
                    rsp.set_errormsg("验证码不存在");
                    return;
                }
                //存在验证码，验证验证码与会话是否一直
                if(codeMeta->_session_id != sessionId)
                {
                    rsp.set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_VERIFY_CODE_MISMATCH);
                    rsp.set_errormsg("验证码与会话不一致");
                    return;
                }
                if(codeMeta->_code != request.verifycode())
                {
                    rsp.set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_VERIFY_CODE_INVALID);
                    rsp.set_errormsg("验证码错误");
                    return;
                }
            }
            //验证成功
            suiDataSql::suiSessionMeta::ptr sessionMeta = nullptr;
            {
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                sessionMeta = sessionHandle.selectBySessionId(sessionId);
                if(sessionMeta == nullptr)
                {
                    rsp.set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID);
                    rsp.set_errormsg("会话无效");
                    return;
                }
            }
            //会话有效且验证成功
            suiDataSql::suiUsrMeta::ptr userInfo = nullptr;
            {
                //寻找用户
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                userInfo = userHandle.getUserInfoByEmail(request.emailnumber());
            }
            std::string userId;
            if(userInfo == nullptr)
            {
                //用户不存在
                userId = createRandomId();
                {
                    //新增用户
                    suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                    userHandle.insert(userId, request.emailnumber());
                }
                {
                    //新增权限
                    suiUserIdentityRole::UserIdentityRole roleHandle(dbHandler, reHandler, _cache_sync);
                    roleHandle.insert(userId, suiDataSql::roleType::roleTypeNormal, suiDataSql::identityType::identityTypeNormal);
                }
            }
            else
            {
                //用户存在
                userId = userInfo->getUserId();
                //删除用户以前的会话信息
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                sessionHandle.removeByUserId(userId);
            }
            //
            {
                //登录
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                sessionMeta->setUserId(userId);
                sessionHandle.update(*sessionMeta);
            }
            tx.commit();
            {
                auto result = rsp.mutable_result();
                result->set_userid(userId);
                rsp.set_errorcode(suiErrorCodeDef::SUCCESS);
            }
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
        rsp.set_errorcode(suiErrorCodeDef::ERR_SERVER);
        rsp.set_errormsg("未知错误");
        return;
    }

    void suiUserServerData::emailPasswordLogin(const std::string& ssid, const std::string& email, const std::string& passward, suiApi::passwordLoginResult& result
                            , int32_t& error_code, std::string& error_msg)
    {
        error_code = suiErrorCodeDef::SUCCESS;
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userId;
            {
                //寻找用户
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                auto userInfo = userHandle.getUserInfoByEmail(email);
                if(userInfo == nullptr)
                {
                    //用户不存在
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND;
                    error_msg = "用户不存在";
                    return;
                }
                if(userInfo->getPassword().null() || userInfo->getPassword().get() != passward)
                {
                    //密码没设置或者对不上
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_PASSWORD_INVALID;
                    error_msg = "密码错误";
                    return;
                }
                userId = userInfo->getUserId();
            }
            //验证成功
            {
                //登录
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                sessionHandle.removeByUserId(userId);
                auto sessionMeta = sessionHandle.selectBySessionId(ssid);
                if(sessionMeta == nullptr)
                {
                    //会话状态异常
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(!sessionMeta->getUserId().null())
                {
                    //会话已登录
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SESSION_STATUS_INVALID;
                    error_msg = "会话已登录";
                    return;
                }
                sessionMeta->setUserId(userId);
                sessionHandle.update(*sessionMeta);
            }
            tx.commit();
            {
                result.set_userid(userId);
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::logout(int32_t& error_code, std::string& error_msg, const std::string& ssid)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            {
                //注销
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto sessionMeta = sessionHandle.selectBySessionId(ssid);
                if(sessionMeta == nullptr)
                {
                    //会话状态异常
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(sessionMeta->getUserId().null())
                {
                    //此时直接当作未登录
                }
                else
                {
                    sessionMeta->setUserId(odb::nullable<std::string>());
                    sessionHandle.update(*sessionMeta);
                }
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::setUserAvatar(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& avatar)
    {   
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userid;
            {
                //获取用户id
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto sessionMeta = sessionHandle.selectBySessionId(ssid);
                if(sessionMeta == nullptr)
                {
                    //会话状态异常
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(sessionMeta->getUserId().null())
                {
                    //会话未登录
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SESSION_STATUS_INVALID;
                    error_msg = "会话未登录";
                    return;
                }
                userid = sessionMeta->getUserId().get();
            }
            std::string preAvatarId;
            {
                //更新用户头像
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                preAvatarId = userHandle.setAvatar(userid, avatar);
            }
            if(!preAvatarId.empty())
            {
                //延迟删除文件
                _file_remove_syne->syncCache(preAvatarId);
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::setUserName(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& newName)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userid;
            {
                //获取用户id
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto sessionMeta = sessionHandle.selectBySessionId(ssid);
                if(sessionMeta == nullptr)
                {
                    //会话状态异常
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(sessionMeta->getUserId().null())
                {
                    //会话未登录
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SESSION_STATUS_INVALID;
                    error_msg = "会话未登录";
                    return;
                }
                userid = sessionMeta->getUserId().get();
            }
            {
                //开始修改
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                suiDataSql::suiUsrMeta::ptr userinfo =  userHandle.getUserInfoById(userid);
                if(userinfo == nullptr)
                {
                    INFO("用户{}不存在", userid);
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND;
                    error_msg = "用户不存在";
                    return;
                }
                userinfo->setUserName(newName);
                userHandle.updateUser(userinfo);
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::setPassWord(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& passward)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userid;
            {
                //获取用户id
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto sessionMeta = sessionHandle.selectBySessionId(ssid);
                if(sessionMeta == nullptr)
                {
                    //会话状态异常
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(sessionMeta->getUserId().null())
                {
                    //会话未登录
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SESSION_STATUS_INVALID;
                    error_msg = "会话未登录";
                    return;
                }
                userid = sessionMeta->getUserId().get();
            }
            {
                //开始修改
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                suiDataSql::suiUsrMeta::ptr userinfo =  userHandle.getUserInfoById(userid);
                if(userinfo == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND;
                    error_msg = "用户不存在";
                    return;
                }
                userinfo->setPassword(passward);
                userHandle.updateUser(userinfo);
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::setUserStatus(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId, suiApi::userStatus newSatus)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userid;
            {
                //获取用户id
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto sessionMeta = sessionHandle.selectBySessionId(ssid);
                if(sessionMeta == nullptr)
                {
                    //会话状态异常
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(sessionMeta->getUserId().null())
                {
                    //会话未登录
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SESSION_STATUS_INVALID;
                    error_msg = "会话未登录";
                    return;
                }
                userid = sessionMeta->getUserId().get();
            }
            {
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                auto targetInfo = userHandle.getUserInfoById(targetUserId);
                targetInfo->setUserStatus(userStatusTransformation(newSatus));
                userHandle.updateUser(targetInfo);
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::getUserInfo(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId, suiApi::userInfoResult& result)
    {
        INFO("开始获取用户{}的信息", targetUserId);
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userId;
            {
                //获取用户id
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto sessionMeta = sessionHandle.selectBySessionId(ssid);
                if(sessionMeta == nullptr)
                {
                    //会话状态异常
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(!sessionMeta->getUserId().null())
                    userId = sessionMeta->getUserId().get();
            }
            suiDataSql::suiUsrMeta::ptr userInfo = nullptr;
            {
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                userInfo = userHandle.getUserInfoById(targetUserId);
                if(userInfo == nullptr)
                {
                    INFO("用户{}不存在", targetUserId);
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND;
                    error_msg = "目标用户不存在";
                    return;
                }
            }
            suiDataSql::suiUserIdIdentityRoleMeta::ptr userRole = nullptr;
            {
                //获取权限信息
                suiUserIdentityRole::UserIdentityRole roleHandle(dbHandler, reHandler, _cache_sync);
                userRole =  roleHandle.select(targetUserId);
                if(userRole == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_IDENTITY_ROLE_INVALID;
                    error_msg = "用户属性异常";
                    return;
                }
            }
            suiDataSql::UserHomepageBasicData::ptr userStatics = nullptr;
            {
                //统计数据
                suiUserStatics::suiStatics staticHandle(dbHandler, reHandler, _cache_sync, rtx);
                userStatics = staticHandle.getUserBasicData(targetUserId);
            }
            bool isfollow = false;
            if(!userId.empty())
            {
                //判断是否关注
                suiUserFollow::suiFollow followHandle(dbHandler);
                isfollow = followHandle.judgment(userId, targetUserId);
            }
            INFO("用户{}关注状态：{}", targetUserId, isfollow);
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
                auto ret = result.mutable_userinfo();
                {
                    ret->set_userid(userInfo->getUserId());
                    ret->set_email(userInfo->getBindEmail());
                    ret->set_nickname(userInfo->getUserName());
                    ret->set_role(RoleTransformation(userRole->getRoleType()));
                    ret->set_identify(identityTransformation(userRole->getIdentityType()));
                    ret->set_likecount(userStatics->videoLikeCount);
                    ret->set_playcount(userStatics->videoPlayCount);
                    ret->set_followcount(userStatics->followCount);
                    ret->set_funscount(userStatics->fansCount);
                    ret->set_status(userStatusTransformation(userInfo->getUserStatus()));
                    ret->set_isfollowing(isFollowTransformation(isfollow));
                    if(!userInfo->getUserDescription().null())
                        ret->set_usermemo(userInfo->getUserDescription().get());
                    ret->set_userctime(std::to_string(userInfo->getUploadTime()));
                    if(!userInfo->getHeadImageFileId().null())
                        ret->set_avatarfileid(userInfo->getHeadImageFileId().get());
                }
            }
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

    void suiUserServerData::addFollowing(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userId;
            {
                //获取用户id
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto sessionMeta = sessionHandle.selectBySessionId(ssid);
                if(sessionMeta == nullptr)
                {
                    //会话状态异常
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(sessionMeta->getUserId().null())
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SESSION_STATUS_INVALID;
                    error_msg = "会话未登录";
                    return;
                }
                userId = sessionMeta->getUserId().get();
            }
            
            {
                //验证目标是否存在
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                auto userInfo = userHandle.getUserInfoById(targetUserId);
                if(userInfo == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND;
                    error_msg = "目标用户不存在";
                    return;
                }
            }
            {
                //进行添加
                suiUserFollow::suiFollow followHandle(dbHandler);
                followHandle.insert(userId, targetUserId);
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::removeFollowing(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userId;
            {
                //获取用户id
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto sessionMeta = sessionHandle.selectBySessionId(ssid);
                if(sessionMeta == nullptr)
                {
                    //会话状态异常
                    error_code = suiErrorCodeDef::ERR_FILE_SERVICE_SESSION_INVALID;
                    error_msg = "会话无效";
                    return;
                }
                if(sessionMeta->getUserId().null())
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SESSION_STATUS_INVALID;
                    error_msg = "会话未登录";
                    return;
                }
                userId = sessionMeta->getUserId().get();
            }
            {
                //进行添加
                suiUserFollow::suiFollow followHandle(dbHandler);
                followHandle.remove(userId, targetUserId);
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::addAdmin(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId)
    {
        (void)ssid;
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            {
                //判断目标是否存在
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                auto userInfo = userHandle.getUserInfoById(targetUserId);
                if(userInfo == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND;
                    error_msg = "目标用户不存在";
                    return;
                }
            }
            {
                //获取权限信息
                suiUserIdentityRole::UserIdentityRole roleHandle(dbHandler, reHandler, _cache_sync);
                auto userRole =  roleHandle.select(targetUserId);
                if(userRole == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_IDENTITY_ROLE_INVALID;
                    error_msg = "用户属性异常";
                    return;
                }
                if(userRole->getRoleType() == suiDataSql::roleType::roleTypeNormal)
                {
                    //普通角色才进行提权
                    userRole->setRoleType(suiDataSql::roleType::roleTypeAdmin);
                    userRole->setIdentityType(suiDataSql::identityType::identityTypeAdmin);
                    roleHandle.update(userRole);
                }
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::removeAdmin(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId)
    {
        (void)ssid;
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            {
                //判断目标是否存在
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                auto userInfo = userHandle.getUserInfoById(targetUserId);
                if(userInfo == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND;
                    error_msg = "目标用户不存在";
                    return;
                }
            }
            {
                //获取权限信息
                suiUserIdentityRole::UserIdentityRole roleHandle(dbHandler, reHandler, _cache_sync);
                auto userRole =  roleHandle.select(targetUserId);
                if(userRole == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_IDENTITY_ROLE_INVALID;
                    error_msg = "用户属性异常";
                    return;
                }
                if(userRole->getRoleType() == suiDataSql::roleType::roleTypeAdmin)
                {
                    //普通管理员才进行降权
                    userRole->setRoleType(suiDataSql::roleType::roleTypeNormal);
                    userRole->setIdentityType(suiDataSql::identityType::identityTypeNormal);
                    roleHandle.update(userRole);
                }
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::editAdmin(int32_t& error_code, std::string& error_msg, const std::string& ssid, const suiApi::AdminInfo& userInfo)
    {
        (void)ssid;
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string targetUserId = userInfo.userid();
            {
                //判断目标是否存在
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                auto info = userHandle.getUserInfoById(targetUserId);
                if(info == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND;
                    error_msg = "目标用户不存在";
                    return;
                }
                //进行修改
                info->setAdministratorName(userInfo.nickname());
                info->setUserStatus(userStatusTransformation(userInfo.status()));
                info->setUserDescription(userInfo.usermemo());
                info->setBindEmail(userInfo.email());
                userHandle.updateUser(info);
            }
            {
                //修改权限
                suiUserIdentityRole::UserIdentityRole roleHandle(dbHandler, reHandler, _cache_sync);
                auto userRole =  roleHandle.select(targetUserId);
                if(userRole == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_IDENTITY_ROLE_INVALID;
                    error_msg = "用户属性异常";
                    return;
                }
                //普通管理员才进行降权
                userRole->setRoleType(RoleTransformation(userInfo.role()));
                userRole->setIdentityType(identityTransformation(userInfo.identify()));
                roleHandle.update(userRole);
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::getAdminByEmail(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& email, suiApi::AdminInfo& result)
    {
        (void)ssid;
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            suiDataSql::suiUsrMeta::ptr userInfo = nullptr;
            {
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                userInfo = userHandle.getUserInfoByEmail(email);
                if(userInfo == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND;
                    error_msg = "目标用户不存在";
                    return;
                }
            }
            suiDataSql::suiUserIdIdentityRoleMeta::ptr userRole = nullptr;
            {
                suiUserIdentityRole::UserIdentityRole roleHandle(dbHandler, reHandler, _cache_sync);
                userRole =  roleHandle.select(userInfo->getUserId());
                if(userRole == nullptr)
                {
                    error_code = suiErrorCodeDef::ERR_USER_IDENTITY_ROLE_INVALID;
                    error_msg = "用户属性异常";
                    return;
                }
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
                result.set_userid(userInfo->getUserId());
                if(!userInfo->getAdministratorName().null())
                    result.set_nickname(userInfo->getAdministratorName().get());
                result.set_role(RoleTransformation(userRole->getRoleType()));
                result.set_identify(identityTransformation(userRole->getIdentityType()));
                result.set_status(userStatusTransformation(userInfo->getUserStatus()));
                if(!userInfo->getUserDescription().null())
                    result.set_usermemo(userInfo->getUserDescription().get());
                result.set_email(userInfo->getBindEmail());
            }
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

    void suiUserServerData::getAminList(int32_t& error_code, std::string& error_msg, const std::string& ssid, suiApi::roleType role
                                    , int32_t pageIndex, int32_t pageCount, suiApi::GetAdminListResult& result)
    {
        (void)ssid;
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            suiDataSql::userInfoList::ptr userList = nullptr;
            {
                suiUserInformation::suiUserInformationOperation userHandle(tx, rtx, _cache_sync);
                userList = userHandle.getUserInfoListByTypeAndStatus(RoleTransformation(role),suiDataSql::userStatus::userStatusEnable
                        , suiDataSql::identityType::identityTypeAdmin, pageIndex, pageCount);
            }
            auto adminList = result.mutable_userlist();
            {
                suiUserIdentityRole::UserIdentityRole roleHandle(dbHandler, reHandler, _cache_sync);
                for(auto& userInfo : userList->list)
                {
                    auto userRole =  roleHandle.select(userInfo.getUserId());
                    if(userRole == nullptr)
                        continue;

                    auto adminInfo = adminList->Add();
                    adminInfo->set_userid(userInfo.getUserId());
                    if(!userInfo.getAdministratorName().null())
                    adminInfo->set_nickname(userInfo.getAdministratorName().get());
                    adminInfo->set_role(RoleTransformation(userRole->getRoleType()));
                    adminInfo->set_identify(identityTransformation(userRole->getIdentityType()));
                    adminInfo->set_status(userStatusTransformation(userInfo.getUserStatus()));
                    if(!userInfo.getUserDescription().null())
                        adminInfo->set_usermemo(userInfo.getUserDescription().get());
                    adminInfo->set_email(userInfo.getBindEmail());
                }
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::setSalt(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& salt)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userId;
            {
                //寻找用户
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto sessionInfo = sessionHandle.selectBySessionId(ssid);
                if(sessionInfo == nullptr)
                {
                    //会话不存在
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID;
                    error_msg = "会话不存在";
                    return;
                }
                if(sessionInfo->getUserId().null())
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID;
                    error_msg = "会话未登录";
                    return;
                }
                userId = sessionInfo->getUserId().get();
            }
            {
                //开始存储
                suiSalt::suiSaltOperation saltHandle(dbHandler, _cache_sync);
                saltHandle.removeByUserId(userId);
                saltHandle.insert(userId, salt);
            }
            tx.commit();
            {
                error_code = suiErrorCodeDef::SUCCESS;
            }
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

    void suiUserServerData::getSalt(int32_t& error_code, std::string& error_msg, const std::string& ssid, suiApi::GetSaltResult& result)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string userId;
            {
                //寻找用户
                suiSession::sessionData sessionHandle(dbHandler, reHandler, _cache_sync);
                auto sessionInfo = sessionHandle.selectBySessionId(ssid);
                if(sessionInfo == nullptr)
                {
                    //会话不存在
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID;
                    error_msg = "会话不存在";
                    return;
                }
                if(sessionInfo->getUserId().null())
                {
                    error_code = suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID;
                    error_msg = "会话未登录";
                    return;
                }
                userId = sessionInfo->getUserId().get();
            }
            suiDataSql::suiSaltMeta::ptr saltInfo = nullptr;
            {
                //开始存储
                suiSalt::suiSaltOperation saltHandle(dbHandler, _cache_sync);
                saltInfo = saltHandle.selectByUserId(userId);
            }
            tx.commit();
            if(saltInfo != nullptr)
            {
                error_code = suiErrorCodeDef::SUCCESS;
                std::string cursalt;
                for(auto it : saltInfo->getSalt())
                    cursalt += it;
                if(!cursalt.empty())
                    result.set_salt(cursalt);
            }
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

    std::string suiUserServerData::createRandomId()
    {
        //创建会话id
        return ::suiRandom::RandomUtil::uuid(suiRandom::RandomUtil::UuidType::ALL);
    }

    std::string suiUserServerData::createVerifyCode()
    {
        //获取6位数字
        int val = rand() % 1000000;
    
        std::ostringstream oss;
        oss << std::setw(6) << std::setfill('0') << val;
        return oss.str();
    }

    void suiUserServerData::replaceAll(std::string& str, const std::string& from, const std::string& to)
    {
        if (from.empty()) 
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) 
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); 
        }
    }

    std::string suiUserServerData::getEmailHtmlContent(const std::string& title, const std::string& body, const std::string& targetEmail, const std::string& emailfrom)
    {
        //读取html模板
        std::ifstream file("./views/emailVerifyCode.html");
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string htmlContent = buffer.str();
        //替换模板字段
        replaceAll(htmlContent, _emailHtmlTemplateFieldTitle, title);
        replaceAll(htmlContent, _emailHtmlTemplateFieldCode, body);
        replaceAll(htmlContent, _emailHtmlTemplateFieldTargetEmail, targetEmail);
        replaceAll(htmlContent, _emailHtmlTemplateFieldEmailfrom, emailfrom);
        return htmlContent;
    }

    ::suiDataSql::userStatus suiUserServerData::userStatusTransformation(::suiApi::userStatus userStatus)
    {
        switch (userStatus)
        {
            case ::suiApi::userStatus::userStatusUnknow:
                return ::suiDataSql::userStatus::userStatusUnknow;
            case ::suiApi::userStatus::userStatusEnable:
                return ::suiDataSql::userStatus::userStatusEnable;
            case ::suiApi::userStatus::userStatusDisable:
                return ::suiDataSql::userStatus::userStatusDisable;
            default:
                return ::suiDataSql::userStatus::userStatusUnknow;
        }
    }

    //用户身份
    ::suiDataSql::identityType suiUserServerData::identityTransformation(::suiApi::identityType identity)
    {
        switch (identity)
        {
            case ::suiApi::identityType::identityUnknow:
                return ::suiDataSql::identityType::identityTypeUnknown;
            case ::suiApi::identityType::identityNormal:
                return ::suiDataSql::identityType::identityTypeNormal;
            case ::suiApi::identityType::identityAdmin:
                return ::suiDataSql::identityType::identityTypeAdmin;
            default:
                return ::suiDataSql::identityType::identityTypeUnknown;
        }
    }

    //用户角色类型转换
    ::suiDataSql::roleType suiUserServerData::RoleTransformation(::suiApi::roleType Role)
    {
        switch (Role)
        {
            //本地访客也切换成未知用户，因为无法和数据库中的用户属性映射起来
            case ::suiApi::roleType::roleUnknow:
                return ::suiDataSql::roleType::roleTypeUnknown;
            case ::suiApi::roleType::roleUserNormal:
                return ::suiDataSql::roleType::roleTypeNormal;
            case ::suiApi::roleType::roleAdminNormal:
                return ::suiDataSql::roleType::roleTypeAdmin;
            case ::suiApi::roleType::roleAdminSuper:
                return ::suiDataSql::roleType::roleTypeSuperAdmin;
            default:
                return ::suiDataSql::roleType::roleTypeUnknown;
        }
    }

    bool suiUserServerData::isFollowTransformation(::suiApi::followStatus value)
    {
        switch (value)
        {
            case ::suiApi::followStatus::followStatusTrue:
                return true;
            default:
                return false;
        }
    }

    //反转
    //用户状态转换
    ::suiApi::userStatus suiUserServerData::userStatusTransformation(::suiDataSql::userStatus userStatus)
    {
        switch (userStatus)
        {
            case ::suiDataSql::userStatus::userStatusUnknow:
                return ::suiApi::userStatus::userStatusUnknow;
            case ::suiDataSql::userStatus::userStatusEnable:
                return ::suiApi::userStatus::userStatusEnable;
            case ::suiDataSql::userStatus::userStatusDisable:
                return ::suiApi::userStatus::userStatusDisable;
            default:
                return ::suiApi::userStatus::userStatusUnknow;
        }
    }

    //用户身份
    ::suiApi::identityType suiUserServerData::identityTransformation(::suiDataSql::identityType identity)
    {
        switch (identity)
        {
            case ::suiDataSql::identityType::identityTypeUnknown:
                return ::suiApi::identityType::identityUnknow;
            case ::suiDataSql::identityType::identityTypeNormal:
                return ::suiApi::identityType::identityNormal;
            case ::suiDataSql::identityType::identityTypeAdmin:
                return ::suiApi::identityType::identityAdmin;
            default:
                return ::suiApi::identityType::identityUnknow;
        }
    }

    //用户角色类型转换
    ::suiApi::roleType suiUserServerData::RoleTransformation(::suiDataSql::roleType Role)
    {
        switch (Role)
        {
            case ::suiDataSql::roleType::roleTypeUnknown:
                return ::suiApi::roleType::roleUnknow;
            case ::suiDataSql::roleType::roleTypeNormal:
                return ::suiApi::roleType::roleUserNormal;
            case ::suiDataSql::roleType::roleTypeAdmin:
                return ::suiApi::roleType::roleAdminNormal;
            case ::suiDataSql::roleType::roleTypeSuperAdmin:
                return ::suiApi::roleType::roleAdminSuper;
            default:
                return ::suiApi::roleType::roleUnknow;
        }
    }

    ::suiApi::followStatus suiUserServerData::isFollowTransformation(bool value)
    {
        if (value)
        {
            return ::suiApi::followStatus::followStatusTrue;
        }
        return ::suiApi::followStatus::followStatusFalse;
    }


}