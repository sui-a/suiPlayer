#pragma once
#include <suiScaffold/suiElastic.hpp>
#include <suiScaffold/log.h>
#include "data.hpp"
#include "data-odb.hxx"

namespace suiVideoSearch 
{
    class videoSearch
    {
    public:
        using ptr = std::shared_ptr<videoSearch>;
        videoSearch(std::shared_ptr<suies::esClient> clientPtr);

        //初始化es索引
        void initIndex();
        //插入视频信息
        bool insertVideo(const std::string& videoId, const suiDataSql::videoStatus videoStatus
        , const std::string& videoTitle, const std::string& videoDesc, const int64_t releaseTime);

        bool updateVideo(const std::string& videoId, const suiDataSql::videoStatus videoStatus
            , const std::string& videoTitle, const std::string& videoDesc
            , const int64_t playCount, const int64_t likeCount);

        //只针对某些字段进行更新
        bool updateVideo(const std::string& curVideoId, const suiDataSql::videoStatus curVideoStatus);
        bool updateVideo(const std::string& curVideoId, const int64_t curPlayCount, const int64_t curLikeCount);

        //删除某个视频
        bool removeVideo(const std::string& curVideoId);

        //进行模糊搜索
        std::vector<std::string> searchVideo(const std::string& keyword, const int from, const int size, size_t& totalCount);
        //这个接口默认为降序
        std::vector<std::string> searchVideoOrderByPlayCount(const std::string& keyword, const int from, const int size, size_t& totalCount);
        std::vector<std::string> searchVideoOrderByLikeCount(const std::string& keyword, const int from, const int size, size_t& totalCount);
        std::vector<std::string> searchVideoOrderByReleaseTime(const std::string& keyword, const int from, const int size, size_t& totalCount);
    private:
        //解析es返回的结果
        void extractVideoIds(const std::string& jsonStr, std::vector<std::string>& videoIds, size_t& totalCount);


    private:
        std::shared_ptr<suies::esClient> _clientPtr;
        
        //es中的索引名称
        static const std::string indexName;
        //es中的字段名称
        static const std::string videoIdField;
        static const std::string videoStatusField;
        static const std::string videoTitleField;
        static const std::string videoDescField;
        static const std::string releaseTimeField;
        static const std::string playCountField;
        static const std::string likeCountField;
    };
}