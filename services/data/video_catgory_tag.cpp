#include "video_catgory_tag.hpp"

namespace suiVideocatgoryTag
{
    videocatgoryTag::videocatgoryTag(odb::database& db)
        : _mysql(db)
    {

    }

    void videocatgoryTag::addTag(std::string& tag_description)
    {
        suiDataSql::suiTagMeta curtag;
        curtag.setTagDescription(tag_description);
        _mysql.persist(curtag);
    }

    suiDataSql::suiTagMeta::ptr videocatgoryTag::selectTag(const std::string& tag_description)
    {
        auto ret = suiDataSql::suiTagMeta::ptr(_mysql.query_one<suiDataSql::suiTagMeta>(
            odb::query<suiDataSql::suiTagMeta>::tag_description == tag_description
        ));
        return ret;
    }

    suiDataSql::suiTagMeta::ptr videocatgoryTag::selectTag(long long tag_id)
    {
        auto ret = suiDataSql::suiTagMeta::ptr(_mysql.query_one<suiDataSql::suiTagMeta>(
            odb::query<suiDataSql::suiTagMeta>::tag_id == tag_id
        ));
        return ret;
    }

    void videocatgoryTag::removeTag(long long tag_id)
    {
        _mysql.erase_query<suiDataSql::suiTagMeta>(odb::query<suiDataSql::suiTagMeta>::tag_id == tag_id);
    }

    void videocatgoryTag::removeTag(const std::string& description)
    {
        _mysql.erase_query<suiDataSql::suiTagMeta>(odb::query<suiDataSql::suiTagMeta>::tag_description == description);
    }

    void videocatgoryTag::addVideoTag(const std::string& video_id, long long tag_id)
    {
        suiDataSql::suiVideoTagMeta cur;
        cur.setTagId(tag_id);
        cur.setVideoId(video_id);
        _mysql.persist(cur);
    }

    suiDataSql::suiVideoTagMeta::ptr videocatgoryTag::selectVideoTag(const std::string& video_id)
    {
        auto ret = suiDataSql::suiVideoTagMeta::ptr(_mysql.query_one<suiDataSql::suiVideoTagMeta>(
            odb::query<suiDataSql::suiVideoTagMeta>::video_id == video_id
        ));
        return ret;
    }

    suiDataSql::suiVideoTagMeta::ptr videocatgoryTag::selectVideoTag(long long tag_id)
    {
        auto ret = suiDataSql::suiVideoTagMeta::ptr(_mysql.query_one<suiDataSql::suiVideoTagMeta>(
            odb::query<suiDataSql::suiVideoTagMeta>::tag_id == tag_id
        ));
        return ret;
    }

    suiDataSql::suiVideoTagMeta::ptr videocatgoryTag::selectVideoTag(const std::string& video_id, long long tag_id)
    {
        auto ret = suiDataSql::suiVideoTagMeta::ptr(_mysql.query_one<suiDataSql::suiVideoTagMeta>(
            odb::query<suiDataSql::suiVideoTagMeta>::tag_id == tag_id
            && odb::query<suiDataSql::suiVideoTagMeta>::video_id == video_id
        ));
        return ret;
    }

    void videocatgoryTag::removeVideoTag(const std::string& video_id, long long tag_id)
    {
        _mysql.erase_query<suiDataSql::suiVideoTagMeta>(odb::query<suiDataSql::suiVideoTagMeta>::tag_id == tag_id
            && odb::query<suiDataSql::suiVideoTagMeta>::video_id == video_id);
    }

    void videocatgoryTag::removeVideoTag(const std::string& video_id)
    {
        _mysql.erase_query<suiDataSql::suiVideoTagMeta>(odb::query<suiDataSql::suiVideoTagMeta>::video_id == video_id);
    }

    suiDataSql::suiVideoListByTag::ptr videocatgoryTag::select(const std::string& description, int page, int pageSize)
    {
        auto q = odb::query<suiDataSql::videoTagView>::suiTagMeta::tag_description == description;
        q += suiDataSql::SqlPaginationUtil::buildPageClause(pageSize, page) + suiDataSql::SqlPaginationUtil::buildOrderByClause("primaryKey");
        auto ret = _mysql.query<suiDataSql::videoTagView>(q);
        auto total = _mysql.query_one<suiDataSql::videoTagTotal>(q);
        suiDataSql::suiVideoListByTag::ptr out = std::make_shared<suiDataSql::suiVideoListByTag>();
        for(auto& itemPtr : ret)
            out->list.push_back(*itemPtr.videoMeta);
        out->total = total->total;
        return out;
    }

    suiDataSql::suiGetTagsByVideoRsp::ptr videocatgoryTag::select(const std::string& video_id)
    {
        auto q = odb::query<suiDataSql::tagVideoView>::suiVideoTagMeta::video_id == video_id;
        auto ret = _mysql.query<suiDataSql::tagVideoView>(q);
        suiDataSql::suiGetTagsByVideoRsp::ptr out = std::make_shared<suiDataSql::suiGetTagsByVideoRsp>();
        for(auto& itemPtr : ret)
            out->list.push_back(*itemPtr.tags);
        return out;
    }
    
}