#include "transcodeServer.hpp"

namespace suiTranscodeServer
{
    transcodeServer::transcodeServer(transcodeServerMq::ptr mq)
        : _transcodeServerMq(mq)
    {

    }

    void transcodeServer::start()
    {
        _transcodeServerMq->start();
    }

    void serverBuilder::setOdb(suiOdb::odbSetting& settings)
    {
        _odbSetting = settings;
    }

    void serverBuilder::setMqUrl(const std::string& url)
    {
        _mqurl = url;
    }

    void serverBuilder::setListenMq(suiQueue::queueSetting& settings)
    {
        _listenQueueSetting = settings;
    }

    void serverBuilder::setTempFilePath(const std::string& tempFilePath)
    {
        _tempFilePath = tempFilePath;
    }

    void serverBuilder::setHlsSettings(suiPeg::hlsSettings& settings)
    {
        _hlsSettings = settings;
    }

    void serverBuilder::setRequestPrefix(const std::string& requestPrefix)
    {
        _requestPrefix = requestPrefix;
    }

    void serverBuilder::setFileRemoveMq(suiQueue::queueSetting& settings)
    {
        _fileRemoveQueueSetting = settings;
    }

    transcodeServer::ptr serverBuilder::build()
    {
        //数据库句柄
        std::shared_ptr<odb::database> mysql = suiOdb::dbFactory::create(_odbSetting);
        //创建mq客户端
        suiQueue::MQClient::ptr _mqClienrt = std::make_shared<suiQueue::MQClient>(_mqurl);
        //创建监听句柄
        suiQueue::suiSubscriber::ptr _subscribeQueue = std::make_shared<suiQueue::suiSubscriber>(_mqClienrt, _listenQueueSetting);
        //数据操作对象
        transcodeServerData::ptr _dataHandle = std::make_shared<transcodeServerData>(mysql);
        //初始化转码对象
        std::shared_ptr<suiPeg::HLSTranscoder> _transcoder = std::make_shared<suiPeg::HLSTranscoder>(_hlsSettings);
        //创建文件删除句柄
        suiCacheSync::CacheSyncClient::ptr cacheSyncClient = std::make_shared<suiCacheSync::CacheSyncClient>(_mqClienrt, _fileRemoveQueueSetting, nullptr);
        //创建服务对象
        transcodeServerMq::ptr mq = std::make_shared<transcodeServerMq>(_mqClienrt, _tempFilePath, _dataHandle, _subscribeQueue, _transcoder, _requestPrefix, cacheSyncClient);
        transcodeServer::ptr server = std::make_shared<transcodeServer>(mq);
        return server;
    }
}