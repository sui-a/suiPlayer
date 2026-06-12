#pragma once
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/log.h>
#include <suiScaffold/suiQueue.hpp>
#include <memory>
#include "message.pb.h"
/*   双写策略    */

namespace suiRemoveCache
{
    class RemoveCache
    {
    public:
        using ptr = std::shared_ptr<RemoveCache>;

        RemoveCache(std::shared_ptr<sw::redis::Redis> redis, suiQueue::MQClient::ptr MqClientPtr);
        bool syncCache(const std::vector<std::string>& cache_key);//缓存同步接口

        //外部单独声明，把构造和订阅方法分离
        bool setCallback();

        ~RemoveCache();
    private:
        bool callback(std::string body); //消息订阅回调函数 取出key对缓存进行删除

    private:
        std::shared_ptr<sw::redis::Redis> _redis;
        suiQueue::suiPublisher::ptr _publisherQueue;
        suiQueue::suiSubscriber::ptr _subscribeQueue;
    };

    class RemoveCacheFactory
    {
    public:
        template<typename... Args>
        static RemoveCache::ptr create(Args&&... args)
        {
            return std::make_shared<RemoveCache>(std::forward<Args>(args)...);
        }
    };



}