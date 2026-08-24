#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include "getwayServerRpc.hpp"
#include "getwayServer.hpp"

//注册中心地址
DEFINE_string(registry_center, "127.0.0.1:8084", "注册中心地址");
//rpc监听端口
DEFINE_int32(server_listen_port, 9010, "服务监听端口");

//子服务名称
DEFINE_string(user_server_name, "user_server", "子服务名称");
DEFINE_string(video_server_name, "video_server", "子服务名称");
DEFINE_string(file_server_name, "file_server", "子服务名称");

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


int main(int argc, char* argv[])
{
    //初始化日志
    suiUtil::suiLogInitDefault();
    //解析gflags
    google::ParseCommandLineFlags(&argc, &argv, true);

    suiGetwayServer::getwayServerBuild serverBuild;
    {
        //设置子服务名称
        serverBuild.setUserServerName(FLAGS_user_server_name);
        serverBuild.setVideoServerName(FLAGS_video_server_name);
        serverBuild.setFileServerName(FLAGS_file_server_name);
    }

    {
        //设置数据库
        suiOdb::odbSetting odbset;
        odbset._host = FLAGS_mysql_server_registry_center;
        odbset._port = FLAGS_mysql_server_listen_port;
        odbset._user = FLAGS_mysql_server_user;
        odbset._password = FLAGS_mysql_server_password;
        odbset._database = FLAGS_mysql_server_database;
        //传入odbset
        serverBuild.setOdb(odbset);
    }
    {
        //设置缓存库
        suiRedis::redisSettings redisset;
        redisset._host = FLAGS_redis_server_host;
        redisset._port = FLAGS_redis_server_port;
        redisset._password = FLAGS_redis_server_password;
        redisset._user = FLAGS_redis_server_user;
        //传入redis
        serverBuild.setRedis(redisset);
    }
    {
        //设置注册中心地址
        serverBuild.setRegistryCenter(FLAGS_registry_center);
    }
    {
        //设置监听地址
        serverBuild.setListenPort(FLAGS_server_listen_port);
    }
    //开始构造
    auto server = serverBuild.build();
    if(server == nullptr)
    {
        ERROR("服务创建失败");
        return 0;
    }
    //启动服务
    server->start();
    return 0;
}
