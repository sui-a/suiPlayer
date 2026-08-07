#include "userServerData.hpp"


namespace suiUser
{
    suiUserServerData::suiUserServerData(const suiOdb::odbSetting &ms,
                const suiRedis::redisSettings &rs, suiQueue::MQClient::ptr mq)
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

    //访客登录
    std::string suiUserServerData::touristLogin()
    {
        
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            std::string key;

            {
                //创建会话操作类
                suiSession::sessionData sessionData(dbHandler, reHandler, _cache_sync);
                while(1)
                {
                    //循环创建id，用id去查询会话表，如果不存在直接break
                    key = createRandomId(); //后续修改多次碰撞

                    //开始查询会话
                    if(!sessionData.selectBySessionId(key))
                    {
                        //构建会话数据
                        suiDataSql::suiSessionMeta curMeta;
                        curMeta.setSessionId(key);
                        //直接添加
                        sessionData.insert(curMeta); //游客访问，不添加用户id，只存储会话id
                        break;
                    }
                    //
                }
                //
            }
            tx.commit();
            return key;
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

        return std::string();
    }

    bool suiUserServerData::verifySession(const std::string& session_id, const std::string& user_id)
    {
        if(user_id.empty())
            return false; //防止攻击
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            {
                suiSession::sessionData sessionData(dbHandler, reHandler, _cache_sync);
                //查询
                auto meta = sessionData.selectBySessionId(session_id);
                if(!meta || meta->getUserId().null() || meta->getUserId().get() != user_id)
                {
                    //此时出现问题
                    return false;
                }
            }
            //提交事务
            tx.commit();
            return true;
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
        return false;
    }

    bool suiUserServerData::verifySession(const std::string& session_id, std::string* user_id)
    {
        if(session_id.empty())
            return false; //防止攻击
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            {
                suiSession::sessionData sessionData(dbHandler, reHandler, _cache_sync);
                //查询
                auto meta = sessionData.selectBySessionId(session_id);
                if(!meta)
                {
                    //此时出现问题
                    return false;
                }
                if(!meta->getUserId().null() && user_id)
                {
                    *user_id = meta->getUserId().get();
                }
            }
            //提交事务
            tx.commit();
            return true;
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
        return false;
    }

    bool suiUserServerData::isFollow(const std::string& user_id, const std::string& targetUserId)
    {
        if(user_id.empty()) //传入的用户id为空 直接当作没有关注
            return false;
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            bool isFollow = false;
            {
                suiUserFollow::suiFollow handle(dbHandler);
                isFollow = handle.judgment(user_id, targetUserId);
            }
            tx.commit();
            return isFollow;
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
        return false;
    }

    bool suiUserServerData::verifyUser(const std::string& user_id)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            bool ret = false;
            {
                //查询用户是否存在
                suiUserInformation::suiUserInformationOperation userInformation(tx, rtx, _cache_sync);
                auto meta = userInformation.getUserInfoById(user_id);
                if(meta)
                    ret = true;
            }
            tx.commit();
            return ret;
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
        return false;
    }

    bool suiUserServerData::isEnable(const std::string& email)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            {
                //查询用户是否存在
                suiUserInformation::suiUserInformationOperation userInformation(tx, rtx, _cache_sync);
                auto meta = userInformation.getUserInfoByEmail(email);
                if(meta != nullptr && meta->getUserStatus() == suiDataSql::userStatus::userStatusDisable)
                    return false;
            }
            tx.commit();
            return true;
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
        return false;
    }

    bool suiUserServerData::hasPermission(const std::string& user_id, const std::string& operationId)
    {
        //查看某个用户是否有执行某个操作的权限
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            //
            auto userIdentity = suiDataSql::identityType::identityTypeUnknown; //默认未知身份
            auto userRole = suiDataSql::roleType::roleTypeUnknown; //默认未知角色
            if(!user_id.empty())
            {
                //如果有id，查询用户身份以及角色信息
                suiUserIdentityRole::UserIdentityRole roleIdentity(dbHandler, reHandler, _cache_sync);
                auto meta = roleIdentity.select(user_id);
                if(meta != nullptr)
                {
                    userIdentity = meta->getIdentityType();
                    userRole = meta->getRoleType();
                }
            }

            //查询操作权限
            {
                
            }

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
        return false;
    }

    std::string suiUserServerData::addVerifyCode(const std::string& session_id, std::string* codeptr)
    {
        try
        {
            //创建事务
            auto rtx = _redis->transaction(false, false);
            //创建绑定线程的操作句柄
            auto reHandler = rtx.redis();

            //不进行会话有效验证，直接创建验证码
            std::string code = createVerifyCode();
            std::string code_id;

            //创建验证码
            {
                suiVerifyCode::suiVerifyCodeOperation verifyCode(reHandler, _cache_sync);
                while(1)
                {
                    //创建随机codeid
                    code_id = createRandomId();
                    //判断是否存在
                    if(!verifyCode.get(code_id))
                        break;  //不存在退出
                }
                //将验证码上传
                verifyCode.insert(code_id, session_id, code);
                if(codeptr)
                    *codeptr += code;
            }
            return code_id;
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库添加验证码，会话id： {}， 异常： {}", session_id, e.what());
        }
        catch (...)
        {
            ERROR("添加验证码未知异常， 会话id： {}", session_id);
        }

        return std::string();
    }

    std::string suiUserServerData::getVerifyCode(const std::string& code_id)
    {
        try
        {
            //创建事务
            auto rtx = _redis->transaction(false, false);
            //创建绑定线程的操作句柄
            auto reHandler = rtx.redis();

            {
                //获取验证码
                suiVerifyCode::suiVerifyCodeOperation verifyCode(reHandler, _cache_sync);
                auto ret = verifyCode.get(code_id);
                if(ret)
                {
                    return ret->_code;
                }
            }

            INFO("验证码不存在， 验证码id： {}", code_id);
            return std::string();
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库获取验证码，验证码id： {}， 异常： {}", code_id, e.what());
        }
        catch (...)
        {
            ERROR("获取验证码未知异常， 验证码id： {}", code_id);
        }

        //不存在直接返回空字符串
        return std::string();
    }

    bool suiUserServerData::verifyAndRemoveVerifyCode(const std::string& session_id, const std::string& code_id, const std::string& code)
    {
        try
        {
            //创建事务
            auto rtx = _redis->transaction(false, false);
            //创建绑定线程的操作句柄
            auto reHandler = rtx.redis();

            {
                //验证
                suiVerifyCode::suiVerifyCodeOperation verifyCode(reHandler, _cache_sync);
                auto ret = verifyCode.get(code_id);
                if(ret && ret->_code == code && ret->_session_id == session_id)
                {
                    //验证成功，删除验证码直接返回true
                    verifyCode.remove(code_id);
                    return true;
                }
            }
            INFO("验证码验证失败， 验证码id： {}", code_id);
            return false;
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库验证验证码，验证码id： {}， 异常： {}", code_id, e.what());
        }
        catch (...)
        {
            ERROR("验证验证码失败， 验证码id： {}", code_id);
        }
        return false;
    }

    void suiUserServerData::removeVerifyCode(const std::string& code_id)
    {
        try
        {
            //创建事务
            auto rtx = _redis->transaction(false, false);
            //创建绑定线程的操作句柄
            auto reHandler = rtx.redis();

            {
                suiVerifyCode::suiVerifyCodeOperation verifyCode(reHandler, _cache_sync);
                verifyCode.remove(code_id);
            }
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库验证验证码，验证码id： {}， 异常： {}", code_id, e.what());
        }
        catch (...)
        {
            ERROR("验证验证码失败， 验证码id： {}", code_id);
        }
    }

    bool suiUserServerData::setSalt(const std::string& user_id, const std::string& salt)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            {
                suiSalt::suiSaltOperation saltOperation(tx.database(), _cache_sync);
                auto ret = saltOperation.selectByUserId(user_id);
                if(ret == nullptr)
                {
                    suiDataSql::suiSaltMeta meta;
                    meta.setUserId(user_id);
                    meta.setSalt(salt);
                    saltOperation.insert(meta);
                }
                else
                {
                    ret->setSalt(salt);
                    saltOperation.update(*ret);
                }
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库设置异常，用户id： {}， 异常： {}", user_id, e.what());
        }
        catch (...)
        {
            ERROR("设置失败未知异常， 用户id： {}", user_id);
        }
        return false;
    }

    //获取
    bool suiUserServerData::getSalt(const std::string& user_id, std::string& salt)
    {
        try
        {
            //创建数据库事务
            odb::transaction tx(_mysql->begin());
            {
                //获取盐
                suiSalt::suiSaltOperation saltOperation(tx.database(), _cache_sync);
                auto ret = saltOperation.selectByUserId(user_id);
                if(ret) //找到才进行拼接，否则不进行处理
                {
                    auto& saltv = ret->getSalt();
                    for(auto it : saltv)
                        salt += it;
                }
                //不存在也返回true
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获取异常，用户id： {}， 异常： {}", user_id, e.what());
        }
        catch (...)
        {
            ERROR("获取失败未知异常， 用户id： {}", user_id);
        }
        //只有数据库错误返回false
        return false;
    }

    //修改/新增盐
    void suiUserServerData::changeSalt(const std::string& user_id, const std::string& salt)
    {
        try
        {
            //创建数据库事务
            odb::transaction tx(_mysql->begin());
            {
                //获取盐
                suiSalt::suiSaltOperation saltOperation(tx.database(), _cache_sync);
                auto ret = saltOperation.selectByUserId(user_id);
                if(ret) //找到才进行拼接，否则不进行处理
                {
                    //修改
                    ret->setSalt(salt);
                    saltOperation.update(*ret);
                }
                else
                {
                    //直接添加
                    saltOperation.insert(user_id, salt);
                }
            }
            tx.commit();
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获取盐异常，用户id： {}， 异常： {}", user_id, e.what());
        }
        catch (...)
        {
            ERROR("获取盐失败未知异常， 用户id： {}", user_id);
        }
    }

    suiDataSql::SessionStatus suiUserServerData::sessionLogin(const std::string& session_id, std::string* userId)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            suiDataSql::suiSessionMeta::ptr ret = nullptr;
            {
                //创建会话操作对象
                suiSession::sessionData sessionData(dbHandler, reHandler, _cache_sync);
                //获取会话信息
                ret = sessionData.selectBySessionId(session_id);
            }
            tx.commit();
            if(!ret)
            {
                //此时也不属于临时
                INFO("会话过期, 会话id： {}", session_id);
                return suiDataSql::SessionStatus::Unknown; //此时返回错误状态
            }
            if(ret->getUserId().null())
            {
                return suiDataSql::SessionStatus::Guest; //此时是临时会话
            }
            if(userId)
                *userId = ret->getUserId().get();
            return suiDataSql::SessionStatus::Normal; //这个是正式会话
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库会话登录异常，会话id： {}， 异常： {}", session_id, e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库会话登录异常，会话id： {}， 异常： {}", session_id, e.what());
        }
        catch (...)
        {
            ERROR("会话登录未知异常， 会话id： {}", session_id);
        }
        return suiDataSql::SessionStatus::Unknown;
    }

    //邮箱登录
    bool suiUserServerData::emailLogin(const std::string& session_id, const std::string& email, std::string* outUserId)
    {
        bool isExist = false;
        std::string curUserId;
        try 
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            suiDataSql::suiUsrMeta::ptr emailret = nullptr;
            {
                //搜索用户
                suiUserInformation::suiUserInformationOperation useOpteration(tx, rtx, _cache_sync);
                emailret = useOpteration.getUserInfoByEmail(email);
            }

            if(emailret)
            {
                //邮箱存在
                isExist = true;
                curUserId = emailret->getUserId();
                if(emailret->getUserStatus() != suiDataSql::userStatus::userStatusEnable)
                {
                    INFO("用户状态异常， 用户 {} 被禁用", curUserId);
                    return false;
                }

                {
                    //删除所有该用户下登记的会话
                    auto& dbHandler = tx.database();
                    auto reHandler = rtx.redis();
                    suiSession::sessionData sessionData(dbHandler, reHandler, _cache_sync);
                    sessionData.removeByUserId(curUserId);
                }
                if(outUserId)
                    *outUserId = curUserId;
            }
            tx.commit();
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}, 邮箱： {}, 会话id： {}, 验证码id： {}", e.what(), email, session_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库邮箱登录异常，邮箱： {}, 异常： {}， 会话id： {}, 验证码id： {}", email, e.what(), session_id);
        }
        catch (...)
        {
            ERROR("邮箱登录未知异常， 邮箱： {}, 会话id： {}, 验证码id： {}", email, session_id);
        }
        if(isExist)
        {
            //存在
            return _emailLoginExist(session_id, curUserId);
        }
        return _emailLoginNotExist(session_id, email, outUserId);
    }

    bool suiUserServerData::emailPasswordLogin(const std::string& session_id, const std::string& email, const std::string& password, std::string* outUserId)
    {
        
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            suiDataSql::suiUsrMeta::ptr authenret = nullptr;
            {
                //获取用户基础信息
                suiUserInformation::suiUserInformationOperation useOpteration(tx, rtx, _cache_sync);
                authenret = useOpteration.getUserInfoByEmail(email);
            }
            if(!authenret)
            {
                //用户不存在
                INFO("用户不存在， 邮箱： {}", email);
                return false;
            }
            if(authenret->getPassword().null() || authenret->getPassword().get() != password 
                || authenret->getUserStatus() != suiDataSql::userStatus::userStatusEnable)
            {
                //密码验证失败
                INFO("密码验证失败， 邮箱： {}, 会话id： {}", email, session_id);
                return false;
            }

            //验证成功，更新会话信息
            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();

            {
                //更新会话信息
                suiSession::sessionData sessionData(dbHandler, reHandler, _cache_sync);
                auto sessret = sessionData.selectBySessionId(session_id);
                if(!sessret)
                {
                    //会话不存在
                    INFO("会话过期， 会话id： {}", session_id); //业务逻辑属于正常日志，不属于错误日志，使用INFO等级
                    return false;
                }
                //更新会话信息
                sessret->setUserId(authenret->getUserId());
                sessionData.update(*sessret);
            }
            if(outUserId)
                *outUserId = authenret->getUserId();
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}, 会话id：{}， 邮箱： {}", e.what(), session_id, email);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库异常： {}, 会话id：{}， 邮箱： {}", e.what(), session_id, email);
        }
        catch (...)
        {
            ERROR("邮箱登录未知异常， 会话id： {}， 邮箱： {}", session_id, email);
        }
        return false;
    }

    void suiUserServerData::logout(const std::string& session_id)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();

            {
                //注销登录会话
                suiSession::sessionData sessionData(dbHandler, reHandler, _cache_sync);
                auto sessret = sessionData.selectBySessionId(session_id);
                if(!sessret)
                {
                    //会话不存在
                    INFO("会话过期， 会话id： {}", session_id);
                    return;
                }
                //更新会话信息
                sessret->setUserId(odb::nullable<std::string>());
                sessionData.update(*sessret);
            }
            tx.commit();
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库注销会话异常： {}， 会话id: {}", e.what(), session_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库注销会话异常： {}， 会话id: {}", e.what(), session_id);
        }
        catch (...)
        {
            ERROR("注销会话异常， 未知异常， 会话id： {}", session_id);
        }
        return;
    }

    void suiUserServerData::removeSessionId(const std::string& session_id)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();

            {
                //删除会话id
                suiSession::sessionData sessionData(dbHandler, reHandler, _cache_sync);
                sessionData.removeBySessionId(session_id);
            }
            tx.commit();
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库注销会话异常： {}， 会话id: {}", e.what(), session_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库注销会话异常： {}， 会话id: {}", e.what(), session_id);
        }
        catch (...)
        {
            ERROR("注销会话异常， 未知异常， 会话id： {}", session_id);
        }
        return;
    }

    bool suiUserServerData::changeName(const std::string& user_id, const std::string& name)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            {
                //修改用户名
                suiUserInformation::suiUserInformationOperation useOpteration(tx, rtx, _cache_sync);
                auto user = useOpteration.getUserInfoById(user_id);
                if(!user)
                {
                    //用户不存在
                    INFO("用户不存在， 用户id： {}", user_id);
                    return false;
                }
                user->setUserName(name);
                useOpteration.updateUser(user);
            }
            
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库修改用户属性异常： {}， 用户id： {}", e.what(), user_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库修改用户属性异常： {}， 用户id: {}", e.what(), user_id);
        }
        catch (...)
        {
            ERROR("修改用户属性异常， 未知异常， 用户id： {}", user_id);
        }
        return false;
    }

    bool suiUserServerData::changePassword(const std::string& user_id, const std::string& password)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            {
                //修改用户密码
                suiUserInformation::suiUserInformationOperation useOpteration(tx, rtx, _cache_sync);
                useOpteration.setUserPassward(user_id, password);
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库修改用户属性异常： {}， 用户id： {}", e.what(), user_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库修改用户属性异常： {}， 用户id: {}", e.what(), user_id);
        }
        catch (...)
        {
            ERROR("修改用户属性异常， 未知异常， 用户id： {}", user_id);
        }
        return false;
    }

    bool suiUserServerData::changeAvatar(const std::string& user_id, const std::string& avatarid)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            {
                //修改用户头像
                suiUserInformation::suiUserInformationOperation useOpteration(tx, rtx, _cache_sync);
                useOpteration.setAvatar(user_id, avatarid);
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库修改用户属性异常： {}， 用户id： {}", e.what(), user_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库修改用户属性异常： {}， 用户id: {}", e.what(), user_id);
        }
        catch (...)
        {
            ERROR("修改用户属性异常， 未知异常， 用户id： {}", user_id);
        }
        return false;
    }

    bool suiUserServerData::changeStatus(const std::string& user_id, const suiDataSql::userStatus status)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            {
                //修改用户状态
                suiUserInformation::suiUserInformationOperation useOpteration(tx, rtx, _cache_sync);
                useOpteration.setUserStatus(user_id, status);
            }

            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库修改用户属性异常： {}， 用户id： {}", e.what(), user_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库修改用户属性异常： {}， 用户id: {}", e.what(), user_id);
        }
        catch (...)
        {
            ERROR("修改用户属性异常， 未知异常， 用户id： {}", user_id);
        }
        return false;
    }

    suiDataSql::suiUsrMeta::ptr suiUserServerData::getUserInfo(const std::string& user_id)
    {
        //验证会话
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            suiDataSql::suiUsrMeta::ptr user = nullptr;
            {
                //获取用户信息
                suiUserInformation::suiUserInformationOperation useOpteration(tx, rtx, _cache_sync);
                user = useOpteration.getUserInfoById(user_id);
            }
            tx.commit();
            return user;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获取用户信息异常： {}， 用户id: {}", e.what(), user_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库获取用户信息异常： {}， 用户id: {}", e.what(), user_id);
        }
        catch (...)
        {
            ERROR("获取用户信息异常， 未知异常， 用户id： {}", user_id);
        }
        return nullptr;
    }

    suiDataSql::suiUsrMeta::ptr suiUserServerData::getUserInfoByEmail(const std::string& email)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            suiDataSql::suiUsrMeta::ptr user = nullptr;
            {
                //获取用户信息
                suiUserInformation::suiUserInformationOperation useOpteration(tx, rtx, _cache_sync);
                user = useOpteration.getUserInfoByEmail(email);
            }
            tx.commit();
            return user;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获取用户信息异常： {}， 查询用户邮箱: {}", e.what(), email);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库获取用户信息异常： {}， 查询用户邮箱: {}", e.what(), email);
        }
        catch (...)
        {
            ERROR("获取用户信息异常， 未知异常， 查询用户邮箱： {}", email);
        }
        return nullptr;
    }

    suiDataSql::suiUserIdIdentityRoleMeta::ptr suiUserServerData::getIdentityRole(const std::string& user_id)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            suiDataSql::suiUserIdIdentityRoleMeta::ptr out = nullptr;
            {
                //获取用户身份角色信息
                suiUserIdentityRole::UserIdentityRole useRole(dbHandler, reHandler, _cache_sync);
                out = useRole.select(user_id);
            }
            tx.commit();
            return out;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获取用户信息异常： {}， 用户id: {}", e.what(), user_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库获取用户信息异常： {}， 用户id: {}", e.what(), user_id);
        }
        catch (...)
        {
            ERROR("获取用户信息异常， 未知异常， 用户id： {}", user_id);
        }
        return nullptr;
    }

    suiDataSql::UserHomepageBasicData::ptr suiUserServerData::getUserStatistics(const std::string& user_id)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            suiDataSql::UserHomepageBasicData::ptr out = nullptr;
            {
                suiUserStatics::suiStatics useStatics(dbHandler, reHandler, _cache_sync, rtx);
                out = useStatics.getUserBasicData(user_id);
            }
            tx.commit();
            return out;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获取用户信息异常： {}， 用户id: {}", e.what(), user_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库获取用户信息异常： {}， 用户id: {}", e.what(), user_id);
        }
        catch (...)
        {
            ERROR("获取用户信息异常， 未知异常， 用户id： {}", user_id);
        }
        return nullptr;
    }

    void suiUserServerData::addFollow(const std::string& user_id, const std::string& follow_id)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto& dbHandler = tx.database();
            {
                //新增关注信息
                suiUserFollow::suiFollow useFollow(dbHandler);
                useFollow.insert(user_id, follow_id);
            }
            tx.commit();
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库新增关注异常： {}， 用户id: {}， 被关注用户id: {}", e.what(), user_id, follow_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库新增关注异常： {}， 用户id: {}， 被关注用户id: {}", e.what(), user_id, follow_id);
        }
        catch (...)
        {
            ERROR("新增关注异常， 未知异常， 用户id： {}， 被关注用户id： {}", user_id, follow_id);
        }
    }

    void suiUserServerData::removeFollow(const std::string& user_id, const std::string& follow_id)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto& dbHandler = tx.database();
            {
                //移除关注信息
                suiUserFollow::suiFollow useFollow(dbHandler);
                useFollow.remove(user_id, follow_id);
            }
            tx.commit();
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库移除关注异常： {}， 用户id: {}， 被关注用户id: {}", e.what(), user_id, follow_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库移除关注异常： {}， 用户id: {}， 被关注用户id: {}", e.what(), user_id, follow_id);
        }
        catch (...)
        {
            ERROR("移除关注异常， 未知异常， 用户id： {}， 被关注用户id： {}", user_id, follow_id);
        }
    }

    bool suiUserServerData::addAdmin(const std::string& user_id)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            {
                //判断用户是否存在
                suiUserInformation::suiUserInformationOperation useOpteration(tx, rtx, _cache_sync);
                auto user = useOpteration.getUserInfoById(user_id);
                if(!user)
                {
                    INFO("被设置管理员的用户不存在，被设置用户id: {}", user_id);
                    return false;
                }
            }
            {
                //新增管理员
                auto& dbHandler = tx.database();
                auto reHandler = rtx.redis();
                suiUserIdentityRole::UserIdentityRole useRole(dbHandler, reHandler, _cache_sync);
                //查询用户权限设置
                auto ret = useRole.select(user_id);
                if(!ret)
                {
                    //无权限设置，直接新增
                    //说明出现业务逻辑错误，这里直接新增
                    WARN("用户id: {} 没有设置权限, 这里新增管理用户权限", user_id);
                    useRole.insert(user_id, suiDataSql::roleType::roleTypeAdmin, suiDataSql::identityType::identityTypeAdmin);
                }
                else
                {
                    //存在
                    //判断是否是管理员
                    if(ret->getRoleType() != suiDataSql::roleType::roleTypeSuperAdmin)
                        ret->setRoleType(suiDataSql::roleType::roleTypeAdmin); //如果用户原本是超级管理员，本修改方法不起效果
                    ret->setIdentityType(suiDataSql::identityType::identityTypeAdmin);
                    //更新数据库
                    useRole.update(ret);
                }
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库修改用户信息异常： {}, 被修改用户id: {}", e.what(), user_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库修改用户信息异常： {}, 被修改用户id: {}", e.what(), user_id);
        }
        catch (...)
        {
            ERROR("修改用户信息异常， 未知异常， 被修改用户id: {}", user_id);
        }
        return false;
    }

    bool suiUserServerData::removeAdmin(const std::string& user_id)
    {
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();

            {
                //新增管理员
                suiUserIdentityRole::UserIdentityRole useRole(dbHandler, reHandler, _cache_sync);
                //查询用户权限设置
                auto ret = useRole.select(user_id);
                if(!ret)
                {
                    //无权限设置，直接新增
                    //说明出现业务逻辑错误，这里直接新增普通权限
                    WARN("用户id: {} 没有设置权限", user_id);
                    useRole.insert(user_id, suiDataSql::roleType::roleTypeNormal, suiDataSql::identityType::identityTypeNormal);
                }
                else
                {
                    //存在
                    if(ret->getRoleType() == suiDataSql::roleType::roleTypeSuperAdmin)
                    {
                        //为超级管理用户，无权移除管理员权限
                        return false;
                    }
                    //可以修改普通用户权限
                    ret->setIdentityType(suiDataSql::identityType::identityTypeNormal);
                    ret->setRoleType(suiDataSql::roleType::roleTypeNormal);
                    //更新数据库
                    useRole.update(ret);
                }
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库修改用户信息异常： {}， 被修改用户id: {}", e.what(), user_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库修改用户信息异常： {}， 被修改用户id: {}", e.what(), user_id);
        }
        catch (...)
        {
            ERROR("修改用户信息异常， 未知异常， 被修改用户id: {}", user_id);
        }
        return false;
    }

    bool suiUserServerData::setAdminProperty(const std::string& user_id
            , const std::string& nickname, const std::string& userMemo)
    {
        try
        {
            INFO("开始创建");
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            //
            {
                //开始修改用户基础管理员信息
                suiUserInformation::suiUserInformationOperation useUserInfo(tx, rtx, _cache_sync);
                useUserInfo.setAdminInfo(user_id, nickname, userMemo);
            }
            //
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库修改用户信息异常： {}， 被修改用户id: {}", e.what(), user_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库修改用户信息异常： {}， 被修改用户id: {}", e.what(), user_id);
        }
        catch (...)
        {
            ERROR("修改用户信息异常， 未知异常， 被修改用户id: {}", user_id);
        }
        return false;
    }

    suiDataSql::userInfoList::ptr suiUserServerData::getAdminList(suiDataSql::userStatus status, const suiDataSql::identityType identity
            , const suiDataSql::roleType role, const int page, const int page_size)
    {
        suiDataSql::userInfoList::ptr ret = nullptr;
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            {
                //查询管理员用户列表
                suiUserInformation::suiUserInformationOperation useUserInfo(tx, rtx, _cache_sync);
                ret = useUserInfo.getUserInfoListByTypeAndStatus(role, status, identity, page, page_size);
            }
            tx.commit();
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获取管理员用户列表异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库获取管理员用户列表异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("获取管理员用户列表异常， 未知异常");
        }
        return ret;
    }

    bool suiUserServerData::hasPermissionBySessionId(const std::string& session_id, const std::string& user_id
            , const suiDataSql::identityType identity, const suiDataSql::roleType role)
    {
        //判断是否大于等于指定权限
        try
        {
            //创建事务
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);

            //创建绑定线程的操作句柄
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();

            suiDataSql::suiSessionMeta::ptr session = nullptr;
            {
                //获取会话信息
                suiSession::sessionData sessionData(dbHandler, reHandler, _cache_sync);
                session = sessionData.selectBySessionId(session_id);
                if(!session || session->getUserId().null() || session->getUserId().get() != user_id)
                {
                    //不存在会话或者为临时会话
                    //不提交，回滚事务
                    return false;
                }
            }

            bool perret = false;
            {
                //判断是否拥有权限
                suiUserIdentityRole::UserIdentityRole useRole(dbHandler, reHandler, _cache_sync);
                perret = useRole.hasPermission(session->getUserId().get(), identity, role);
            }
            
            tx.commit();
            return perret;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获取判断会话用户权限异常： {}， 会话id: {}", e.what(), session_id);
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库获取判断会话用户权限异常： {}， 会话id: {}", e.what(), session_id);
        }
        catch (...)
        {
            ERROR("获取判断会话用户权限异常： {}， 未知异常 {}", session_id);
        }
        return false;
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

    //邮箱不存在操作
    bool suiUserServerData::_emailLoginNotExist(const std::string& session_id, const std::string& email, std::string* outUserId)
    {
        try
        {
            //创建新用户数据
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            //创建用户数据操作对象
            suiUserInformation::suiUserInformationOperation useOpteration(tx, rtx, _cache_sync);
            //创建新用户
            std::string user_id;
            while(1)
            {
                //创建用户id
                user_id = createRandomId();
                //通过id查询是否重复
                auto user = useOpteration.getUserInfoById(user_id);
                if(!user)
                {
                    //用户id不存在，创建新用户
                    useOpteration.insert(user_id, email);
                    if(outUserId)
                        *outUserId = user_id;
                    break;
                }
            }
            
            {
                //更新会话信息
                auto& dbHandler = tx.database();
                auto reHandler = rtx.redis();
                suiSession::sessionData sessionData(dbHandler, reHandler, _cache_sync);
                auto ret = sessionData.selectBySessionId(session_id);
                if(!ret)
                {
                    //会话不存在，出现错误
                    INFO("会话过期， 会话id： {}", session_id);
                    //tx.commit(); 会话过期，不进行提交，选择回滚
                    return false;
                }
                ret->setUserId(user_id);
                sessionData.update(*ret);
            }

            {
                //添加普通用户权限
                auto& dbHandler = tx.database();
                auto reHandler = rtx.redis();
                suiUserIdentityRole::UserIdentityRole useRole(dbHandler, reHandler, _cache_sync);
                useRole.insert(user_id, suiDataSql::roleType::roleTypeNormal, suiDataSql::identityType::identityTypeNormal);
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获邮箱登录异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库获邮箱登录异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("获邮箱登录异常");
        }
        
        return false;
    }


    //邮箱存在操作
    bool suiUserServerData::_emailLoginExist(const std::string& session_id, const std::string& user_id)
    {
        try
        {
            //更新会话信息
            odb::transaction tx(_mysql->begin());
            auto rtx = _redis->transaction(false, false);
            auto& dbHandler = tx.database();
            auto reHandler = rtx.redis();
            suiSession::sessionData sessionData(dbHandler, reHandler, _cache_sync);
            auto ret = sessionData.selectBySessionId(session_id);
            if(!ret)
            {
                //会话不存在
                INFO("会话过期， 会话id： {}", session_id);
                //tx.commit(); 会话过期，不进行提交，选择回滚
                return false;
            }
            //会话存在，绑定用户会话
            ret->setUserId(user_id);
            sessionData.update(*ret);
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获邮箱登录异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库获邮箱登录异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("获邮箱登录异常");
        }
        return false;
    }


}