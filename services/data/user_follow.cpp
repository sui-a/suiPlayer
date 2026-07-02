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

    }

    //删
    void suiFollow::remove(const std::string& user_id, const std::string& follow_user_id)
    {

    }

    //查
    bool suiFollow::judgment(const std::string& user_id, const std::string& follow_user_id)
    {
        
    }




}