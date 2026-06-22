#pragma once 
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Watcher.hpp>
#include <etcd/Response.hpp>
#include <etcd/Value.hpp>
#include <string>
#include "suiRandom.hpp"
#include "log.h"


namespace suiEtcd
{
    class serProvider
    {
        //服务注册
    public:
        using ptr = std::shared_ptr<serProvider>;

        serProvider(const std::string &serName, const std::string addr);
        ~serProvider();

        //进行服务注册
        void setSerAddr(const std::string &serAddr);
        void redister();


    private:
        std::string _addr; //注册中心地址
        std::string _id;  //服务id
        std::string _serName; //服务名称
        std::string _serAddr; //注册的地址
        std::shared_ptr<etcd::KeepAlive> _keepAlive;
        //等待连接
        void waitConnect(etcd::Client& client);
        //构建key
        std::string makeKey();
    };

    class serSearch
    {
    //服务发现
    public:
        using ptr = std::shared_ptr<serSearch>;

        //回调函数
        using ModCallback = std::function<void(std::string, std::string)>;
        serSearch(const std::string &serName, const std::string& addr, const ModCallback& online, const ModCallback& offline);
        ~serSearch();

        //进行服务发现
        void search();

    private:
        std::string _addr; //注册中心地址
        ModCallback _online; //上线的回调
        ModCallback _offline; //下线的回调
        std::shared_ptr<etcd::Watcher> _watcher; //监控对象
        std::string _serName; //服务名称

        //等待连接
        void waitConnect(etcd::Client& client);
        //回调处理
        void callback(const etcd::Response& resp);
        //获取服务名称
        std::string getSerName(const std::string& key);
    };
}
