#pragma once
#include <memory>
#include <suiScaffold/suiodb.hpp>
#include <odb/query.hxx>
#include "data.hpp"
#include "data-odb.hxx"

namespace suiVideocatgoryTag
{
    class videocatgoryTag
    {
    public:
        using ptr = std::shared_ptr<videocatgoryTag>;
    
        //只通过数据库处理
        videocatgoryTag(odb::database& db);

        //添加新标签
        void addTag(const std::string& tag_description);
        //通过标签描述索引标签
        suiDataSql::suiTagMeta::ptr selectTag(const std::string& tag_description);
        suiDataSql::suiTagMeta::ptr selectTag(long long tag_id);
        suiDataSql::videoTagList selectAllTag();

        //删除标签
        void removeTag(long long tag_id);
        void removeTag(const std::string& description);

        //视频操作
        void addVideoTag(const std::string& video_id, long long tag_id);
        std::vector<long long> selectVideoTag(const std::string& video_id);
        std::vector<std::string> selectVideoTag(long long tag_id);
        suiDataSql::suiVideoTagMeta::ptr selectVideoTag(const std::string& video_id, long long tag_id);
        void removeVideoTag(const std::string& video_id, long long tag_id);
        void removeVideoTag(const std::string& video_id);

        //根据标签查询视频
        suiDataSql::suiVideoListByTag::ptr select(const std::string& description, int page, int pageSize);
        //查询视频标签 //没必要控制区间
        suiDataSql::suiGetTagsByVideoRsp::ptr select(const std::string& video_id);
    private:
        odb::database& _mysql;
    };

}