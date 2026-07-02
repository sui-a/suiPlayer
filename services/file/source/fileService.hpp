#pragma once
#include <brpc/server.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/suiFastdfs.hpp>
#include "fileServicemq.hpp"
#include "fileRpcService.hpp"
#include "suiIp.hpp"

namespace suiFileService
{
    class suifileServerBuiler;
    class suifileServer
    {
    private:
        suifileServer(suiEtcd::serProvider::ptr provider, std::shared_ptr<brpc::Server> server, suiFileService::fileRemoveMq::ptr fileRemoveMq);

    public:
        using ptr = std::shared_ptr<suifileServer>;
        
        void start();
        friend class suifileServerBuiler;
        friend class std::allocator<suifileServer>;
    private:
        //服务注册对象
        suiEtcd::serProvider::ptr _provider;
        //brpc对象(rpc调用)
        std::shared_ptr<brpc::Server> _server;
        //mq对象（异步删除）
        suiFileService::fileRemoveMq::ptr _fileRemoveMq;

    };

    struct registrySettings
    {
        std::string registry_center; //注册中心地址
        std::string service_name; //服务名称
        std::string service_addr; //服务地址
    };

    class suifileServerBuiler
    {
    public:
        suifileServerBuiler() = default;

        void setRegistrySettings(registrySettings settings);
        void setRegistrySettings(std::string registry_center, std::string service_name, std::string service_addr);

        void setOdbSetting(suiOdb::odbSetting settings);
        void setRedisSetting(suiRedis::redisSettings settings);
        void setQueueUrl(std::string url);
        void setRemoveQueueSetting(suiQueue::queueSetting settings);
        void setFastdfsSetting(suifd::FastdfsSetting settings);
        void setListenPort(int port);

        suifileServer::ptr build();
    private:
        //etcd设置
        registrySettings _registrySettings;
        //数据库设置
        suiOdb::odbSetting _odbSetting;
        //redis设置
        suiRedis::redisSettings _redisSetting;
        //mq设置
        std::string _mqurl;
        //文件删除队列的mq设置
        suiQueue::queueSetting _removeQueueSetting;
        //fdfs
        suifd::FastdfsSetting _fastdfsSetting;
        //监听端口
        int _listen_port = 0;
    };


}