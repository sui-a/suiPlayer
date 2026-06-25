#include "suiRpc.hpp"

namespace suiRpc
{
    Channels::Channels(std::string& name)
        :_srvName(name)
        ,_index(_channels.end())
    {}
    Channels::~Channels()
    {}

    void Channels::insert(const std::string& addr)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        if(_map.find(addr) != _map.end())
            return;

        //构造并进行初始化
        ChannelPtr curChannelPtr = std::make_shared<brpc::Channel>();
        brpc::ChannelOptions options;
        options.protocol = "baidu_std";
        curChannelPtr->Init(addr.c_str(), &options);

        //进行添加
        _channels.push_back(curChannelPtr);
        _map[addr] = --_channels.end();

        //判断是否要初始化索引
        if(_index == _channels.end())
            _index = _channels.begin();
    }
    
    void Channels::remove(const std::string& addr)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        auto it = _map.find(addr);
        if(it == _map.end())
            return;
        auto st = *it;
        //存在，开始移除
        if(_index == (*it).second)
        {
            //索引到同一个位置
            _index = _channels.erase((*it).second);
            if(_index == _channels.end())
                _index = _channels.begin();
        }
        else
            _channels.erase((*it).second);
        _map.erase(addr);
    }

    ChannelPtr Channels::select()
    {
        std::unique_lock<std::mutex> lock(_mtx);
        if(_index == _channels.end())
            return nullptr;
        auto it = _index;
        _index++;
        if(_index == _channels.end())
            _index = _channels.begin();
        return *it;
    }

    svcChannels::svcChannels()
    {

    }
    svcChannels::~svcChannels()
    {

    }
    
    void svcChannels::setWatch(std::string& name)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        if(_map.find(name) != _map.end())
            return;
        std::shared_ptr<Channels> channel = std::make_shared<Channels>(name);
        _map.insert(std::make_pair(name, channel));
    }

    void svcChannels::addNode(std::string& name, std::string& addr)
    {
        Channels::Ptr curPtr;
        {
            std::unique_lock<std::mutex> lock(_mtx);
            auto it = _map.find(name);
            if(it == _map.end())
                return;

            curPtr = it->second;
        }
        curPtr->insert(addr);
    }

    void svcChannels::delNode(std::string& name, std::string& addr)
    {
        Channels::Ptr curPtr;
        {
            std::unique_lock<std::mutex> lock(_mtx);
            auto it = _map.find(name);
            if(it == _map.end())
                return;

            curPtr = it->second;
        }
        curPtr->remove(addr);
    }

    ChannelPtr svcChannels::getNode(std::string& name)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        auto it = _map.find(name);
        if(it == _map.end())
            return nullptr;

        //找到
        return it->second->select();
    }

    google::protobuf::Closure* ClosureFactor::create(callBackFun&& fun)
    {
        ClosureFactor::Object::ptr obj = std::make_shared<ClosureFactor::Object>();
        obj->callback = std::move(fun);
        return brpc::NewCallback(&ClosureFactor::asyncCallback, obj);
    }


    void ClosureFactor::asyncCallback(const ClosureFactor::Object::ptr obj)
    {
        obj->callback();
    }


    std::shared_ptr<brpc::Server> RpcServerFactory::create(int port, google::protobuf::Service* svc)
    {
        //定义服务器配置对象
        brpc::ServerOptions options;
        options.idle_timeout_sec = -1; //设置永远不超时
        //实例化服务器对象
        auto server = std::make_shared<brpc::Server>();
        //进行服务添加
        int ret = server->AddService(svc, brpc::SERVER_OWNS_SERVICE);
        if(ret == -1)
        {
            ERROR("服务添加失败");
            return nullptr;

        }
        //启动服务器对象
        ret = server->Start(port, &options);
        if(ret == -1)
        {
            ERROR("服务开始失败");
            return nullptr;
        }
        return server;
    }


}