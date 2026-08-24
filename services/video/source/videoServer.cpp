#include "videoServer.hpp"

namespace suiVideoServer
{
    videoServer::videoServer(suiEtcd::serProvider::ptr provider, std::shared_ptr<brpc::Server> server)
        : _provider(provider), _server(server)
    {

    }

    void videoServer::start()
    {
        _server->RunUntilAskedToQuit();
    }

    void videoServerBuild::setRegistrySettings(registrySettings settings)
    {
        _registrySettings = settings;
    }

    void videoServerBuild::setMqSettings(std::string mqUrl)
    {
        _mqurl = mqUrl;
    }
    void videoServerBuild::setOdb(suiOdb::odbSetting settings)
    {
        _odbSetting = settings;
    }

    void videoServerBuild::setRedis(suiRedis::redisSettings settings)
    {
        _redisSetting = settings;
    }

    void videoServerBuild::setEsUrl(const std::string& esUrl)
    {
        _esUrl = esUrl;
    }

    void videoServerBuild::setFileRemoveSetting(suiQueue::queueSetting settings)
    {
        _fileRemoveSetting = settings;
    }

    void videoServerBuild::setTranscodeSetting(suiQueue::queueSetting settings)
    {
        _transcodeSetting = settings;
    }

    void videoServerBuild::setListenPort(int port)
    {
        _listen_port = port;
    }

    videoServer::ptr videoServerBuild::build()
    {
        //创建etcd客户端
        auto _provider = std::make_shared<suiEtcd::serProvider>(_registrySettings.service_name, _registrySettings.registry_center);
        _provider->setSerAddr(_registrySettings.service_addr);
        //创建mq客户端
        suiQueue::MQClient::ptr _mqClienrt = std::make_shared<suiQueue::MQClient>(_mqurl);
        //数据库句柄
        std::shared_ptr<odb::database> mysql = suiOdb::dbFactory::create(_odbSetting);
        std::shared_ptr<sw::redis::Redis> redis = suiRedis::RedisFactory::create(_redisSetting);
        //创建异步视频弹幕维护类
        suiVideoSubtitleTarget::videoSubtitleAsync::ptr videoSubtitleAsync = std::make_shared<suiVideoSubtitleTarget::videoSubtitleAsync>(mysql, redis, _mqClienrt);
        //es客户端
        auto esHandlePtr = std::shared_ptr<suies::esClient>(new suies::esClient({_esUrl}));
        //视频搜索句柄
        suiVideoSearch::videoSearch::ptr videoSearch = std::make_shared<suiVideoSearch::videoSearch>(esHandlePtr);
        //普通缓存异步维护类
        std::shared_ptr<suiRemoveCache::RemoveCache> removeCache = std::make_shared<suiRemoveCache::RemoveCache>(redis, _mqClienrt);
        //文件异步删除类
        suiCacheSync::CacheSyncClient::ptr _fileRemove = std::make_shared<suiCacheSync::CacheSyncClient>(_mqClienrt, _fileRemoveSetting, nullptr);
        //视频异步统计同步类
        suiVideoDataStatistics::VideoDataStatisticsAsync::ptr videoDataStatisticsAsync = std::make_shared<suiVideoDataStatistics::VideoDataStatisticsAsync>(mysql, redis, _mqClienrt, removeCache);
        //转码消息发布句柄
        suiCacheSync::CacheSyncClient::ptr transcode_operation_handle = std::make_shared<suiCacheSync::CacheSyncClient>(_mqClienrt, _transcodeSetting, nullptr);

        //创建综合操作句柄
        videoServerData::ptr operationHandle = std::make_shared<videoServerData>(mysql, redis, removeCache, videoSubtitleAsync
                                                , videoSearch, _fileRemove, transcode_operation_handle, videoDataStatisticsAsync);
        
        //创建rpc服务器
        videoServerRpc* _rpc = new videoServerRpc(operationHandle);

        //构造server对象
        std::shared_ptr<brpc::Server> _server = std::make_shared<brpc::Server>();
        auto serRet = _server->AddService(_rpc, brpc::ServiceOwnership::SERVER_OWNS_SERVICE);
        if(serRet != 0)
        {
            ERROR("brpc服务添加失败, 错误码是：{}", serRet);
            return nullptr;
        }
        brpc::ServerOptions options;
        options.idle_timeout_sec = -1;
        auto startRet = _server->Start(_listen_port, &options);
        if(startRet != 0)
        {
            ERROR("brpc服务启动失败, 错误码是：{}", startRet);
            return nullptr;
        }
        //启动etcd
        _provider->redister();
        //构造服务对象
        videoServer::ptr retVideoServer(new videoServer(_provider, _server));
        INFO("服务key为： {}", _provider->getKey());
        return retVideoServer;
    }

}