#include "suiSerSearch.hpp"

namespace suiEtcd
{
    serProvider::serProvider(const std::string &serName, const std::string addr)
        : _addr(addr)
        , _id(suiRandom::RandomUtil::uuid())
        , _serName(serName)
    {

    }

    serProvider::~serProvider()
    {}

    std::string serProvider::makeKey()
    {
        return "/" + _serName + "/" + _id;
    }

    void serProvider::setSerAddr(const std::string &serAddr)
    {
        _serAddr = serAddr;
    }

    void serProvider::redister()
    {
        //初始化客户端对象
        etcd::Client client(_addr);
        waitConnect(client);
        //已经连接成功
        //创建租约
        auto lease_resp = client.leasegrant(3).get();
        if(lease_resp.is_ok() == false)
        {
            //租约创建失败
            return;
        }
        //获取key
        auto key = makeKey();
        //创建租约成功
        //获取租约id
        auto lease_id = lease_resp.value().lease();
        // 2. 使用租约设置键值对
        auto put_resp = client.put(key, _serAddr, lease_id).get();
        if (!put_resp.is_ok()) 
        {
            //设置键值对失败
            return;
        }
        INFO("服务注册成功: {} -> {}", key, _serAddr);
        _keepAlive.reset(new etcd::KeepAlive(_addr, [this](std::exception_ptr ex) {
                std::thread threads([this](){
                    ERROR("KeepAlive发生异常，正在重新注册服务...");
                    this->redister(); 
                });
                threads.detach();
            }, 3, lease_id));
        //启动keepAlive线程
    }
    

    void serProvider::waitConnect(etcd::Client& client)
    {
        while(client.head().get().is_ok() == false)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void serSearch::waitConnect(etcd::Client& client)
    {
        while(client.head().get().is_ok() == false)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    serSearch::serSearch(const std::string &serName
                        , const std::string& addr
                        , const ModCallback& online
                        , const ModCallback& offline)
        : _serName(serName)
        , _addr(addr)
        , _online(online)
        , _offline(offline)
    { }
    
    serSearch::~serSearch()
    {
        
    }

    void serSearch::search()
    {
        //初始化客户端对象//实例化客户端对象
        etcd::Client client(_addr);
        waitConnect(client);

        //开始获取数据
        auto resp = client.ls("/").get();
        if(resp.is_ok() == false)
        {
            //数据获取失败
            return;
        }
        //开始初始化数据
        auto values = resp.values();
        for(auto it : values)
        {
            std::string serName = getSerName(it.key());
            std::string serAddr = it.as_string();
            if(_online)
                _online(serName, serAddr);
        }

        //开始创建监视器
        std::string key = "/" + _serName;
        
        ERROR("开始监控服务变化: {}", key);
        _watcher.reset(new etcd::Watcher(_addr, key, [this](const etcd::Response& resp) {
            this->callback(resp);
        }, true));

        _watcher->Wait(
            [this](bool cont){
                if(cont == true)
                {
                    //手动结束监控，正常退出
                    return ;
                }
                //监控异常，继续监控
                cont = true;
                this->search();
            }
        );
    }

    void serSearch::callback(const etcd::Response& resp)
    {
        if(resp.is_ok() == false)
        {
            //监控发生错误
            return;
        }

        auto events = resp.events();
        for(auto& it : events)
        {
            std::string curEventType;
            switch (it.event_type())
            {
                case etcd::Event::EventType::PUT:
                    curEventType = "新增";
                    break;
                case etcd::Event::EventType::DELETE_:
                    curEventType = "删除";
                    break;
                default:
                    curEventType = "异常";
                    break;
            };

            INFO("收到事件: {}, key: {} :  value: {}", curEventType, it.kv().key(), it.kv().as_string());
            if(it.event_type() == etcd::Event::EventType::PUT)
            {
                //数据改变
                std::string serName = getSerName(it.kv().key());
                std::string serAddr = it.kv().as_string();
                if(_online)
                    _online(serName, serAddr);
            }
            else if(it.event_type() == etcd::Event::EventType::DELETE_)
            {
                //数据删除
                std::string serName = getSerName(it.kv().key());
                std::string serAddr = it.prev_kv().as_string();
                if(_offline)
                    _offline(serName, serAddr);
            }
        }
        //
    }

    std::string serSearch::getSerName(const std::string& key)
    {
        // 空字符串或非绝对路径（不以'/'开头）返回空
        if (key.empty() || key[0] != '/') {
            return "";
        }

        size_t start = 1;                     // 跳过开头的 '/'
        size_t pos = key.find('/', start);    // 查找下一个 '/'

        if (pos == std::string::npos) {
            // 没有后续 '/'，如 "/dir1"
            return key.substr(start);
        } 
        return key.substr(start, pos - start);
    }
}