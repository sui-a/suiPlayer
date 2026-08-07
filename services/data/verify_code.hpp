#pragma once
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/log.h>
#include "removeCache.hpp"
#include "data.hpp"
#include "data-odb.hxx"


namespace suiVerifyCode
{
    class suiVerifyCodeOperation
    {
    public:
        //验证码操作类构造函数
        suiVerifyCodeOperation(sw::redis::Redis& redis, suiRemoveCache::RemoveCache::ptr removeCachePtr);

        //新增验证码
        void insert(const std::string& code_id, const std::string& session_id, const std::string& code);

        //获取验证码
        suiDataSql::suiVirfyCoder::ptr get(const std::string& code_id);

        //删除验证码
        void remove(const std::string& code_id);

    private:
        //获取验证码缓存key
        std::string getCacheKey(const std::string& code_id);

    private:
        //缓存事务对象
        sw::redis::Redis& _redis;
        //缓存延时删除对象
        suiRemoveCache::RemoveCache::ptr _removeCachePtr;

        //有效时间
        static const int _expire_time;

        //验证码缓存key前缀
        static const std::string _cache_key;
        static const std::string _session_key;
        static const std::string _code_key;

    };
}