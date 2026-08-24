#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiMail.hpp>
#include "videoServer.hpp"
#include "suiIp.hpp"

//etcd地址
DEFINE_string(registry_center, "127.0.0.1:8084", "注册中心地址");
//服务注册名称
DEFINE_string(service_name, "video_server", "服务注册名称");
//rpc监听端口
DEFINE_int32(video_server_listen_port, 9004, "服务监听端口");
//rabbit地址
DEFINE_string(amqp_addr, "amqp://sui:suisuipingan@localhost:8082//", "amqp地址");
//数据库配置
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
//es配置
DEFINE_string(es_url, "http://elastic:suisuipingan@127.0.0.1:8085/", "es地址");
//文件删除队列配置
DEFINE_string(file_remove_exchange, "file_remove_exchange", "文件删除交换机名称");
DEFINE_string(file_remove_exchange_type, "direct", "文件删除队列交换机类型");
DEFINE_string(file_remove_queue, "file_remove_queue", "文件删除队列名称");
DEFINE_string(file_remove_bind_key, "file_remove_key", "文件删除队列绑定键");
//转码消息队列配置
DEFINE_string(video_transcode_exchange, "video_transcode_exchange", "转码交换机名称");
DEFINE_string(video_transcode_exchange_type, "direct", "转码队列交换机类型");
DEFINE_string(video_transcode_queue, "video_transcode_queue", "转码队列名称");
DEFINE_string(video_transcode_bind_key, "video_transcode_key", "转码队列绑定键");

int main(int argc, char* argv[])
{
    //初始化日志
    suiUtil::suiLogInitDefault();
    //解析gflags
    google::ParseCommandLineFlags(&argc, &argv, true);
    //创建服务构建类
    suiVideoServer::videoServerBuild serverBuild;
    
    {
        //设置etcd
        suiVideoServer::registrySettings registrySet;
        registrySet.registry_center = FLAGS_registry_center;
        registrySet.service_name = FLAGS_service_name;
        registrySet.service_addr = suiIp::suiIper::GetLocalIP().value() + ":" + std::to_string(FLAGS_video_server_listen_port);
        serverBuild.setRegistrySettings(registrySet);
    }
    {
        //设置消息队列地址
        serverBuild.setMqSettings(FLAGS_amqp_addr);
    }
    {
        //设置数据库
        suiOdb::odbSetting odbset;
        odbset._host = FLAGS_mysql_server_registry_center;
        odbset._port = FLAGS_mysql_server_listen_port;
        odbset._user = FLAGS_mysql_server_user;
        odbset._password = FLAGS_mysql_server_password;
        odbset._database = FLAGS_mysql_server_database;
        serverBuild.setOdb(odbset);
    }
    {
        //设置缓存
        suiRedis::redisSettings redisset;
        redisset._host = FLAGS_redis_server_host;
        redisset._port = FLAGS_redis_server_port;
        redisset._password = FLAGS_redis_server_password;
        redisset._user = FLAGS_redis_server_user;
        serverBuild.setRedis(redisset);
    }
    {
        //设置es路径
        serverBuild.setEsUrl(FLAGS_es_url);
    }
    {
        //设置文件删除队列
        suiQueue::queueSetting removeQueueSetting;
        removeQueueSetting.exchange = FLAGS_file_remove_exchange;
        removeQueueSetting.exchangeType = FLAGS_file_remove_exchange_type;
        removeQueueSetting.queue = FLAGS_file_remove_queue;
        removeQueueSetting.bindKey = FLAGS_file_remove_bind_key;
        serverBuild.setFileRemoveSetting(removeQueueSetting);
    }
    {
        //设置转码队列
        suiQueue::queueSetting transcodeQueueSetting;
        transcodeQueueSetting.exchange = FLAGS_video_transcode_exchange;
        transcodeQueueSetting.exchangeType = FLAGS_video_transcode_exchange_type;
        transcodeQueueSetting.queue = FLAGS_video_transcode_queue;
        transcodeQueueSetting.bindKey = FLAGS_video_transcode_bind_key;
        serverBuild.setTranscodeSetting(transcodeQueueSetting);
    }
    {
        //设置监听地址
        serverBuild.setListenPort(FLAGS_video_server_listen_port);
    }
    auto ret = serverBuild.build();
    if(ret == nullptr)
    {
        INFO("服务创建失败");
        return 0;
    }
    //阻塞执行流程
    ret->start();
    return 0;
}


