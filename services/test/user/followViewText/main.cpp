#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiodb.hpp>
#include "data.hpp"
#include "data-odb.hxx"

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


//查询关注用户的数量
void getFollowUserCount(std::shared_ptr<odb::database> curDb, const std::string& userId)
{
    try
    {
        //创建odb事务
        odb::transaction tx(curDb->begin());
        //创建单连接操作句柄
        auto& curhandle = tx.database();
        
        //开始测试
        INFO("查询A用户关注的所有用户");
        auto ret = curhandle.query_one<suiDataSql::suiUserFollowCountView>(odb::query<suiDataSql::suiUserFollowCountView>::user_id == userId);
        tx.commit();
        if(ret == nullptr)
        {
            INFO("查询结果为空，出现错误");
            return;
        }
        INFO("查询成功, 用户关注数量为： {}", ret->count);
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("数据库异常： {}", e.what());
        return;
    }
    catch (...)
    {
        ERROR("未知异常");
        return;
    }
}

//获取粉丝数量
void getFansCount(std::shared_ptr<odb::database> curDb, const std::string& userId)
{
    try
    {
        //创建odb事务
        odb::transaction tx(curDb->begin());
        //创建单连接操作句柄
        auto& curhandle = tx.database();
        
        //开始测试
        INFO("查询用户 {} 关注的所有用户", userId);
        auto ret = curhandle.query_one<suiDataSql::suiUserFollowCountView>(odb::query<suiDataSql::suiUserFollowCountView>::follow_user_id == userId);
        tx.commit();
        if(ret == nullptr)
        {
            INFO("查询结果为空，出现错误");
            return;
        }
        INFO("查询成功, 用户粉丝数量为： {}", ret->count);
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("数据库异常： {}", e.what());
        return;
    }
    catch (...)
    {
        ERROR("未知异常");
        return;
    }
}


//获取用户基础信息集合
void getBasicInformationCollection(std::shared_ptr<odb::database> curDb, const std::string& userId)
{
    try
    {
        //创建odb事务
        odb::transaction tx(curDb->begin());
        //创建单连接操作句柄
        auto& curhandle = tx.database();
        
        //开始测试
        INFO("查询A用户关注的所有用户");
        auto ret = curhandle.query_one<suiDataSql::suiUserDataCountView>(odb::query<suiDataSql::suiUserDataCountView>::user::user_id == userId);
        tx.commit();
        if(ret == nullptr)
        {
            INFO("查询结果为空，出现错误");
            return;
        }
        INFO("查询成功, 用户粉丝数量为： {}", ret->followed_count);
        INFO("用户关注数量为： {}", ret->following_count);
        INFO("用户基础信息为： 用户id： {}", ret->user->getUserId());
        INFO("用户名： {}", ret->user->getUserName());
        INFO("用户绑定邮箱： {}", ret->user->getBindEmail());
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("数据库异常： {}", e.what());
        return;
    }
    catch (...)
    {
        ERROR("未知异常");
        return;
    }
}


int main(int argc, char* argv[])
{
    //初始化日志
    suiUtil::suiLogInitDefault();
    //解析gflags
    google::ParseCommandLineFlags(&argc, &argv, true);
    INFO("对用户视图进行测试");

    //数据库设置
    suiOdb::odbSetting dbSetting;
    dbSetting._host = FLAGS_mysql_server_registry_center;
    dbSetting._port = FLAGS_mysql_server_listen_port;
    dbSetting._user = FLAGS_mysql_server_user;
    dbSetting._password = FLAGS_mysql_server_password;
    dbSetting._database = FLAGS_mysql_server_database;
    //创建数据库操作句柄db
    std::shared_ptr<odb::database> curDb = suiOdb::dbFactory::create(dbSetting);
    {
        //查询关注用户的数量
        getFollowUserCount(curDb, "user_A");
        getFollowUserCount(curDb, "user_B");
        getFollowUserCount(curDb, "user_C");
        getFollowUserCount(curDb, "user_D");
        getFollowUserCount(curDb, "user_E");
        getFollowUserCount(curDb, "user_F");
    }

    {
        //查询被粉丝的数量
        getFansCount(curDb, "user_A");
        getFansCount(curDb, "user_B");
        getFansCount(curDb, "user_C");
        getFansCount(curDb, "user_D");
        getFansCount(curDb, "user_E");
        getFansCount(curDb, "user_F");
    }

    {
        //获取各个用户的基础信息集
    }

    return 0;
}
