#include "video_like.hpp"

namespace suiVideoLike
{
    videoLike::videoLike(odb::database& db)
        : _mysql(db)
    {

    }

    void videoLike::insert(const std::string& video_id, const std::string& user_id)
    {
        //创建
        suiDataSql::suiUserLikeMeta like;
        like.setUserId(user_id);
        like.setVideoId(video_id);
        //插入
        insert(like);
    }

    void videoLike::insert(suiDataSql::suiUserLikeMeta& like)
    {
        //插入
        _mysql.persist(like);
    }
    
    void videoLike::remove(const std::string& video_id, const std::string& user_id)
    {
        //删除
        _mysql.erase_query<suiDataSql::suiUserLikeMeta>(odb::query<suiDataSql::suiUserLikeMeta>::video_id == video_id 
            && odb::query<suiDataSql::suiUserLikeMeta>::user_id == user_id);
    }

    void videoLike::removeByVideoId(const std::string& video_id)
    {
        _mysql.erase_query<suiDataSql::suiUserLikeMeta>(odb::query<suiDataSql::suiUserLikeMeta>::video_id == video_id);
    }

    void videoLike::removeByUserId(const std::string& user_id)
    {
        _mysql.erase_query<suiDataSql::suiUserLikeMeta>(odb::query<suiDataSql::suiUserLikeMeta>::user_id == user_id);
    }
    
    bool videoLike::isLike(const std::string& video_id, const std::string& user_id)
    {
        //先查询 再判断
        auto like = suiDataSql::suiUserLikeMeta::ptr(_mysql.query_one<suiDataSql::suiUserLikeMeta>(odb::query<suiDataSql::suiUserLikeMeta>::video_id == video_id 
            && odb::query<suiDataSql::suiUserLikeMeta>::user_id == user_id));
        return like != nullptr;
    }

    suiDataSql::suiUserLikeMetaList::ptr videoLike::select(const std::string& user_id, int page, int pageSize)
    {
        //开始查询
        auto q = odb::query<suiDataSql::suiUserLikeMeta>::user_id == user_id;
        auto total = _mysql.query_one<suiDataSql::suiUserLikeCountView>(q);
        q +=  " " + suiDataSql::SqlPaginationUtil::buildPageClause(pageSize, page);  //定义输出顺序以及内容
        auto ret = _mysql.query<suiDataSql::suiUserLikeMeta>(q);
        suiDataSql::suiUserLikeMetaList::ptr out = std::shared_ptr<suiDataSql::suiUserLikeMetaList>();
        //进行遍历
        for(auto& itemPtr : ret)
            out->list.push_back(itemPtr);
        out->total = total->count;
        return out;
    }

    size_t videoLike::countByUserId(const std::string& user_id)
    {
        //开始查询
        auto q = odb::query<suiDataSql::suiUserLikeMeta>::user_id == user_id;
        auto total = _mysql.query_one<suiDataSql::suiUserLikeCountView>(q);
        return total->count;
    }

    size_t videoLike::countByVideoId(const std::string& video_id)
    {
        //开始查询
        auto q = odb::query<suiDataSql::suiUserLikeMeta>::video_id == video_id;
        auto total = _mysql.query_one<suiDataSql::suiUserLikeCountView>(q);
        return total->count;
    }

}