#include "removeCache.hpp"

namespace suiRemoveCache
{
    RemoveCache::RemoveCache(std::shared_ptr<sw::redis::Redis> redis, suiQueue::MQClient::ptr MqClientPtr)
        :_redis(redis)
    {
        //创建消息队列发布者
        struct suiQueue::queueSetting set;
        set.exchange = "delete_cache_exchange",
        set.exchangeType = "delayed",
        set.queue = "delete_cache_queue",
        set.bindKey = "delete_cache",
        set.ttl = 3000;
        _publisherQueue = std::make_shared<suiQueue::suiPublisher>(MqClientPtr, set);
        
        _subscribeQueue = std::make_shared<suiQueue::suiSubscriber>(MqClientPtr, set);
        //设置订阅方法
        //_subscribeQueue->consume(callback);

        std::cout << "1 this 指针地址是 ： " << this << std::endl;
        // _subscribeQueue->consume([=](std::string msg){
        //     std::cout << "2 this 指针地址是 ： " << this << std::endl;
        //     return callback(msg);
        // });

         _subscribeQueue->consume(std::bind(&RemoveCache::callback, this, std::placeholders::_1));
        
    }

    bool RemoveCache::syncCache(const std::vector<std::string>& cache_key)
    {
        if (cache_key.empty()) {
            return false;
        }
        // 1. 构造消息
        suiApi::DeleteCacheMsg msg;
        for (auto &id : cache_key) {
            _redis->del(id);
            msg.add_key(id);
        }
        // 2. 发布消息  
        INFO("删除缓存key: {}", cache_key.size());
        INFO("发布缓存同步消息: {}", msg.SerializeAsString());
        _publisherQueue->publish(msg.SerializeAsString());
        INFO("发布缓存同步消息完成");
        return true;
    }
    bool RemoveCache::callback(std::string body)
    {
        INFO("进入消息回调处理");
        suiApi::DeleteCacheMsg msg;
        bool ret = msg.ParseFromString(body);
        if(!ret) {
            ERROR("收到缓存同步消息，但反序列化失败");
            return true;  //反序列化失败，但返回true 目的是不中断消息队列的消费 无效消息丢弃即可
        }
        int sz = msg.key_size();
        INFO("收到删除缓存key消息，共{}个key", sz);
        for(int i = 0; i < sz; i++) {
            INFO("删除缓存key: {}", msg.key(i));
            _redis->del(msg.key(i));
        }
        INFO("删除缓存key完成");
        return true;
    }

    // bool RemoveCache::setCallback()
    // {
    //     _subscribeQueue->consume([this](std::string msg){
    //         return callback(msg);
    //     });
    //     return true;

        
    // }


    RemoveCache::~RemoveCache(){
        std::cout << "RemoveCache 析构函数调用" << std::endl;
    }
    
}