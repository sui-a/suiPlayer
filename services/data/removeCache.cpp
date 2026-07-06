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
        _publisherQueue->publish(msg.SerializeAsString());
        return true;
    }
    bool RemoveCache::callback(std::string body)
    {
        suiApi::DeleteCacheMsg msg;
        bool ret = msg.ParseFromString(body);
        if(!ret) {
            ERROR("收到缓存同步消息，但反序列化失败");
            return true;  //反序列化失败，但返回true 目的是不中断消息队列的消费 无效消息丢弃即可
        }
        int sz = msg.key_size();
        for(int i = 0; i < sz; i++) {
            INFO("删除缓存key: {}", msg.key(i));
            _redis->del(msg.key(i));
        }
        return true;
    }

    RemoveCache::~RemoveCache()
    {

    }
    
}