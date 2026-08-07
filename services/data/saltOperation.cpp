#include "saltOperation.hpp"

namespace suiSalt
{
    suiSaltOperation::suiSaltOperation(odb::database& db, suiRemoveCache::RemoveCache::ptr dataSync)
        : _db(db), _dataSync(dataSync)
    {

    }

    void suiSaltOperation::insert(suiDataSql::suiSaltMeta& salt)
    {
        insertToDb(salt);
    }

    void suiSaltOperation::insert(const std::string& userId, const std::string& salt)
    {
        //进行构建
        suiDataSql::suiSaltMeta curmeta;
        curmeta.setSalt(salt);
        curmeta.setUserId(userId);
        insertToDb(curmeta);
    }

    void suiSaltOperation::update(suiDataSql::suiSaltMeta& salt)
    {
        updateToDb(salt);
    }

    suiDataSql::suiSaltMeta::ptr suiSaltOperation::selectByUserId(const std::string& userId)
    {
        return selectByUserIdToDb(userId);
    }

    void suiSaltOperation::removeByUserId(const std::string& userId)
    {
        removeByUserIdToDb(userId);
    }

    //数据库操作
    void suiSaltOperation::insertToDb(suiDataSql::suiSaltMeta& salt)
    {
        _db.persist(salt);
    }

    void suiSaltOperation::updateToDb(suiDataSql::suiSaltMeta& salt)
    {
        //线进行索引，然后更新
        auto ret = suiDataSql::suiSaltMeta::ptr(_db.query_one<suiDataSql::suiSaltMeta>(
            odb::query<suiDataSql::suiSaltMeta>::user_id == salt.getUserId()
        ));
        if(!ret)
        {
            WARN("用户盐属性不存在，用户id： {}", salt.getUserId());
            return;
        }
        //属性存在，进行更新
        ret->setSalt(salt.getSalt());
        _db.update(*ret);
    }

    suiDataSql::suiSaltMeta::ptr suiSaltOperation::selectByUserIdToDb(const std::string& userId)
    {
        //进行索引
        auto ret = suiDataSql::suiSaltMeta::ptr(_db.query_one<suiDataSql::suiSaltMeta>(
            odb::query<suiDataSql::suiSaltMeta>::user_id == userId
        ));
        return ret;
    }

    void suiSaltOperation::removeByUserIdToDb(const std::string& userId)
    {
        //删除
        _db.erase_query<suiDataSql::suiSaltMeta>(odb::query<suiDataSql::suiSaltMeta>::user_id == userId);
    }







}