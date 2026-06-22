#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>

//rpc监听端口
DEFINE_int32(file_server_listen_port, 9001, "服务监听端口");

DEFINE_string(file_server_amqp_addr, "amqp://sui:suisuipingan@localhost:8082//", "amqp地址");

DEFINE_int32(mysql_server_listen_port, 24411, "mysql服务监听端口");
DEFINE_string(mysql_server_registry_center, "sh-cdb-l9h13in4.sql.tencentcdb.com", "mysql注册中心地址");
DEFINE_string(mysql_server_user, "sui", "mysql用户名");
DEFINE_string(mysql_server_password, "suisuipingan", "mysql密码");
DEFINE_string(mysql_server_database, "bilibili", "mysql数据库");

DEFINE_string(redis_server_host, "127.0.0.1", "redis主机地址");
DEFINE_int32(redis_server_port, 8081, "redis端口");
DEFINE_string(redis_server_password, "suisuipingan", "redis密码");
DEFINE_string(redis_server_user, "default", "redis用户名");

//设置文件删除队列的mq设置
DEFINE_string(file_server_remove_exchange, "file_remove_exchange", "文件删除交换机名称");
DEFINE_string(file_server_remove_exchange_type, "direct", "文件删除队列交换机类型");
DEFINE_string(file_server_remove_queue, "file_remove_queue", "文件删除队列名称");
DEFINE_string(file_server_remove_bind_key, "file_remove_key", "文件删除队列绑定键");

//设置fdfs配置
DEFINE_string(file_server_fdfs_addr, "127.0.0.1:22122", "fdfs地址");

//注册中心
DEFINE_string(file_server_registry_center, "127.0.0.1:8084", "file_server注册中心地址");

int main(int argc, char* argv[])
{
    //初始化日志
    suiUtil::suiLogInitDefault();
    //解析gflags
    google::ParseCommandLineFlags(&argc, &argv, true);
    
    //
    {
        //测试服务注册
        suiEtcd::serProvider pro("test", "http://127.0.0.1:8084");
        pro.setSerAddr("127.0.0.1:1212");
        pro.redister();
        INFO("注册成功");
    }
    return 0;
}
