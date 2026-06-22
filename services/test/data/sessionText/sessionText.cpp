#include <thread>
#include <chrono>
#include <suiScaffold/log.h>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiQueue.hpp>
#include "removeCache.hpp"
#include "session.hpp"


int main()
{
    suiUtil::suiLogInitDefault();
    {
        //创建redis操作句柄
        suiRedis::redisSettings redisSettings;
        redisSettings._host = "127.0.0.1";
        redisSettings._port = 8081;
        redisSettings._user = "default";
        redisSettings._password = "suisuipingan";
        redisSettings._db = 0;
        redisSettings._connection_pool_size = 10;

        std::shared_ptr<sw::redis::Redis> redis = suiRedis::RedisFactory::create(redisSettings);
        if (redis == nullptr)
        {
            ERROR("redis链接失败");
            return -1;
        }

        //创建数据库操作句柄
        suiOdb::odbSetting dbSetting;
        dbSetting._host = "sh-cdb-l9h13in4.sql.tencentcdb.com";
        dbSetting._port = 24411;
        dbSetting._user = "sui";
        dbSetting._password = "suisuipingan";
        dbSetting._database = "bilibili";
        dbSetting._connection_pool_size = 10;
        auto handler = suiOdb::dbFactory::create(dbSetting);
        if (handler == nullptr)
        {
            ERROR("数据库操作句柄创建失败");
            return -1;
        }
        
        //定义发布者
        INFO("开始创建mq客户端");
        std::shared_ptr<suiQueue::MQClient> mqclient = std::make_shared<suiQueue::MQClient>("amqp://sui:suisuipingan@127.0.0.1:8082/");
        INFO("创建removeCache缓存");
        //创建removeCache缓存
        auto removeCache = suiRemoveCache::RemoveCacheFactory::create(redis, mqclient);
        INFO("mq客户端创建完成");
        std::cin.get();
        INFO("开始进行测试");
        {
            INFO("开始添加会话");
            //创建事务
            INFO("创建事务");
            odb::transaction t(handler->begin());

            //创建session
            INFO("创建会话数据");
            suiSession::sessionData sedata(t.database(), *redis, removeCache);
            
            INFO("添加会话数据");
            suiDataSql::suiSessionMeta session;
            session.setSessionId("123456");
            session.setUserId(std::string("223456"));
            session.setUploadTime();
            INFO("添加会话操作");
            sedata.insert(session);
            t.commit();
            INFO("添加操作完成");
            std::cin.get();
        }
        
        {
            //查找
            INFO("开始查询会话");
            //创建事务
            odb::transaction t(handler->begin());

            //创建session
            auto& curdb = t.database();
            suiSession::sessionData sedata(curdb, *redis, removeCache);

            auto session = sedata.selectBySessionId("123456");
            if (session == nullptr) {
                ERROR("会话数据不存在");
                return 0;
            }

            INFO("会话数据查询成功");
            INFO("会话数据: {}", session->getSessionId());
            if (!session->getUserId().null()) 
                INFO("会话数据用户ID: {}", session->getUserId().get());

            INFO("会话数据上传时间: {}", session->getUploadTimeString());
            std::cin.get();
        }
        
        {
            //更新操作
            //查找
            //创建事务
            INFO("开始更新");
            odb::transaction t(handler->begin());

            //创建session
            auto& curdb = t.database();
            suiSession::sessionData sedata(curdb, *redis, removeCache);

            auto session = sedata.selectBySessionId("123456");
            if (session == nullptr) {
                ERROR("会话数据不存在");
                return 0;
            }

            INFO("会话数据查询成功");
            INFO("会话数据: {}", session->getSessionId());
            if (!session->getUserId().null()) 
                INFO("会话数据用户ID: {}", session->getUserId().get());

            INFO("会话数据上传时间: {}", session->getUploadTimeString());

            session->setUserId(std::string("33456"));
            sedata.update(*session);
            t.commit();
            INFO("更新操作完成");
            std::cin.get();
        }
        {
            //再次查找确认更新操作
            //创建事务
            INFO("确认更新");
            odb::transaction t(handler->begin());

            //创建session
            auto& curdb = t.database();
            suiSession::sessionData sedata(curdb, *redis, removeCache);

            auto session = sedata.selectBySessionId("123456");
            if (session == nullptr) {
                ERROR("会话数据不存在");
                return 0;
            }

            INFO("会话数据查询成功");
            INFO("会话数据: {}", session->getSessionId());
            if (!session->getUserId().null()) 
                INFO("会话数据用户ID: {}", session->getUserId().get());

            INFO("会话数据上传时间: {}", session->getUploadTimeString());
            t.commit();
            INFO("更新操作确认完成");
            std::cin.get();
        }
        {
            //删除数据
            //创建事务
            INFO("开始删除会话");
            odb::transaction t(handler->begin());

            //创建session
            auto& curdb = t.database();
            suiSession::sessionData sedata(curdb, *redis, removeCache);

            sedata.removeBySessionId("123456");
            t.commit();
            INFO("删除操作完成");
            std::cin.get();
        }
        {
            //查找
            INFO("开始确认删除成功");
            //创建事务
            odb::transaction t(handler->begin());

            //创建session
            auto& curdb = t.database();
            suiSession::sessionData sedata(curdb, *redis, removeCache);
            
            auto session = sedata.selectBySessionId("123456");
            if (session == nullptr) {
                ERROR("会话数据不存在");
            }
            else
            {
                ERROR("会话数据存在，删除失败");
            }
            t.commit();
            INFO("删除操作确认完成");
            std::cin.get();
        }
        {
            INFO("添加同用户多个会话");
            odb::transaction t(handler->begin());

            //创建session
            auto& curdb = t.database();
            suiSession::sessionData sedata(curdb, *redis, removeCache);

            suiDataSql::suiSessionMeta session;
            session.setSessionId("123456");
            session.setUserId(std::string("223456"));
            session.setUploadTime();
            sedata.insert(session);

            suiDataSql::suiSessionMeta session2;
            session2.setSessionId("1234567");
            session2.setUserId(std::string("223456"));
            session2.setUploadTime();
            sedata.insert(session2);

            suiDataSql::suiSessionMeta session3;
            session3.setSessionId("1234568");
            session3.setUserId(std::string("223456"));
            session3.setUploadTime();
            sedata.insert(session3);

            t.commit();
            INFO("添加操作完成");
            std::cin.get();
        }
        {
            //根据用户id进行删除会话
            INFO("根据用户id删除会话");
            odb::transaction t(handler->begin());

            //创建session
            auto& curdb = t.database();
            suiSession::sessionData sedata(curdb, *redis, removeCache);

            sedata.removeByUserId("223456");
            t.commit();
            INFO("删除操作完成");
            std::cin.get();
        }
        {
            INFO("开始确认删除成功");
            odb::transaction t(handler->begin());

            //创建session
            auto& curdb = t.database();
            suiSession::sessionData sedata(curdb, *redis, removeCache);

            auto session = sedata.selectBySessionId("123456");
            if (session == nullptr) {
                ERROR("会话数据不存在");
            }
            else{
                INFO("123456会话数据查询成功");
            }

            auto session2 = sedata.selectBySessionId("1234567");
            if (session2 == nullptr) {
                ERROR("会话数据不存在");
            }
            else{
                INFO("1234567会话数据查询成功");
            }

            auto session3 = sedata.selectBySessionId("1234568");
            if (session3 == nullptr) {
                ERROR("会话数据不存在");
            }
            else{
                INFO("1234568会话数据查询成功");
            }
            
            t.commit();
            INFO("删除操作确认完成");
            std::cin.get();
        }
    }
    INFO("所有操作完成");
    return 0;
}