#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>
#include "fileService.hpp"
#include "suiIp.hpp"

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


int main(int argc, char* argv[])
{
    //初始化日志
    suiUtil::suiLogInitDefault();
    //解析gflags
    google::ParseCommandLineFlags(&argc, &argv, true);

    {
        //创建build对象
        suiFileService::suifileServerBuiler build;

        //设置服务监听端口
        build.setListenPort(FLAGS_file_server_listen_port);

        //设置etcd
        suiFileService::registrySettings registrySet;
        registrySet.registry_center = FLAGS_file_server_registry_center;
        registrySet.service_name = "file_server";
        registrySet.service_addr = suiIp::suiIper::GetLocalIP().value() + ":" + std::to_string(FLAGS_file_server_listen_port);
        //传入etcd
        build.setRegistrySettings(registrySet);

        //fdfs设置
        suifd::FastdfsSetting fdset;
        fdset.Addr = FLAGS_file_server_fdfs_addr;
        //传入fdfs
        build.setFastdfsSetting(fdset);
        
        //设置odb配置
        suiOdb::odbSetting odbset;
        odbset._host = FLAGS_mysql_server_registry_center;
        odbset._port = FLAGS_mysql_server_listen_port;
        odbset._user = FLAGS_mysql_server_user;
        odbset._password = FLAGS_mysql_server_password;
        odbset._database = FLAGS_mysql_server_database;
        //传入odbset
        build.setOdbSetting(odbset);

        //设置redis
        suiRedis::redisSettings redisset;
        redisset._host = FLAGS_redis_server_host;
        redisset._port = FLAGS_redis_server_port;
        redisset._password = FLAGS_redis_server_password;
        redisset._user = FLAGS_redis_server_user;
        //传入redis
        build.setRedisSetting(redisset);

        //设置mq路径
        build.setQueueUrl(FLAGS_file_server_amqp_addr);

        //设置文件删除队列的mq设置
        suiQueue::queueSetting removeQueueSetting;
        removeQueueSetting.exchange = FLAGS_file_server_remove_exchange;
        removeQueueSetting.exchangeType = FLAGS_file_server_remove_exchange_type;
        removeQueueSetting.queue = FLAGS_file_server_remove_queue;
        removeQueueSetting.bindKey = FLAGS_file_server_remove_bind_key;
        //传入文件删除队列的mq设置
        build.setRemoveQueueSetting(removeQueueSetting);

        //创建文件服务
        auto fileServerPtr = build.build();
        if(fileServerPtr == nullptr)
        {
            ERROR("文件服务创建失败");
            return -1;
        }
        INFO("文件服务创建成功");
        fileServerPtr->start();
    }
    
    return 0;
}
