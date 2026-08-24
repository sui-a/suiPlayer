#pragma once
#include <memory>
#include <filesystem>
#include <string>
#include <chrono>
#include <thread>
#include <suiScaffold/suipeg.hpp>
#include <suiScaffold/suiFastdfs.hpp>
#include <suiScaffold/suiRandom.hpp>
#include <suiScaffold/suiQueue.hpp>
#include "transcodeServerData.hpp"
#include "cache_sync.hpp"

namespace suiTranscodeServer
{
    class transcodeServerMq
    {
    public:
        using ptr = std::shared_ptr<transcodeServerMq>;
        transcodeServerMq(suiQueue::MQClient::ptr clientPtr, const std::string& tempFilePath, transcodeServerData::ptr dataHandle
            , suiQueue::suiSubscriber::ptr subscribeQueue, std::shared_ptr<suiPeg::HLSTranscoder> transcode
            , const std::string& requestPrefix, suiCacheSync::CacheSyncClient::ptr fileRemoveSyncClient);
            
        void start();
    
    private:
        bool callback(const std::string& videoid);
        void deletePath(const std::string& pathStr);
        bool ensureDir(const std::string& filepath);

    private:
        //mq客户端
        suiQueue::MQClient::ptr _clientPtr;
        //临时文件存放路径
        std::string _temp_file_path;
        //操作句柄
        transcodeServerData::ptr _dataHandle;
        //监听对象
        suiQueue::suiSubscriber::ptr _subscribeQueue;
        //转码对象
        std::shared_ptr<suiPeg::HLSTranscoder> _transcoder;
        //请求前缀
        std::string _requestPrefix;
        //文件删除句柄
        suiCacheSync::CacheSyncClient::ptr _fileRemoveSyncClient;
    };



}