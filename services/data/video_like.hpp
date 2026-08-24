#pragma once
#include <memory>
#include <suiScaffold/suiodb.hpp>
#include <odb/query.hxx>
#include "data.hpp"
#include "data-odb.hxx"

namespace suiVideoLike
{
    class videoLike
    {
    public:
        using ptr = std::shared_ptr<videoLike>;

        //只通过数据库处理
        videoLike(odb::database& db);

        //新增
        void insert(const std::string& video_id, const std::string& user_id);
        void insert(suiDataSql::suiUserLikeMeta& like);
        //删除
        void remove(const std::string& video_id, const std::string& user_id);
        void removeByVideoId(const std::string& video_id);
        void removeByUserId(const std::string& user_id);
        //判断
        bool isLike(const std::string& video_id, const std::string& user_id);
        //返回点赞视频列表
        suiDataSql::suiUserLikeMetaList::ptr select(const std::string& user_id, int page, int pageSize);
        //返回用户点赞的视频的数量
        size_t countByUserId(const std::string& user_id);
        //返回视频被点赞的数量
        size_t countByVideoId(const std::string& video_id);

    private:
        odb::database& _mysql;
    };
}