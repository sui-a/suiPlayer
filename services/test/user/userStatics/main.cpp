#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include "db_set_async.hpp"
#include "user_data_statistics.hpp"

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


//获取基础信息
void getBasicInfo(std::shared_ptr<odb::database> curDb, std::shared_ptr<sw::redis::Redis> curRedis
                , suiRemoveCache::RemoveCache::ptr curRemoveCache
                , const std::string& userId)
{
    try{
        INFO("user {}基础数据的获取", userId);

        //创建数据库事务
        odb::transaction t(curDb->begin());
        auto& dbHandler = t.database();
        
        //创建redis事务
        auto rtx = curRedis->transaction(false, false);
        auto rehandler = rtx.redis();

        //创建信息操作对象
        suiUserStatics::suiStatics stats(dbHandler, rehandler, curRemoveCache, rtx);

        //获取信息
        auto it = stats.getUserBasicData(userId);

        if(!it)
        {
            //存在
        }

        //打印信息
        INFO("user {} 信息获取成功", userId);
        INFO("user {} 粉丝数量: {}", userId, it->fansCount);
        INFO("user {} 关注数量: {}", userId, it->followCount);
        INFO("user {} 总播放数量: {}", userId, it->videoPlayCount);
        INFO("user {} 点赞视频数量: {}", userId, it->videoLikeCount);

        t.commit();
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("数据库异常： {}", e.what());
        return;
    }
    catch (const sw::redis::Error& e) 
    {
        // 捕获 Redis 异常
        ERROR("redis异常： {}", e.what());
        return;
    }
    catch (...)
    {
        ERROR("未知异常");
        return;
    }
    //
}

//点赞量+10接口
void likeCountSetText(std::shared_ptr<odb::database> curDb, std::shared_ptr<sw::redis::Redis> curRedis
                , suiRemoveCache::RemoveCache::ptr curRemoveCache
                , const std::string& userId)
{
    try{
        INFO("user {}基础数据的获取", userId);

        //创建数据库事务
        odb::transaction t(curDb->begin());
        auto& dbHandler = t.database();
        
        //创建redis事务
        auto rtx = curRedis->transaction(false, false);
        auto rehandler = rtx.redis();

        //创建信息操作对象
        suiUserStatics::suiStatics stats(dbHandler, rehandler, curRemoveCache, rtx);

        //获取信息
        stats.setVideoLikeCountByChange(userId, 10);
        rtx.exec();
        t.commit();
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("数据库异常： {}", e.what());
        return;
    }
    catch (const sw::redis::Error& e) 
    {
        // 捕获 Redis 异常
        ERROR("redis异常： {}", e.what());
        return;
    }
    catch (...)
    {
        ERROR("未知异常");
        return;
    }
    //
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
    
    {
        //数据获取正确
        getBasicInfo(curDb, curRedis, curRemoveCache, "user_A");
        /*
        getBasicInfo(curDb, curRedis, curRemoveCache, setUserStatistics, "user_B");
        getBasicInfo(curDb, curRedis, curRemoveCache, setUserStatistics, "user_C");
        getBasicInfo(curDb, curRedis, curRemoveCache, setUserStatistics, "user_D");
        getBasicInfo(curDb, curRedis, curRemoveCache, setUserStatistics, "user_E");
        getBasicInfo(curDb, curRedis, curRemoveCache, setUserStatistics, "user_F");*/
    }
    INFO("数据获取完成, 点击回车开始测试数据修改");
    std::cin.get();

    //开始测试数据修改
    {
        //直接让user_A总点赞量+10
        likeCountSetText(curDb, curRedis, curRemoveCache, "user_A");
        INFO("user_A 点赞量+10 完成");
        INFO("查看redis是否改变");
        //再获取一遍数据，查看redis中是否改变
        getBasicInfo(curDb, curRedis, curRemoveCache, "user_A");
    }
    INFO("本次测试结束");
    return 0;
}
