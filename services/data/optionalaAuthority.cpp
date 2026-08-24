#include "optionalaAuthority.hpp"

namespace suiOptionalaAuthority
{
    const std::string OptionalaAuthority::_cacheKeyPrefix = "optionalaAuthority:";
    const std::string OptionalaAuthority::_cacheKeyField = "operationMeta";
    const int OptionalaAuthority::_defaultCacheExpire = 60 * 10;

    OptionalaAuthority::OptionalaAuthority(odb::database& db, sw::redis::Redis& redis)
        : _db(db), _redis(redis)
    {

    }

    suiDataSql::suiRoleOperationMeta::ptr OptionalaAuthority::select(const std::string& operation_url)
    {
        //先从缓存查询
        auto ret = selectFromCache(operation_url);
        if(ret)
            return ret;
        //从数据库查询
        ret = selectFromDb(operation_url);
        if(ret)
            addToCache(ret);
        return ret;
    }
    
    suiDataSql::suiRoleOperationMeta::ptr OptionalaAuthority::selectFromDb(const std::string& operation_url)
    {
        auto ret = suiDataSql::suiRoleOperationMeta::ptr(_db.query_one<suiDataSql::suiRoleOperationMeta>(
            odb::query<suiDataSql::suiRoleOperationMeta>::operation_url == operation_url));
        return ret;
    }
    
    void OptionalaAuthority::addToCache(suiDataSql::suiRoleOperationMeta::ptr operationMeta)
    {
        //添加进缓存
        std::unordered_map<std::string, std::string> cacheMap;
        cacheMap[_cacheKeyField] = suiDataSql::suiRoleType::roleTypeToString(operationMeta->getRoleType());
        _redis.hmset(getCacheKey(operationMeta->getOperationUrl()), cacheMap.begin(), cacheMap.end());
        _redis.expire(getCacheKey(operationMeta->getOperationUrl()), std::chrono::seconds(_defaultCacheExpire));
    }
    
    suiDataSql::suiRoleOperationMeta::ptr OptionalaAuthority::selectFromCache(const std::string& operation_url)
    {
        std::unordered_map<std::string, std::string> _data;
        _redis.hgetall(getCacheKey(operation_url), std::inserter(_data, _data.begin()));
        if(_data.empty())
        {
            return nullptr;
        }
        //构造输出
        auto ret = std::make_shared<suiDataSql::suiRoleOperationMeta>();
        ret->setOperationUrl(operation_url);
        ret->setRoleType(suiDataSql::suiRoleType::stringToRoleType(_data[_cacheKeyField]));
        return ret;
    }

    std::string OptionalaAuthority::getCacheKey(const std::string& operation_url)
    {
        return _cacheKeyPrefix + operation_url;
    }

}
