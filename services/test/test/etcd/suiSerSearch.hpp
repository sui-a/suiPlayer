#pragma once 
#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>
#include <etcd/Watcher.hpp>
#include <etcd/Response.hpp>
#include <etcd/Value.hpp>
#include <string>
#include <random>
#include <iomanip>
#include <atomic>


namespace sui
{
    class serProvider
    {
        //服务注册
    public:
        serProvider(const std::string &serName, const std::string addr);
        ~serProvider();

        //进行服务注册
        void redister(const std::string &serAddr);


    private:
        std::string _addr; //注册中心地址
        std::string _id;  //服务id
        std::string _serName; //服务名称
        std::string _serAddr;
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

    const size_t uuid_size = 16;
    class RandomUtil
    {
    public:
        enum class UuidType
        {
            DIGIT, //数字
            ALPHA, //字母
            ALL    //数字和字母
        };

        //产生固定长度的随机字符串
        static std::string uuid(UuidType type = UuidType::DIGIT, size_t len = uuid_size);
    };
}
