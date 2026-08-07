#include "cache_sync.hpp"

namespace suiCacheSync
{
    CacheSyncClient::CacheSyncClient(suiQueue::MQClient::ptr MqClientPtr, suiQueue::queueSetting set, cacheCallbackPtr callback)
        : _callback(callback)
        , _set(set)
    {
        _publisherQueue = std::make_shared<suiQueue::suiPublisher>(MqClientPtr, set);
        _subscribeQueue = std::make_shared<suiQueue::suiSubscriber>(MqClientPtr, set);
        _subscribeQueue->consume(std::bind(&CacheSyncClient::callback, this, std::placeholders::_1));
    }

    void CacheSyncClient::syncCache(const std::string& cache_key)
    {
        if (cache_key.empty()) {
            return;
        }
        // 1. 构造消息
        suiApi::DeleteCacheMsg msg;
        msg.add_key(cache_key);
        // 2. 发布消息
        _publisherQueue->publish(msg.SerializeAsString());
        return;
    }

    bool CacheSyncClient::callback(std::string body)
    {
        //对body进行反序列化
        suiApi::DeleteCacheMsg msg;
        bool ret = msg.ParseFromString(body);
        if(!ret) 
        {
            ERROR("同步接口反序列化失败");
            return true;
        }
        return _callback(msg.key(0));
    }

}