#include "getwayServer.hpp"

namespace suiGetwayServer
{
    getwayServer::getwayServer(std::shared_ptr<brpc::Server> server)
        : _server(server)
    {

    }

    void getwayServer::start()
    {
        //启动server
        _server->RunUntilAskedToQuit();
    }

    void getwayServerBuild::setUserServerName(const std::string& user_server_name)
    {
        _user_server_name = user_server_name;
    }

    void getwayServerBuild::setVideoServerName(const std::string& video_server_name)
    {
        _video_server_name = video_server_name;
    }

    void getwayServerBuild::setFileServerName(const std::string& file_server_name)
    {
        _file_server_name = file_server_name;
    }
    void getwayServerBuild::setOdb(suiOdb::odbSetting settings)
    {
        _odbSetting = settings;
    }

    void getwayServerBuild::setRedis(suiRedis::redisSettings settings)
    {
        _redisSetting = settings;
    }

    void getwayServerBuild::setListenPort(int port)
    {
        _listen_port = port;
    }

    void getwayServerBuild::setRegistryCenter(const std::string& registry_center)
    {
        _registry_center = registry_center;
    }

    getwayServer::ptr getwayServerBuild::build()
    {
        //创建数据处理接口
        getwayServerData::ptr _dataHandle = std::make_shared<getwayServerData>(_odbSetting, _redisSetting);
        //创建信道管理句柄
        suiRpc::svcChannels::ptr _channels = std::make_shared<suiRpc::svcChannels>();
        _channels->setWatch(_user_server_name);
        _channels->setWatch(_video_server_name);
        _channels->setWatch(_file_server_name);
        //创建服务搜索句柄
        suiEtcd::serSearch::ptr _search = std::make_shared<suiEtcd::serSearch>("", _registry_center
            , [_channels](std::string serName, std::string addr){
                _channels->addNode(serName, addr);
                INFO("服务 {} 上线, 地址: {}", serName, addr);
        }, [_channels](std::string serName, std::string addr){
            _channels->delNode(serName, addr);
            INFO("服务 {} 离线, 地址: {}", serName, addr);
        });
        //初始化服务搜索句柄
        _search->search();
        //创建RPC服务
        getwayServerRpc* _rpc = new getwayServerRpc(_user_server_name, _video_server_name, _file_server_name, _search, _channels,  _dataHandle);
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

        getwayServer::ptr retGetwayServer(new getwayServer(_server));
        INFO("网关服务创建成功");
        return retGetwayServer;
    }

}