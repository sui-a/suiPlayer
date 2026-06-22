#include "fileService.hpp"
#include <iostream>

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
        //创建etcd保活类
        auto _provider = std::make_shared<suiEtcd::serProvider>(_registrySettings.service_name, _registrySettings.registry_center);
        _provider->setSerAddr(_registrySettings.service_addr);
        INFO("etcd保活类创建成功");
        
        //初始化fdfs
        suifd::fdfsCreate(_fastdfsSetting);
        INFO("fdfs初始化成功");

        //创建mq客户端
        suiQueue::MQClient::ptr _mqClienrt = std::make_shared<suiQueue::MQClient>(_mqurl);
        INFO("mq客户端创建成功");

        //创建文件元信息管理器
        fileMetaService::ptr _filemetaptr = std::make_shared<fileMetaService>(_odbSetting, _redisSetting, _mqClienrt);
        INFO("文件元信息管理器创建成功");
        
        //打印_removeQueueSetting验证问题
        INFO("removeQueueSetting: 交换机名称： {}", _removeQueueSetting.exchange);
        INFO("removeQueueSetting: 交换机类型： {}", _removeQueueSetting.exchangeType);
        INFO("removeQueueSetting: 队列名称： {}", _removeQueueSetting.queue);
        INFO("removeQueueSetting: 绑定键： {}", _removeQueueSetting.bindKey);

        //设置mq队列
        auto _filemq = std::make_shared<fileRemoveMq>(_filemetaptr, _mqClienrt, _removeQueueSetting);
        INFO("mq队列创建成功");
        std::cin.get();

        //创建brpc服务器
        suiFileRpcService* _rpc = new suiFileRpcService(_filemetaptr);
        INFO("brpc服务创建成功");
    
        //构造server对象
        std::shared_ptr<brpc::Server> _server = std::make_shared<brpc::Server>();
        auto serRet = _server->AddService(_rpc, brpc::ServiceOwnership::SERVER_OWNS_SERVICE);
        if(serRet != 0)
        {
            ERROR("brpc服务添加失败, 错误码是：{}", serRet);
            return nullptr;
        }
        INFO("brpc服务添加成功");

        //启动服务
        brpc::ServerOptions options;
        options.idle_timeout_sec = -1;
        auto startRet = _server->Start(_listen_port, &options);
        if(startRet != 0)
        {
            ERROR("brpc服务启动失败, 错误码是：{}", startRet);
            return nullptr;
        }
        INFO("brpc服务启动成功");


        //启动etcd
        _provider->redister();
        INFO("etcd服务注册成功");

        //构造返回对象
        suifileServer::ptr retFileServer(new suifileServer(_provider, _server, _filemq));
        INFO("文件服务创建成功");
        return retFileServer;
    }



}