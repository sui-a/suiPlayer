#include "verify_code.hpp"


namespace suiVerifyCode
{
    const std::string suiVerifyCodeOperation::_cache_key = "verify_code_";
    const std::string suiVerifyCodeOperation::_session_key = "verify_code_session_id";
    const std::string suiVerifyCodeOperation::_code_key = "verify_code_code";
    const int suiVerifyCodeOperation::_expire_time = 5 * 60;

    suiVerifyCodeOperation::suiVerifyCodeOperation(sw::redis::Redis& redis, suiRemoveCache::RemoveCache::ptr removeCachePtr)
        : _redis(redis), _removeCachePtr(removeCachePtr)
    {

    }

    void suiVerifyCodeOperation::insert(const std::string& code_id, const std::string& session_id, const std::string& code)
    {
        //获取key
        std::string key = getCacheKey(code_id);

        //创建数据
        std::unordered_map<std::string, std::string> data;
        data[_session_key] = session_id;
        data[_code_key] = code;
        
        //添加进缓存，并设置五分钟的时间
        _redis.hmset(key, data.begin(), data.end());
        _redis.expire(key, std::chrono::seconds(_expire_time));
    }

    suiDataSql::suiVirfyCoder::ptr suiVerifyCodeOperation::get(const std::string& code_id)
    {
        //直接构造key获取
        std::string key = getCacheKey(code_id);

        std::unordered_map<std::string, std::string> data;
        _redis.hgetall(key, std::inserter(data, data.begin()));
        if(data.empty())
        {
            return nullptr;//为空直接返回空指针
        }
        //不为空，构造数据对象
        suiDataSql::suiVirfyCoder::ptr out = std::make_shared<suiDataSql::suiVirfyCoder>();
        out->_code_id = code_id;
        out->_session_id = data[_session_key];
        out->_code = data[_code_key];
        return out;
    }

    void suiVerifyCodeOperation::remove(const std::string& code_id)
    {
        //构造key
        std::string key = getCacheKey(code_id);
        //发布异步删除指令
        _removeCachePtr->syncCache({key});
    }

    std::string suiVerifyCodeOperation::getCacheKey(const std::string& code_id)
    {
        return  _cache_key + code_id;
    }
}


