#pragma once
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/log.h>
#include <suiScaffold/suiQueue.hpp>
#include <memory>
#include "message.pb.h"

namespace suiCacheSync
{
    //缓存同步
    using cacheCallbackPtr = std::function<bool(std::string body)>;
    class CacheSyncClient
    {
    public:
        using ptr = std::shared_ptr<CacheSyncClient>;

        CacheSyncClient(suiQueue::MQClient::ptr MqClientPtr, suiQueue::queueSetting set, cacheCallbackPtr callback);
        void syncCache(const std::string& cache_key);
    private:
        bool callback(std::string body); //消息订阅回调函数 取出key对缓存进行删除
    private:
        cacheCallbackPtr _callback;
        suiQueue::queueSetting _set;
        suiQueue::suiPublisher::ptr _publisherQueue;
        suiQueue::suiSubscriber::ptr _subscribeQueue;
    };



}