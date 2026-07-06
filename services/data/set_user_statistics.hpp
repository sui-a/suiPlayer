#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/log.h>
#include "data.hpp"
#include "data-odb.hxx"
#include "message.pb.h"

namespace suiUserStatics
{
    class suiSetUserStatistics
    {
    public:
        using ptr = std::shared_ptr<suiSetUserStatistics>;

        suiSetUserStatistics(std::shared_ptr<odb::database> curDb, suiQueue::MQClient::ptr MqClientPtr);

        void setVideoChangeSyncCache(const std::string video_id, int change); //数据库同步接口
        ~suiSetUserStatistics();

    private:
        bool VideoChangecallback(std::string body);

    private:
        std::shared_ptr<odb::database> _db;  //单纯客户端，未绑定链接对象
        suiQueue::MQClient::ptr _mqClient;

        suiQueue::suiPublisher::ptr _publisherQueue;
        suiQueue::suiSubscriber::ptr _subscribeQueue;
    };
}