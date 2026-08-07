#include "session.hpp"

namespace suiSession
{
    const std::string sessionData::_cache_prefix = "sui_session_";
    const int sessionData::_cache_expire = 3600;
    const std::string sessionData::_field_session_id = "session_id";
    const std::string sessionData::_field_user_id = "user_id"; 
    const std::string sessionData::_field_upload_time = "upload_time";  //上传时间字段

    sessionData::sessionData(odb::database& db, sw::redis::Redis& redis, suiRemoveCache::RemoveCache::ptr dataSync)
        : _db(db), _redis(redis), _dataSync(dataSync)
    {

    }

    //通过会话id获取缓存key
    std::string sessionData::getCacheKey(std::string sessionId)
    {
        return _cache_prefix + sessionId;
    }

    void sessionData::insertToDb(suiDataSql::suiSessionMeta& session)
    {
        //添加会话数据
        _db.persist(session);
    }
    //更新会话
    void sessionData::updateToDb(suiDataSql::suiSessionMeta& session)
    {
        //重新查询数据，然后更新
        auto session2 = selectToDb(session.getSessionId());
        if (session2 == nullptr) {
            ERROR("会话数据已被删除");
            return;
        }
        //更新用户id
        session2->setUserId(session.getUserId());
        //更新会话
        session2->setSessionId(session.getSessionId());
        _db.update(*session2);
    }
    //删除会话
    void sessionData::removeBysessionIdToDb(std::string sessionId)
    {
        _db.erase_query<suiDataSql::suiSessionMeta>(odb::query<suiDataSql::suiSessionMeta>::session_id == sessionId);
    }
    void sessionData::removeByUserIdToDb(std::string userId)
    {
        _db.erase_query<suiDataSql::suiSessionMeta>(odb::query<suiDataSql::suiSessionMeta>::user_id == userId);
    }
    //查询会话
    suiDataSql::suiSessionMeta::ptr sessionData::selectToDb(const std::string& sessionId)
    {
        auto ret = suiDataSql::suiSessionMeta::ptr(_db.query_one<suiDataSql::suiSessionMeta>(
            odb::query<suiDataSql::suiSessionMeta>::session_id == sessionId
        ));
        return ret;
    }

    //缓存操作
    //向redis缓存会话
    void sessionData::insertToRedis(suiDataSql::suiSessionMeta& session)
    {
        std::string key = getCacheKey(session.getSessionId());
        std::unordered_map<std::string, std::string> _data;
        _data[_field_session_id] = session.getSessionId();
        if(!session.getUserId().null())
        {
            //用户id不为空
            _data[_field_user_id] = session.getUserId().get();
        }
        //上传时间
        _data[_field_upload_time] = std::to_string(session.getUploadTime());

        _redis.hmset(key, _data.begin(), _data.end());
        _redis.expire(key, std::chrono::seconds(_cache_expire));
    }
    //更新redis会话
    void sessionData::updateToRedis(suiDataSql::suiSessionMeta& session)
    {
        insertToRedis(session);
    }
    //删除redis会话
    void sessionData::removeBySessionIdToRedis(std::string sessionId)
    {
        publishDeleteMessage({getCacheKey(sessionId)});
    }
    //根据会话ID查询会话
    suiDataSql::suiSessionMeta::ptr sessionData::selectToRedis(const std::string& sessionId)
    {
        //构造key
        std::string key = getCacheKey(sessionId);
        //获取缓存数据
        std::unordered_map<std::string, std::string> _data;
        _redis.hgetall(key, std::inserter(_data, _data.begin()));
        if(_data.empty())
            return nullptr;
        //构造输出对象
        std::shared_ptr<suiDataSql::suiSessionMeta> ret = std::make_shared<suiDataSql::suiSessionMeta>();
        if(_data.find(_field_session_id) != _data.end())
            ret->setSessionId(_data[_field_session_id]);

        if(_data.find(_field_user_id) != _data.end())
            ret->setUserId(_data[_field_user_id]);

        ret->setUploadTime(std::stoull(_data[_field_upload_time]));
        return ret;
    }

    //延迟发布会话删除消息
    void sessionData::publishDeleteMessage(const std::vector<std::string>& sessionIdList)
    {
        if(sessionIdList.empty())
            return;
        //发布消息
        _dataSync->syncCache(sessionIdList);
    }

    void sessionData::insert(suiDataSql::suiSessionMeta& session)
    {
        insertToDb(session);
    }
    //更新会话
    void sessionData::update(suiDataSql::suiSessionMeta& session)
    {
        updateToDb(session);
        publishDeleteMessage({getCacheKey(session.getSessionId())});
    }
    //删除会话
    void sessionData::removeBySessionId(std::string sessionId)
    {
        removeBysessionIdToDb(sessionId);
        publishDeleteMessage({getCacheKey(sessionId)});
    }
    std::shared_ptr<std::vector<suiDataSql::suiSessionMeta::ptr>> sessionData::getDataListByUserId(std::string userId)
    {
        odb::result<suiDataSql::suiSessionPtr> res(_db.query<suiDataSql::suiSessionPtr>(odb::query<suiDataSql::suiSessionPtr>::user_id == userId));
        std::shared_ptr<std::vector<suiDataSql::suiSessionMeta::ptr>> sessionIdList = std::make_shared<std::vector<suiDataSql::suiSessionMeta::ptr>>();
        for(auto it = res.begin(); it != res.end(); it++)
        {
            (*sessionIdList).push_back(it->session);
        }

        return sessionIdList;
    }

    void sessionData::removeByUserId(std::string userId)
    {
        //获取所有数据段
        auto dataList = getDataListByUserId(userId);
        if(dataList->empty())
            return;
        //遍历删除所有会话缓存
        std::vector<std::string> sessionIdList;
        for(std::shared_ptr<suiDataSql::suiSessionMeta> it : (*dataList))
        {
            DEBUG("删除用户： {} 会话： {}", userId, it->getSessionId());
            sessionIdList.push_back(getCacheKey(it->getSessionId()));
        }
        //发布删除消息
        publishDeleteMessage(sessionIdList);
        //删除数据库
        removeByUserIdToDb(userId);

    }

    //根据会话ID查询会话信息
    suiDataSql::suiSessionMeta::ptr sessionData::selectBySessionId(const std::string& sessionId)
    {
        auto curSession = selectToRedis(sessionId);
        if(curSession != nullptr)
            return curSession; //缓存命中，直接返回
        curSession =  selectToDb(sessionId);
        if(curSession != nullptr) 
            insertToRedis(*curSession);
        return curSession;
    }   
    




}