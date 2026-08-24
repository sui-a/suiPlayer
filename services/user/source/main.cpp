#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiMail.hpp>
#include "userServer.hpp"
#include "suiIp.hpp"

//注册中心地址
DEFINE_string(registry_center, "127.0.0.1:8084", "user_server注册中心地址");
//rpc监听端口
DEFINE_int32(user_server_listen_port, 9003, "user_server服务监听端口");

//消息队列
DEFINE_string(amqp_addr, "amqp://sui:suisuipingan@localhost:8082//", "amqp地址");

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

//验证码服务发送参数设置
DEFINE_string(imap_from_set, "2076354958@qq.com", "imap发送邮箱");
DEFINE_string(imap_username_set, "2076354958@qq.com", "imap发送邮箱");
DEFINE_string(imap_passward, "uddbtalnaergciaf", "imap发送邮箱");
DEFINE_string(imap_url_set, "smtps://smtp.qq.com:465", "imap发送邮箱");

//设置文件删除队列的mq设置
DEFINE_string(remove_exchange, "file_remove_exchange", "文件删除交换机名称");
DEFINE_string(remove_exchange_type, "direct", "文件删除队列交换机类型");
DEFINE_string(remove_queue, "file_remove_queue", "文件删除队列名称");
DEFINE_string(remove_bind_key, "file_remove_key", "文件删除队列绑定键");


int main(int argc, char* argv[])
{
    //初始化日志
    suiUtil::suiLogInitDefault();
    //解析gflags
    google::ParseCommandLineFlags(&argc, &argv, true);

    //创建构造类
    {
        suiUser::userServerBuild userServerBuild;
        
        //设置etcd
        {
            suiUser::registrySettings registrySet;
            registrySet.registry_center = FLAGS_registry_center;
            registrySet.service_name = "user_server";
            registrySet.service_addr = suiIp::suiIper::GetLocalIP().value() + ":" + std::to_string(FLAGS_user_server_listen_port);
            userServerBuild.setRegistrySettings(registrySet);
        }

        //设置服务监听
        {
            userServerBuild.setListenPort(FLAGS_user_server_listen_port);
        }

        //设置消息队列
        {
            userServerBuild.setMqSettings(FLAGS_amqp_addr);
        }
        //设置文件删除队列
        {
            suiQueue::queueSetting removeQueueSetting;
            removeQueueSetting.exchange = FLAGS_remove_exchange;
            removeQueueSetting.exchangeType = FLAGS_remove_exchange_type;
            removeQueueSetting.queue = FLAGS_remove_queue;
            removeQueueSetting.bindKey = FLAGS_remove_bind_key;
            userServerBuild.setFileRemoveSetting(removeQueueSetting);
        }

        //设置数据库
        {
            suiOdb::odbSetting odbset;
            odbset._host = FLAGS_mysql_server_registry_center;
            odbset._port = FLAGS_mysql_server_listen_port;
            odbset._user = FLAGS_mysql_server_user;
            odbset._password = FLAGS_mysql_server_password;
            odbset._database = FLAGS_mysql_server_database;
            //传入odbset
            userServerBuild.setOdbSetting(odbset);

            suiRedis::redisSettings redisset;
            redisset._host = FLAGS_redis_server_host;
            redisset._port = FLAGS_redis_server_port;
            redisset._password = FLAGS_redis_server_password;
            redisset._user = FLAGS_redis_server_user;
            //传入redis
            userServerBuild.setRedisSetting(redisset);
        }

        //设置邮箱
        {
            suiMail::MailSetting set;
            set.from = "2076354958@qq.com";
            set.username = "2076354958@qq.com";
            set.password = "uddbtalnaergciaf";
            set.url = "smtps://smtp.qq.com:465";
            userServerBuild.setImapSetting(set);
        }

        auto userServer = userServerBuild.build();
        if(!userServer)
        {
            INFO("服务构建失败");
        }
        INFO("服务构建成功，开始运行");
        userServer->start();
    }

    return 0;
}