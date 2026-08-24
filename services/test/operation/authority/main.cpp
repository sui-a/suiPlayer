#include <suiScaffold/log.h>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiRpc.hpp>
#include <gflags/gflags.h>
#include <iostream>
#include <fstream>
#include <suiScaffold/suiHash.hpp>
#include "base.pb.h"
#include "user.pb.h"
#include "optionalaAuthority.hpp"

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

    suiOdb::odbSetting odbset;
    odbset._host = FLAGS_mysql_server_registry_center;
    odbset._port = FLAGS_mysql_server_listen_port;
    odbset._user = FLAGS_mysql_server_user;
    odbset._password = FLAGS_mysql_server_password;
    odbset._database = FLAGS_mysql_server_database;

    suiRedis::redisSettings redisset;
    redisset._host = FLAGS_redis_server_host;
    redisset._port = FLAGS_redis_server_port;
    redisset._password = FLAGS_redis_server_password;
    redisset._user = FLAGS_redis_server_user;

    //创建操作句柄
    auto redis = suiRedis::RedisFactory::create(redisset);
    auto mysql = suiOdb::dbFactory::create(odbset);

    {
        //查看某个操作是否存在
        odb::transaction tx(mysql->begin());
        auto rtx = redis->transaction(false, false);
        //创建操作句柄
        auto& dbhandle = tx.database();
        auto rehandle = rtx.redis();
        suiOptionalaAuthority::OptionalaAuthority optionalaAuthority(dbhandle, rehandle);
        auto ret = optionalaAuthority.select("/rpc/user/touristLogin");
        if(ret)
        {
            INFO("用户申请临时会话操作存在");
        }
        else
        {
            INFO("操作不存在");
        }
        tx.commit();
    }
    return 0;
}