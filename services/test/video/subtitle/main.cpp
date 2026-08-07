#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiJson.hpp>
#include "video_subtitle_target.hpp"

//注册中心地址
DEFINE_string(file_server_registry_center, "127.0.0.1:8084", "file_server注册中心地址");
//rpc监听端口
DEFINE_int32(file_server_listen_port, 9001, "服务监听端口");

//设置fdfs配置
DEFINE_string(file_server_fdfs_addr, "127.0.0.1:22122", "fdfs地址");

//消息队列
DEFINE_string(file_server_amqp_addr, "amqp://sui:suisuipingan@localhost:8082//", "amqp地址");

//mysql配置
DEFINE_int32(mysql_server_listen_port, 24411, "mysql服务监听端口");
DEFINE_string(mysql_server_registry_center, "sh-cdb-l9h13in4.sql.tencentcdb.com", "mysql注册中心地址");
DEFINE_string(mysql_server_user, "sui", "mysql用户名");
DEFINE_string(mysql_server_password, "suisuipingan", "mysql密码");
DEFINE_string(mysql_server_database, "bilibili", "mysql数据库");

//redis配置
DEFINE_string(redis_server_host, "127.0.0.1", "redis主机地址");
DEFINE_int32(redis_server_port, 8081, "redis端口");
DEFINE_string(redis_server_password, "suisuipingan", "redis密码");
DEFINE_string(redis_server_user, "default", "redis用户名");

//设置文件删除队列的mq设置
DEFINE_string(file_server_remove_exchange, "file_remove_exchange", "文件删除交换机名称");
DEFINE_string(file_server_remove_exchange_type, "direct", "文件删除队列交换机类型");
DEFINE_string(file_server_remove_queue, "file_remove_queue", "文件删除队列名称");
DEFINE_string(file_server_remove_bind_key, "file_remove_key", "文件删除队列绑定键");

//添加弹幕接口
void addBulletChat(std::shared_ptr<odb::database> mysql, std::shared_ptr<sw::redis::Redis> redis
    , suiRemoveCache::RemoveCache::ptr curRemoveCache, suiVideoSubtitleTarget::videoSubtitleAsync::ptr curBulletChatSync
    , const std::string& userid, const std::string& video_id
    , const std::string& subtitleid, const std::string& content)
{
    try
    {
        //构建事务
        odb::transaction tx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        suiVideoSubtitleTarget::videoSubtitleTarget target(tx, rtx, curBulletChatSync);
        target.insert(userid, video_id, subtitleid, content, 1000);
        tx.commit();
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("odb数据库添加弹幕异常： {}", e.what());
    }
    catch (const sw::redis::Error& e) 
    {
        // 捕获 Redis 异常
        ERROR("redis数据库添加弹幕异常： {}", e.what());
    }
    catch (...)
    {
        ERROR("添加弹幕时未知异常");
    }
}

void removeBulletChat(std::shared_ptr<odb::database> mysql, std::shared_ptr<sw::redis::Redis> redis
    , suiRemoveCache::RemoveCache::ptr curRemoveCache, suiVideoSubtitleTarget::videoSubtitleAsync::ptr curBulletChatSync
    , const std::string& video_id, const std::string& subtitleid)
{
    try
    {
        //构建事务
        odb::transaction tx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        suiVideoSubtitleTarget::videoSubtitleTarget target(tx, rtx, curBulletChatSync);
        target.removeBySubtitleid(video_id, subtitleid);
        tx.commit();
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("odb数据库添加弹幕异常： {}", e.what());
    }
    catch (const sw::redis::Error& e) 
    {
        // 捕获 Redis 异常
        ERROR("redis数据库添加弹幕异常： {}", e.what());
    }
    catch (...)
    {
        ERROR("添加弹幕时未知异常");
    }
}

void removeAllBulletChat(std::shared_ptr<odb::database> mysql, std::shared_ptr<sw::redis::Redis> redis
    , suiRemoveCache::RemoveCache::ptr curRemoveCache, suiVideoSubtitleTarget::videoSubtitleAsync::ptr curBulletChatSync
    , const std::string& video_id)
{
    try
    {
        //构建事务
        odb::transaction tx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        suiVideoSubtitleTarget::videoSubtitleTarget target(tx, rtx, curBulletChatSync);
        target.removeByVideoid(video_id);
        tx.commit();
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("odb数据库添加弹幕异常： {}", e.what());
    }
    catch (const sw::redis::Error& e) 
    {
        // 捕获 Redis 异常
        ERROR("redis数据库添加弹幕异常： {}", e.what());
    }
    catch (...)
    {
        ERROR("添加弹幕时未知异常");
    }
}

int main(int argc, char* argv[])
{
    //初始化日志
    suiUtil::suiLogInitDefault();
    //解析gflags
    google::ParseCommandLineFlags(&argc, &argv, true);

    //数据库设置
    suiOdb::odbSetting dbSetting;
    dbSetting._host = FLAGS_mysql_server_registry_center;
    dbSetting._port = FLAGS_mysql_server_listen_port;
    dbSetting._user = FLAGS_mysql_server_user;
    dbSetting._password = FLAGS_mysql_server_password;
    dbSetting._database = FLAGS_mysql_server_database;
    //创建数据库操作句柄db
    std::shared_ptr<odb::database> curDb = suiOdb::dbFactory::create(dbSetting);

    //设置redis
    //设置redis
    suiRedis::redisSettings redisset;
    redisset._host = FLAGS_redis_server_host;
    redisset._port = FLAGS_redis_server_port;
    redisset._password = FLAGS_redis_server_password;
    redisset._user = FLAGS_redis_server_user;
    //创建redis
    auto curRedis = suiRedis::RedisFactory::create(redisset);

    ///创建mq客户端
    suiQueue::MQClient::ptr _mqClienrt = std::make_shared<suiQueue::MQClient>(FLAGS_file_server_amqp_addr);

    //删除队列
    suiRemoveCache::RemoveCache::ptr curRemoveCache = suiRemoveCache::RemoveCacheFactory::create(curRedis, _mqClienrt);
    //异步弹幕队列
    suiVideoSubtitleTarget::videoSubtitleAsync::ptr curBulletChatSync = std::make_shared<suiVideoSubtitleTarget::videoSubtitleAsync>(curDb, curRedis, _mqClienrt);
    INFO("点击回车开始测试");
    std::cin.get();
    INFO("开始添加弹幕");
    {
        addBulletChat(curDb, curRedis, curRemoveCache, curBulletChatSync, "user_1", "video_1", "1", "1111111111");
        addBulletChat(curDb, curRedis, curRemoveCache, curBulletChatSync, "user_2", "video_1", "2", "565656");
        addBulletChat(curDb, curRedis, curRemoveCache, curBulletChatSync, "user_1", "video_1", "3", "786786246");
        addBulletChat(curDb, curRedis, curRemoveCache, curBulletChatSync, "user_3", "video_1", "4", "546687");
        addBulletChat(curDb, curRedis, curRemoveCache, curBulletChatSync, "user_4", "video_1", "5", "465486");
        addBulletChat(curDb, curRedis, curRemoveCache, curBulletChatSync, "user_5", "video_1", "6", "sss");
    }
    INFO("添加弹幕完成, 点击回车继续");
    std::cin.get();
    INFO("开始删除单个弹幕");
    {
        removeBulletChat(curDb, curRedis, curRemoveCache, curBulletChatSync, "video_1", "1");
        removeBulletChat(curDb, curRedis, curRemoveCache, curBulletChatSync, "video_1", "2");
    }
    INFO("删除单个弹幕完成, 点击回车继续");
    std::cin.get();
    INFO("开始删除整个视频弹幕");
    {
        removeAllBulletChat(curDb, curRedis, curRemoveCache, curBulletChatSync, "video_1");
    }
    INFO("删除整个视频弹幕完成, 点击回车继续");
    std::cin.get();
    INFO("本次测试结束, 点击回车退出");
    std::cin.get();
    return 0;
}
