#include "video_search.hpp"

namespace suiVideoSearch 
{
    const std::string videoSearch::indexName = "videos";
    const std::string videoSearch::videoIdField = "video_id";
    const std::string videoSearch::videoStatusField = "video_status";
    const std::string videoSearch::videoTitleField = "video_title";
    const std::string videoSearch::videoDescField = "video_desc";
    const std::string videoSearch::releaseTimeField = "release_time";
    const std::string videoSearch::playCountField = "play_count";
    const std::string videoSearch::likeCountField = "like_count";


    videoSearch::videoSearch(std::shared_ptr<suies::esClient> clientPtr)
        : _clientPtr(clientPtr)
    {

    }

    void videoSearch::initIndex()
    {
        suies::suiIndexer indexer(indexName);
        {
            auto map = indexer.getMap(); 
            auto pro = map->setDynamic(false).getProperties();

            // {
            //     //创建视频id字段
            //     auto videoId = std::make_shared<suies::esBase>(videoIdField);
            //     videoId->add("type", "keyword");
            //     videoId->add("analyzer", "ik_max_word");
            //     videoId->add("index", true);
            // }
            //不创建视频id字段，视频id作为文档id
            {
                //创建视频状态字段
                auto videoStatus = std::make_shared<suies::esBase>(videoStatusField);
                videoStatus->add("type", "integer");
                videoStatus->add("index", true);
                pro->addField(videoStatus);
            }
            {
                //视频标题
                auto videoTitle = std::make_shared<suies::esBase>(videoTitleField);
                videoTitle->add("type", "text");
                videoTitle->add("analyzer", "ik_max_word");
                videoTitle->add("index", true);
                pro->addField(videoTitle);
            }
            {
                //视频描述
                auto videoDesc = std::make_shared<suies::esBase>(videoDescField);
                videoDesc->add("type", "text");
                videoDesc->add("analyzer", "ik_max_word");
                videoDesc->add("index", true);
                pro->addField(videoDesc);
            }
            {
                //发布的时间戳
                auto releaseTime = std::make_shared<suies::esBase>(releaseTimeField);
                releaseTime->add("type", "long");
                releaseTime->add("index", true);
                pro->addField(releaseTime);
            }
            {
                //播放数
                auto playCount = std::make_shared<suies::esBase>(playCountField);
                playCount->add("type", "long");
                playCount->add("index", true);
                pro->addField(playCount);
            }
            {
                //点赞数量
                auto likeCount = std::make_shared<suies::esBase>(likeCountField);
                likeCount->add("type", "long");
                likeCount->add("index", true);
                pro->addField(likeCount);
            }
        }

        //开始创建
        _clientPtr->create(indexer);
    }
    

    bool videoSearch::insertVideo(const std::string& videoId, const suiDataSql::videoStatus videoStatus
        , const std::string& videoTitle, const std::string& videoDesc, const int64_t releaseTime)
    {
        //初次插入，点赞播放量设置为0
        suies::suiInserter indexer(indexName);
        indexer.setId(videoId);
        indexer.add(videoStatusField, suiDataSql::videoStatusUtil::getIntFromEnum(videoStatus));
        indexer.add(videoTitleField, videoTitle);
        indexer.add(videoDescField, videoDesc);
        indexer.add(releaseTimeField, releaseTime);
        indexer.add(playCountField, 0);
        indexer.add(likeCountField, 0);
        if(_clientPtr->insert(indexer))
            return true;
        return false;
    }

    bool videoSearch::updateVideo(const std::string& videoId, const suiDataSql::videoStatus videoStatus
        , const std::string& videoTitle, const std::string& videoDesc
        , const int64_t playCount, const int64_t likeCount)
    {
        if(videoId.empty())
            return false;
        suies::suiUpdate update(indexName, videoId);
        {
            auto doc = update.getDoc();
            //视频标题
            if(!videoTitle.empty())
                doc->add(videoTitleField, videoTitle);
            //视频描述
            if(!videoDesc.empty())
                doc->add(videoDescField, videoDesc);
            //播放量
            if(playCount != -1)
                doc->add(playCountField, playCount);
            //点赞量
            if(likeCount != -1)
                doc->add(likeCountField, likeCount);
            //视频状态 (如：0下架/删除)
            if(videoStatus != suiDataSql::videoStatus::videoStatusUnknow)
                doc->add(videoStatusField, suiDataSql::videoStatusUtil::getIntFromEnum(videoStatus));
        }
        //进行发送
        return _clientPtr->update(update); //不在乎是否发送成功
    }

    bool videoSearch::updateVideo(const std::string& curVideoId, const suiDataSql::videoStatus curVideoStatus)
    {
        if(curVideoId.empty())
            return false;
        suies::suiUpdate update(indexName, curVideoId);
        {
            auto doc = update.getDoc();
            if(curVideoStatus != suiDataSql::videoStatus::videoStatusUnknow)
                doc->add(videoStatusField, suiDataSql::videoStatusUtil::getIntFromEnum(curVideoStatus));
        }
        //进行发送
        return _clientPtr->update(update); //不在乎是否发送成功
    }

    bool videoSearch::updateVideo(const std::string& curVideoId, const int64_t curPlayCount, const int64_t curLikeCount)
    {
        if(curVideoId.empty())
            return false;
        suies::suiUpdate update(indexName, curVideoId);
        {
            auto doc = update.getDoc();
            if(curPlayCount != -1)
                doc->add(playCountField, curPlayCount);
            // 6. 点赞量
            if(curLikeCount != -1)
                doc->add(likeCountField, curLikeCount);
        }
        //进行发送
        return _clientPtr->update(update); //不在乎是否发送成功
    }

    bool videoSearch::removeVideo(const std::string& curVideoId)
    {
        suies::suiDeleter del(indexName, curVideoId);
        return _clientPtr->remove(del);
    }

    std::vector<std::string> videoSearch::searchVideo(const std::string& keyword, const int from, const int size, size_t& totalCount)
    {
        //默认权重
        suies::suiSearch search(indexName);
        {
            //设置范围
            search.setFrom(from);
            search.setSize(size);
        }
        {
            //设置筛选流程
            auto query = search.query();
            auto boolkey = query->QBool();
            auto must = boolkey->must();
            {
                auto multi= must->multi_match();
                multi->setQuery(keyword);
                multi->appendField(videoTitleField + "^3"); //标题优先
                multi->appendField(videoDescField + "^2"); //描述次之
                multi->setOperator("or");
                multi->setFuzziness("AUTO");
                multi->setType("best_fields");
            }
            suies::suiFilter::Ptr filter = boolkey->filter();
            {
                //添加
                auto term = filter->getFilterTerm(videoStatusField);
                term->setValue(4);
            }
        }
        {
            //默认接口，只对时间进行降序排列
            auto sort = search.sort();
            sort->addOrder("_score", suies::sortType::descending);
            sort->addOrder(releaseTimeField, suies::sortType::descending);
        }
        {
            //设置只返回
            search.setSource();
        }

        auto ret = _clientPtr->search(search);
        std::vector<std::string> out;
        if(ret.has_value())
        {
            //开始提取结果并返回
            extractVideoIds(ret.value(), out, totalCount);
        }
        return out;
    }
    std::vector<std::string> videoSearch::searchVideoOrderByPlayCount(const std::string& keyword, const int from, const int size, size_t& totalCount)
    {
        //默认权重
        suies::suiSearch search(indexName);
        {
            //设置范围
            search.setFrom(from);
            search.setSize(size);
        }
        {
            //设置筛选流程
            auto query = search.query();
            auto boolkey = query->QBool();
            auto must = boolkey->must();
            {
                auto multi= must->multi_match();
                multi->setQuery(keyword);
                multi->appendField(videoTitleField + "^3"); //标题优先
                multi->appendField(videoDescField + "^2"); //描述次之
                multi->setOperator("or");
                multi->setFuzziness("AUTO");
                multi->setType("best_fields");
            }
            suies::suiFilter::Ptr filter = boolkey->filter();
            {
                //添加
                auto term = filter->getFilterTerm(videoStatusField);
                term->setValue(4);
            }
        }
        {
            //默认接口，只对时间进行降序排列
            auto sort = search.sort();
            sort->addOrder(playCountField, suies::sortType::descending);
            sort->addOrder("_score", suies::sortType::descending);
            sort->addOrder(releaseTimeField, suies::sortType::descending);
        }
        {
            //设置只返回
            search.setSource();
        }

        auto ret = _clientPtr->search(search);
        std::vector<std::string> out;
        if(ret.has_value())
        {
            //开始提取结果并返回
            extractVideoIds(ret.value(), out, totalCount);
        }
        return out;
    }
    std::vector<std::string> videoSearch::searchVideoOrderByLikeCount(const std::string& keyword, const int from, const int size, size_t& totalCount)
    {
        //默认权重
        suies::suiSearch search(indexName);
        {
            //设置范围
            search.setFrom(from);
            search.setSize(size);
        }
        {
            //设置筛选流程
            auto query = search.query();
            auto boolkey = query->QBool();
            auto must = boolkey->must();
            {
                auto multi= must->multi_match();
                multi->setQuery(keyword);
                multi->appendField(videoTitleField + "^3"); //标题优先
                multi->appendField(videoDescField + "^2"); //描述次之
                multi->setOperator("or");
                multi->setFuzziness("AUTO");
                multi->setType("best_fields");
            }
            suies::suiFilter::Ptr filter = boolkey->filter();
            {
                //添加
                auto term = filter->getFilterTerm(videoStatusField);
                term->setValue(4);
            }
        }
        {
            //默认接口，只对时间进行降序排列
            auto sort = search.sort();
            sort->addOrder(likeCountField, suies::sortType::descending);
            sort->addOrder("_score", suies::sortType::descending);
            sort->addOrder(releaseTimeField, suies::sortType::descending);
        }
        {
            //设置只返回
            search.setSource();
        }

        auto ret = _clientPtr->search(search);
        std::vector<std::string> out;
        if(ret.has_value())
        {
            //开始提取结果并返回
            extractVideoIds(ret.value(), out, totalCount);
        }
        return out;
    }
    std::vector<std::string> videoSearch::searchVideoOrderByReleaseTime(const std::string& keyword, const int from, const int size, size_t& totalCount)
    {
        //默认权重
        suies::suiSearch search(indexName);
        {
            //设置范围
            search.setFrom(from);
            search.setSize(size);
        }
        {
            //设置筛选流程
            auto query = search.query();
            auto boolkey = query->QBool();
            auto must = boolkey->must();
            {
                auto multi= must->multi_match();
                multi->setQuery(keyword);
                multi->appendField(videoTitleField + "^3"); //标题优先
                multi->appendField(videoDescField + "^2"); //描述次之
                multi->setOperator("or");
                multi->setFuzziness("AUTO");
                multi->setType("best_fields");
            }
            suies::suiFilter::Ptr filter = boolkey->filter();
            {
                //添加
                auto term = filter->getFilterTerm(videoStatusField);
                term->setValue(4);
            }
        }
        {
            //默认接口，只对时间进行降序排列
            auto sort = search.sort();
            sort->addOrder(releaseTimeField, suies::sortType::descending);
            sort->addOrder("_score", suies::sortType::descending);
        }
        {
            //设置只返回
            search.setSource();
        }

        auto ret = _clientPtr->search(search);
        std::vector<std::string> out;
        if(ret.has_value())
        {
            //开始提取结果并返回
            extractVideoIds(ret.value(), out, totalCount);
        }
        return out;
    }

    void videoSearch::extractVideoIds(const std::string& jsonStr, std::vector<std::string>& videoIds, size_t& totalCount)
    {
        Json::Value root;
        std::string errs;
        Json::CharReaderBuilder readerBuilder;
        std::unique_ptr<Json::CharReader> const reader(readerBuilder.newCharReader());

        // 1. 解析 JSON 字符串
        if (!reader->parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), &root, &errs)) {
            return;
        }

        // 2. 校验第一层 "hits" 对象是否存在
        if (!root.isMember("hits") || !root["hits"].isObject()) {
            return;
        }

        const Json::Value& hitsObj = root["hits"];

        // 提取总命中数
        if (hitsObj.isMember("total")) {
            const Json::Value& totalVal = hitsObj["total"];
            // 兼容 ES 7.x 及以上版本 ("total": { "value": 128, "relation": "eq" })
            if (totalVal.isObject() && totalVal.isMember("value") && totalVal["value"].isNumeric()) {
                totalCount = totalVal["value"].asUInt64();
            } 
            // 兼容 ES 6.x 及以下版本 ("total": 128)
            else if (totalVal.isNumeric()) {
                totalCount = totalVal.asUInt64();
            }
        }

        // 3. 校验第二层 "hits" 数组是否存在
        if (!hitsObj.isMember("hits") || !hitsObj["hits"].isArray()) {
            return;
        }

        const Json::Value& hitsArray = hitsObj["hits"];

        // 4. 预分配内存，避免 vector 频繁扩容
        videoIds.reserve(hitsArray.size());

        // 5. 遍历提取每个命中项的 _id
        for (Json::ArrayIndex i = 0; i < hitsArray.size(); ++i) {
            const Json::Value& item = hitsArray[i];
            if (item.isMember("_id") && item["_id"].isString()) {
                videoIds.push_back(item["_id"].asString());
            }
        }

        return;
    }
}