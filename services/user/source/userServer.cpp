#include "userServer.hpp"

namespace suiUser
{

    suiUserServer::suiUserServer(suiEtcd::serProvider::ptr provider, std::shared_ptr<brpc::Server> server)
        : _provider(provider), _server(server)
    {

    }

    void suiUserServer::start()
    {
        _server->RunUntilAskedToQuit();
    }
    
    void userServerBuild::setRegistrySettings(registrySettings settings)
    {
        _registrySettings = settings;
    }

    void userServerBuild::setMqSettings(std::string mqUrl)
    {
        _mqurl = mqUrl;
    }

    void userServerBuild::setImapSetting(suiMail::MailSetting settings)
    {
        _imapSetting = settings;
    }

    void userServerBuild::setListenPort(int port)
    {
        _listen_port = port;
    }

    void userServerBuild::setOdbSetting(suiOdb::odbSetting settings)
    {
        _odbSetting = settings;
    }

    void userServerBuild::setRedisSetting(suiRedis::redisSettings settings)
    {
        _redisSetting = settings;
    }

    void userServerBuild::setFileRemoveSetting(suiQueue::queueSetting settings)
    {
        _fileRemoveSetting = settings;
    }

    suiUserServer::ptr userServerBuild::build()
    {
        //创建etcd
        auto _provider = std::make_shared<suiEtcd::serProvider>(_registrySettings.service_name, _registrySettings.registry_center);
        _provider->setSerAddr(_registrySettings.service_addr);

        //创建mq客户端
        suiQueue::MQClient::ptr _mqClienrt = std::make_shared<suiQueue::MQClient>(_mqurl);
        //创建邮箱操作句柄
        suiMail::suiMailClient::ptr mailClient = std::make_shared<suiMail::suiMailClient>(_imapSetting);
        //创建文件删除句柄
        suiCacheSync::CacheSyncClient::ptr cacheSyncClient = std::make_shared<suiCacheSync::CacheSyncClient>(_mqClienrt, _fileRemoveSetting, nullptr);

        //创建操作句柄
        suiUserServerData::ptr userMetaOperation = std::make_shared<suiUserServerData>(_odbSetting, _redisSetting, _mqClienrt, mailClient, cacheSyncClient);
        //创建brpc服务器
        suiUserServerRpc* _rpc = new suiUserServerRpc(userMetaOperation);

        //构造server对象
        std::shared_ptr<brpc::Server> _server = std::make_shared<brpc::Server>();
        auto serRet = _server->AddService(_rpc, brpc::ServiceOwnership::SERVER_OWNS_SERVICE);
        if(serRet != 0)
        {
            ERROR("brpc服务添加失败, 错误码是：{}", serRet);
            return nullptr;
        }

        //启动
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
        suiUserServer::ptr retUserServer(new suiUserServer(_provider, _server));
        INFO("服务key为： {}", _provider->getKey());
        return retUserServer;
    }
}