#pragma once
#include <memory>
#include <brpc/server.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/suiFastdfs.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiElastic.hpp>
#include "video_subtitle_target.hpp"
#include "video_data_statistics.hpp"
#include "videoServerData.hpp"
#include "videoServerRpc.hpp"

namespace suiVideoServer
{
    class videoServerBuild;
    class videoServer
    {
    private:
        videoServer(suiEtcd::serProvider::ptr provider, std::shared_ptr<brpc::Server> server);
    
    public:
        using ptr = std::shared_ptr<videoServer>;

        void start();

        friend class videoServerBuild;
        friend class std::allocator<videoServerBuild>;
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

    class videoServerBuild
    {
    public:
        videoServerBuild() = default;

        //设置
        void setRegistrySettings(registrySettings settings);
        void setMqSettings(std::string mqUrl);
        void setOdb(suiOdb::odbSetting settings);
        void setRedis(suiRedis::redisSettings settings);
        void setEsUrl(const std::string& esUrl);
        void setFileRemoveSetting(suiQueue::queueSetting settings);
        void setTranscodeSetting(suiQueue::queueSetting settings);
        void setListenPort(int port);

        videoServer::ptr build();
    private:
        //etcd设置
        registrySettings _registrySettings;
        //mq路径设置
        std::string _mqurl;
        //数据库缓存设置
        suiOdb::odbSetting _odbSetting;
        suiRedis::redisSettings _redisSetting;
        std::string _esUrl;
        //文件异步删除设置
        suiQueue::queueSetting _fileRemoveSetting;
        //转码异步处理类设置
        suiQueue::queueSetting _transcodeSetting;
        //监听端口
        int _listen_port;
    };


}