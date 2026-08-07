#pragma once
#include <memory>
#include <brpc/server.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/suiFastdfs.hpp>
#include "userServerData.hpp"
#include "userServerRpc.hpp"

namespace suiUser
{
    class suiUserServer
    {
    private:
        suiUserServer(suiEtcd::serProvider::ptr provider, std::shared_ptr<brpc::Server> server);
    
    public:
        using ptr = std::shared_ptr<suiUserServer>;

        void start();

        friend class userServerBuild;
        friend class std::allocator<suiUserServer>;
    private:
        suiEtcd::serProvider::ptr _provider;
        std::shared_ptr<brpc::Server> _server;
    };

    struct registrySettings
    {
        std::string registry_center; //注册中心地址
        std::string service_name; //服务名称
        std::string service_addr; //服务地址
    };

    class userServerBuild
    {
    public:
        userServerBuild() = default;

        //设置
        void setRegistrySettings(registrySettings settings);
        void setMqSettings(std::string mqUrl);
        void setImapSetting(suiMail::MailSetting settings);
        void setListenPort(int port);
        void setOdbSetting(suiOdb::odbSetting settings);
        void setRedisSetting(suiRedis::redisSettings settings);

        suiUserServer::ptr build();
    private:
        //etcd设置
        registrySettings _registrySettings;
        //mq设置
        std::string _mqurl;
        //数据库设置
        suiOdb::odbSetting _odbSetting;
        //redis设置
        suiRedis::redisSettings _redisSetting;
        //设置impq
        suiMail::MailSetting _imapSetting;
        //监听端口
        int _listen_port = 0;
    };
}
