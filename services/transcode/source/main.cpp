#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include "transcodeServer.hpp"

//mysql配置
DEFINE_int32(mysql_server_listen_port, 24411, "mysql服务监听端口");
DEFINE_string(mysql_server_registry_center, "sh-cdb-l9h13in4.sql.tencentcdb.com", "mysql注册中心地址");
DEFINE_string(mysql_server_user, "sui", "mysql用户名");
DEFINE_string(mysql_server_password, "suisuipingan", "mysql密码");
DEFINE_string(mysql_server_database, "bilibili", "mysql数据库");
//消息队列
DEFINE_string(amqp_addr, "amqp://sui:suisuipingan@localhost:8082//", "amqp地址");
//转码消息队列配置
DEFINE_string(video_transcode_exchange, "video_transcode_exchange", "转码交换机名称");
DEFINE_string(video_transcode_exchange_type, "direct", "转码队列交换机类型");
DEFINE_string(video_transcode_queue, "video_transcode_queue", "转码队列名称");
DEFINE_string(video_transcode_bind_key, "video_transcode_key", "转码队列绑定键");
//临时路径 默认 ./temp_transcode
DEFINE_string(video_transcode_temp_path, "./temp_transcode", "转码临时路径");
//请求前缀
DEFINE_string(video_transcode_request_prefix, "", "转码请求前缀");
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
    
    {
        suiTranscodeServer::serverBuilder builder;
        {
            //设置mysql配置
            suiOdb::odbSetting odbset;
            odbset._host = FLAGS_mysql_server_registry_center;
            odbset._port = FLAGS_mysql_server_listen_port;
            odbset._user = FLAGS_mysql_server_user;
            odbset._password = FLAGS_mysql_server_password;
            odbset._database = FLAGS_mysql_server_database;
            builder.setOdb(odbset);
        }
        {
            //设置消息队列
            builder.setMqUrl(FLAGS_amqp_addr);
        }
        {
            //设置转码队列
            suiQueue::queueSetting transcodeQueueSetting;
            transcodeQueueSetting.exchange = FLAGS_video_transcode_exchange;
            transcodeQueueSetting.exchangeType = FLAGS_video_transcode_exchange_type;
            transcodeQueueSetting.queue = FLAGS_video_transcode_queue;
            transcodeQueueSetting.bindKey = FLAGS_video_transcode_bind_key;
            builder.setListenMq(transcodeQueueSetting);
        }
        {
            //设置临时路径
            builder.setTempFilePath(FLAGS_video_transcode_temp_path);
        }
        {
            //转码配置
            suiPeg::hlsSettings hlsSettings;
            hlsSettings.hls_time = 10;
            hlsSettings.playlistType = "vod";
        }
        {
            //设置请求前缀
            builder.setRequestPrefix(FLAGS_video_transcode_request_prefix);
        }
        {
            //文件删除队列
            //文件删除队列配置
            suiQueue::queueSetting removeQueueSetting;
            removeQueueSetting.exchange = FLAGS_remove_exchange;
            removeQueueSetting.exchangeType = FLAGS_remove_exchange_type;
            removeQueueSetting.queue = FLAGS_remove_queue;
            removeQueueSetting.bindKey = FLAGS_remove_bind_key;
            //传入文件删除队列的mq设置
            builder.setFileRemoveMq(removeQueueSetting);
        }
        auto server = builder.build();
        server->start();
    }

    return 0;
}
