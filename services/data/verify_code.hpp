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
        suiVerifyCodeOperation(sw::redis::Transaction& cache_tx, suiRemoveCache::RemoveCache::ptr removeCachePtr);

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
        sw::redis::Transaction& _cache_tx;
        //缓存延时删除对象
        suiRemoveCache::RemoveCache::ptr _removeCachePtr;

    };
}