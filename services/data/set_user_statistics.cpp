#include "set_user_statistics.hpp"


namespace suiUserStatics
{

    suiSetUserStatistics::suiSetUserStatistics(std::shared_ptr<odb::database> curDb, suiQueue::MQClient::ptr MqClientPtr)
        : _db(curDb)
        , _mqClient(MqClientPtr)
    {
        //创建消息队列发布者
        struct suiQueue::queueSetting set;
        set.exchange = "set_db_exchange",
        set.exchangeType = "delayed",
        set.queue = "set_db_queue",
        set.bindKey = "set_db",
        set.ttl = 3000;
        _publisherQueue = std::make_shared<suiQueue::suiPublisher>(MqClientPtr, set);
        _subscribeQueue = std::make_shared<suiQueue::suiSubscriber>(MqClientPtr, set);
        _subscribeQueue->consume(std::bind(&suiSetUserStatistics::VideoChangecallback, this, std::placeholders::_1));
    }

    void suiSetUserStatistics::setVideoChangeSyncCache(const std::string userId, int change)
    {
        suiApi::setUserStatisticsMsg msg;
        msg.set_user_id(userId);
        msg.set_change(change);
        _publisherQueue->publish(msg.SerializeAsString());
    }
    suiSetUserStatistics::~suiSetUserStatistics()
    {

    }

    bool suiSetUserStatistics::VideoChangecallback(std::string body)
    {
        suiApi::setUserStatisticsMsg msg;
        bool ret = msg.ParseFromString(body);
        if(!ret) {
            ERROR("收到数据库用户播放量同步消息，但反序列化失败");
            return true; //直接丢弃消息，不影响后面消息处理
        }
        while(1)
        {
            try 
            {
                // 1. 开启数据库事务
                odb::transaction tx(_db->begin());
                auto& curdb = tx.database();
                std::string sql = "UPDATE tbl_video_meta SET video_play_count = video_play_count + " 
                                + std::to_string(msg.change()) + " WHERE user_id = '" + msg.user_id() + "'";
                curdb.execute(sql);
                tx.commit();
                break;
            } 
            catch (const std::exception& e) 
            {
                ERROR("更新数据库用户播放量失败，错误信息：%s", e.what());
                sleep(10); //休眠几秒，然后继续处理
                continue;
            }
        }
        return true;
    }

    
}