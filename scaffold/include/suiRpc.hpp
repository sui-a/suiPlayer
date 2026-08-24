#pragma once
#include <brpc/channel.h>
#include <brpc/server.h>
#include <functional>
#include <mutex>
#include <list>
#include "log.h"

namespace suiRpc
{
    using ChannelPtr = std::shared_ptr<brpc::Channel>;
    class Channels
    {
    private:
        //单服务节点管理类
    public:
        Channels(std::string& name);
        ~Channels();

        //新增节点
        void insert(const std::string& addr);
        //删除节点
        void remove(const std::string& addr);

        //获取节点
        ChannelPtr select();

        using Ptr = std::shared_ptr<Channels>;
    private:
        std::mutex _mtx;
        std::string _srvName; //服务名字
        std::list<ChannelPtr>::iterator _index;  //轮询的迭代器
        std::list<ChannelPtr> _channels;
        std::unordered_map<std::string, std::list<ChannelPtr>::iterator> _map;
    };

    class svcChannels
    {
    private:
        //多节点管理类
    public:
        using ptr = std::shared_ptr<svcChannels>;

        svcChannels();
        ~svcChannels();
        
        void setWatch(std::string& name);
        void addNode(std::string& name, std::string& addr);
        void delNode(std::string& name, std::string& addr);
        ChannelPtr getNode(std::string& name);
    private:
        std::mutex _mtx;
        std::unordered_map<std::string, Channels::Ptr> _map;
    };


    class ClosureFactory
    {
    public:
        using callBackFun = std::function<void()>;
        static google::protobuf::Closure* create(callBackFun&& fun); 
    private:
        struct Object{
            using ptr = std::shared_ptr<Object>;
            callBackFun callback;
        };
        static void asyncCallback(const Object::ptr obj);
    };

    class RpcServerFactory
    {
    public:
        static std::shared_ptr<brpc::Server> create(int port, google::protobuf::Service* svc);
    private:
    };
}