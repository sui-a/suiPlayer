#pragma once
#include <suiScaffold/log.h>
#include <suiScaffold/suipeg.hpp>
#include "transcodeServerData.hpp"
#include "transcodeServerMq.hpp"

namespace suiTranscodeServer
{
    class transcodeServer
    {
    public:
    using ptr = std::shared_ptr<transcodeServer>;
        transcodeServer(transcodeServerMq::ptr mq);

        void start();

    private:
        transcodeServerMq::ptr _transcodeServerMq;
    };

    class serverBuilder
    {
    public:
        serverBuilder() = default;

        //设置
        void setOdb(suiOdb::odbSetting& settings);
        void setMqUrl(const std::string& url);
        void setListenMq(suiQueue::queueSetting& settings);
        void setTempFilePath(const std::string& tempFilePath);
        void setHlsSettings(suiPeg::hlsSettings& settings);
        void setRequestPrefix(const std::string& requestPrefix);
        void setFileRemoveMq(suiQueue::queueSetting& settings);

        transcodeServer::ptr build();
    private:
        //数据库缓存设置
        suiOdb::odbSetting _odbSetting;
        //mq路径设置
        std::string _mqurl;
        //监听mq设置
        suiQueue::queueSetting _listenQueueSetting;
        //临时文件路径设置
        std::string _tempFilePath;
        //转码设置
        suiPeg::hlsSettings _hlsSettings;
        //请求前缀
        std::string _requestPrefix;
        //文件删除队列mq设置
        suiQueue::queueSetting _fileRemoveQueueSetting;
    };
}