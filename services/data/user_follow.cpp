#include "user_follow.hpp"

namespace suiUserFollow
{
    suiFollow::suiFollow(odb::database& db)
        :_db(db)
    {

    }

    //增
    void suiFollow::insert(const std::string& user_id, const std::string& follow_user_id)
    {
        suiDataSql::suiUserFollowMeta curMeta;
        curMeta.setUserId(user_id);
        curMeta.setFollowUserId(follow_user_id);
        _db.persist(curMeta);
    }

    //删
    void suiFollow::remove(const std::string& user_id, const std::string& follow_user_id)
    {
        _db.erase_query<suiDataSql::suiUserFollowMeta>(odb::query<suiDataSql::suiUserFollowMeta>::user_id == user_id
                && odb::query<suiDataSql::suiUserFollowMeta>::follow_user_id == follow_user_id);
    }

    //查
    bool suiFollow::judgment(const std::string& user_id, const std::string& follow_user_id)
    {
        return selectToDb(user_id, follow_user_id) != nullptr;
    }


    suiDataSql::suiUserFollowMeta::ptr suiFollow::selectToDb(const std::string& user_id, const std::string& follow_user_id)
    {
        auto ret = suiDataSql::suiUserFollowMeta::ptr(_db.query_one<suiDataSql::suiUserFollowMeta>(
            odb::query<suiDataSql::suiUserFollowMeta>::user_id == user_id
            && odb::query<suiDataSql::suiUserFollowMeta>::follow_user_id == follow_user_id
        ));
        return ret;
    }

    std::vector<suiDataSql::suiUserFollowMeta> suiFollow::selectToDbByUserId(const std::string& user_id)
    {
        odb::result<suiDataSql::suiUserFollowMeta> ret = _db.query<suiDataSql::suiUserFollowMeta>(
            odb::query<suiDataSql::suiUserFollowMeta>::user_id == user_id
        );

        //递归获取
        std::vector<suiDataSql::suiUserFollowMeta> followList;
        for (auto& meta : ret)
        {
            followList.push_back((meta));
        }
        
        return followList;
    }

    std::vector<suiDataSql::suiUserFollowMeta> suiFollow::selectToDbByFollowUserId(const std::string& follow_user_id)
    {
        odb::result<suiDataSql::suiUserFollowMeta> ret = _db.query<suiDataSql::suiUserFollowMeta>(
            odb::query<suiDataSql::suiUserFollowMeta>::follow_user_id == follow_user_id
        );

        //递归获取
        std::vector<suiDataSql::suiUserFollowMeta> followList;
        for (auto& meta : ret)
        {
            followList.push_back(meta);
        }
        
        return followList;
    }



}