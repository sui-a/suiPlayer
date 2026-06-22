#include "fileService.hpp"


namespace suiFileService
{
    
    suifileServer::suifileServer(suiEtcd::serProvider::ptr provider, std::shared_ptr<brpc::Server> server, suiFileService::fileRemoveMq::ptr fileRemoveMq)
        : _provider(provider), _server(server), _fileRemoveMq(fileRemoveMq)
    {

    }

    void suifileServer::start()
    {
        //等待结束
        _server->RunUntilAskedToQuit();
    }

    void suifileServerBuiler::setRegistrySettings(registrySettings settings)
    {
        _registrySettings = settings;
    }
    void suifileServerBuiler::setRegistrySettings(std::string registry_center, std::string service_name, std::string service_addr)
    {
        _registrySettings.registry_center = registry_center;
        _registrySettings.service_name = service_name;
        _registrySettings.service_addr = service_addr;
    }

    void suifileServerBuiler::setOdbSetting(suiOdb::odbSetting settings)
    {
        _odbSetting = settings;
    }

    void suifileServerBuiler::setRedisSetting(suiRedis::redisSettings settings)
    {
        _redisSetting = settings;
    }

    void suifileServerBuiler::setQueueUrl(std::string url)
    {
        _mqurl = url;
    }

    void suifileServerBuiler::setRemoveQueueSetting(suiQueue::queueSetting settings)
    {
        _removeQueueSetting = settings;
    }

    void suifileServerBuiler::setFastdfsSetting(suifd::FastdfsSetting settings)
    {
        _fastdfsSetting = settings;
    }

    void suifileServerBuiler::setListenPort(int port)
    {
        _listen_port = port;
    }

    suifileServer::ptr suifileServerBuiler::build()
    {
        
        return nullptr;
    }



}