#pragma once
#include <memory>
#include <brpc/server.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/suiFastdfs.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiodb.hpp>
#include "getwayServerData.hpp"
#include "getwayServerRpc.hpp"

namespace suiGetwayServer
{
    class getwayServerBuild;
    class getwayServer
    {
    private:
        getwayServer(std::shared_ptr<brpc::Server> server);
    
    public:
        using ptr = std::shared_ptr<getwayServer>;

        void start();

        friend class getwayServerBuild;
        friend class std::allocator<getwayServerBuild>;
    private:
        std::shared_ptr<brpc::Server> _server;
    };

    class getwayServerBuild
    {
    public:
        //参数设置
        void setUserServerName(const std::string& user_server_name);
        void setVideoServerName(const std::string& video_server_name);
        void setFileServerName(const std::string& file_server_name);
        void setOdb(suiOdb::odbSetting settings);
        void setRedis(suiRedis::redisSettings settings);
        void setListenPort(int port);
        void setRegistryCenter(const std::string& registry_center);

        getwayServer::ptr build();
    private:
        //设置服务名
        std::string _user_server_name;
        std::string _video_server_name;
        std::string _file_server_name;
        //注册中心地址
        std::string _registry_center;
        //监听端口
        int _listen_port;
        //数据库缓存设置
        suiOdb::odbSetting _odbSetting;
        suiRedis::redisSettings _redisSetting;
        
    };
}