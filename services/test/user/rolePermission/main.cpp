#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiodb.hpp>
#include "role_permission.hpp"

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

    //创建rolePermission
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

    INFO("开始测试rolePermission");
    INFO("添加单个url");
    {
        try
        {
            //创建数据库事务
            odb::transaction t(curDb->begin());
            auto& dbHandler = t.database();
            
            //创建redis事务
            auto rtx = curRedis->transaction(false, false);
            auto rehandler = rtx.redis();

            //开始创建rolePermission对象
            auto curRolePermission = std::make_shared<suiRolePermission::rolePermission>(dbHandler, rehandler, curRemoveCache);

            //开始添加
            curRolePermission->insert(suiDataSql::roleType::roleTypeUnknown, "http://www.baidu.com");
            t.commit();
            INFO("添加成功, 点击回车继续运行");
            std::cin.get();
            INFO("开始测试查询功能");
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("数据库异常： {}", e.what());
            return -1;
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis异常： {}", e.what());
            return -1;
        }
        catch (...)
        {
            ERROR("未知异常");
            return -1;
        }
    }

    {
        try
        {
            //创建数据库事务
            odb::transaction t(curDb->begin());
            auto& dbHandler = t.database();
            
            //创建redis事务
            auto rtx = curRedis->transaction(false, false);
            auto rehandler = rtx.redis();

            //开始创建rolePermission对象
            auto curRolePermission = std::make_shared<suiRolePermission::rolePermission>(dbHandler, rehandler, curRemoveCache);

            //开始添加
            auto ret = curRolePermission->getPermissionsByOperation("http://www.baidu.com");
            t.commit();
            if(!ret)
            {
                //不存在
                ERROR("查询失败, url不存在");
                return -1;
            }
            //存在
            INFO("查询成功, url存在, 权限为: {}", suiDataSql::suiRoleType::roleTypeToString(ret->getRoleType()));
            INFO("单次查询成功, 点击回车继续运行");
            std::cin.get();
            INFO("开始测试缓存查询功能");
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("数据库异常： {}", e.what());
            return -1;
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis异常： {}", e.what());
            return -1;
        }
        catch (...)
        {
            ERROR("未知异常");
            return -1;
        }
    }

    {
        try
        {
            //创建数据库事务
            odb::transaction t(curDb->begin());
            auto& dbHandler = t.database();
            
            //创建redis事务
            auto rtx = curRedis->transaction(false, false);
            auto rehandler = rtx.redis();

            //开始创建rolePermission对象
            auto curRolePermission = std::make_shared<suiRolePermission::rolePermission>(dbHandler, rehandler, curRemoveCache);

            //开始添加
            auto ret = curRolePermission->getPermissionsByOperation("http://www.baidu.com");
            t.commit();
            if(!ret)
            {
                //不存在
                ERROR("查询失败, url不存在");
                return -1;
            }
            //存在
            INFO("查询成功, url存在, 权限为: {}", suiDataSql::suiRoleType::roleTypeToString(ret->getRoleType()));
            INFO("缓存查询功能测试成功, 点击回车继续运行");
            std::cin.get();
            INFO("开始测试删除功能");
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("数据库异常： {}", e.what());
            return -1;
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis异常： {}", e.what());
            return -1;
        }
        catch (...)
        {
            ERROR("未知异常");
            return -1;
        }
    }

    {  
        try
        {
            //创建数据库事务
            odb::transaction t(curDb->begin());
            auto& dbHandler = t.database();
            
            //创建redis事务
            auto rtx = curRedis->transaction(false, false);
            auto rehandler = rtx.redis();

            //开始创建rolePermission对象
            auto curRolePermission = std::make_shared<suiRolePermission::rolePermission>(dbHandler, rehandler, curRemoveCache);

            //开始删除
            curRolePermission->remove("http://www.baidu.com");
            t.commit();
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("数据库异常： {}", e.what());
            return -1;
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis异常： {}", e.what());
            return -1;
        }
        catch (...)
        {
            ERROR("未知异常");
            return -1;
        }
    }

    INFO("删除成功, 开始查询");

    {
        try
        {
            //创建数据库事务
            odb::transaction t(curDb->begin());
            auto& dbHandler = t.database();
            
            //创建redis事务
            auto rtx = curRedis->transaction(false, false);
            auto rehandler = rtx.redis();

            //开始创建rolePermission对象
            auto curRolePermission = std::make_shared<suiRolePermission::rolePermission>(dbHandler, rehandler, curRemoveCache);

            //开始添加
            auto ret = curRolePermission->getPermissionsByOperation("http://www.baidu.com");
            t.commit();

            if(ret)
            {
                //不存在
                ERROR("查询成功, url存在，删除失败");
                return -1;
            }
            //存在
            INFO("查询成功, url不存在，删除成功");
            INFO("测试rolePermission完成, 点击回车退出");
            std::cin.get();
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("数据库异常： {}", e.what());
            return -1;
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis异常： {}", e.what());
            return -1;
        }
        catch (...)
        {
            ERROR("未知异常");
            return -1;
        }
    }

    
    return 0;
}
