#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include "user_data_statistics.hpp"
#include "user_information_operation.hpp"
#include "user_Identity_role.hpp"
#include "removeCache.hpp"

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


void curInsert(std::shared_ptr<odb::database> curDb, std::shared_ptr<sw::redis::Redis> curRedis
            , suiQueue::MQClient::ptr _mqClienrt, suiRemoveCache::RemoveCache::ptr curRemoveCache
        , const std::string& user_id, const std::string& bindemail, suiDataSql::identityType identity
        , suiDataSql::roleType role)
{
    (void) _mqClienrt;
    try
    {
        //创建事务
        odb::transaction t(curDb->begin());
        auto rtx = curRedis->transaction(false, false);

        //创建操作对象
        auto handle = std::make_shared<suiUserInformation::suiUserInformationOperation>(t, rtx, curRemoveCache);
        //创建用户数据
        suiDataSql::suiUsrMeta::ptr userInfoPtr = std::make_shared<suiDataSql::suiUsrMeta>();
        //进行插入
        handle->insert(user_id, bindemail);
        //创建用户身份操作

        auto& dbHandler = t.database();
        auto rehandler = rtx.redis();
        auto userIdRole = std::make_shared<suiUserIdentityRole::UserIdentityRole>(dbHandler, rehandler, curRemoveCache);
        //添加用户身份信息
        userIdRole->insert(user_id, role, identity);
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
}

//判断邮箱是否注册过了
void judgeEmail(std::shared_ptr<odb::database> curDb, std::shared_ptr<sw::redis::Redis> curRedis
            , suiQueue::MQClient::ptr _mqClienrt, suiRemoveCache::RemoveCache::ptr curRemoveCache
        , const std::string& email)
{
    (void) _mqClienrt;
    try
    {
        //创建事务
        odb::transaction t(curDb->begin());
        auto rtx = curRedis->transaction(false, false);

        //创建操作对象
        auto handle = std::make_shared<suiUserInformation::suiUserInformationOperation>(t, rtx, curRemoveCache);
        if(handle->isExistByEmail(email))
        {
            INFO("邮箱 {} 已经注册过了", email);
        }
        else
        {
            INFO("邮箱 {} 没有注册过", email);
        }
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
}

//获取用户数据
void curGetUserById(std::shared_ptr<odb::database> curDb, std::shared_ptr<sw::redis::Redis> curRedis
            , suiQueue::MQClient::ptr _mqClienrt, suiRemoveCache::RemoveCache::ptr curRemoveCache
        , const std::string& user_id)
{
    (void) _mqClienrt;
    try
    {
        //创建事务
        odb::transaction t(curDb->begin());
        auto rtx = curRedis->transaction(false, false);

        //创建操作对象
        auto handle = std::make_shared<suiUserInformation::suiUserInformationOperation>(t, rtx, curRemoveCache);
        auto ret = handle->getUserInfoById(user_id);
        if(ret)
        {
            INFO("用户存在， 用户id为 {}", ret->getUserId());
            INFO("用户邮箱为 {}", ret->getBindEmail());
            INFO("用户名称为 {}", ret->getUserName());
            if(!ret->getAdministratorName().null())
                INFO("用户的管理名称为 {}", ret->getAdministratorName());
            if(!ret->getPassword().null())
                INFO("用户有设置密码，密码为 {}", ret->getPassword());
            if(!ret->getHeadImageFileId().null())
                INFO("用户有设置自定义头像，头像id为 {}", ret->getHeadImageFileId());
            if(!ret->getUserDescription().null())
                INFO("管理员有给该用户设置自定义描述，描述为 {}", ret->getUserDescription());
            if(ret->getUserStatus() == suiDataSql::userStatus::userStatusEnable)
                INFO("用户状态为正常");
            else 
                INFO("用户被封禁");
        }
        else
        {
            INFO("用户 {} 不存在", user_id);
        }
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
}

//根据页和每页数量获取用户列表
void curGetUserByIdPagination(std::shared_ptr<odb::database> curDb, std::shared_ptr<sw::redis::Redis> curRedis
            , suiQueue::MQClient::ptr _mqClienrt, suiRemoveCache::RemoveCache::ptr curRemoveCache
        , suiDataSql::roleType role, suiDataSql::identityType type, int page, int pageSize)
{
    (void) _mqClienrt;
    try
    {
        //创建事务
        odb::transaction t(curDb->begin());
        auto rtx = curRedis->transaction(false, false);

        //创建操作对象
        auto handle = std::make_shared<suiUserInformation::suiUserInformationOperation>(t, rtx, curRemoveCache);
        auto it = handle->getUserInfoListByTypeAndStatus(role, type, page, pageSize);
        for(auto ret : it->list)
        {
            INFO("用户存在， 用户id为 {}", ret.getUserId());
            INFO("用户邮箱为 {}", ret.getBindEmail());
            INFO("用户名称为 {}", ret.getUserName());
            if(!ret.getAdministratorName().null())
                INFO("用户的管理名称为 {}", ret.getAdministratorName());
            if(!ret.getPassword().null())
                INFO("用户有设置密码，密码为 {}", ret.getPassword());
            if(!ret.getHeadImageFileId().null())
                INFO("用户有设置自定义头像，头像id为 {}", ret.getHeadImageFileId());
            if(!ret.getUserDescription().null())
                INFO("管理员有给该用户设置自定义描述，描述为 {}", ret.getUserDescription());
            if(ret.getUserStatus() == suiDataSql::userStatus::userStatusEnable)
                INFO("用户状态为正常");
            else 
                INFO("用户被封禁");
        }
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
}

void updateUserInfo(std::shared_ptr<odb::database> curDb, std::shared_ptr<sw::redis::Redis> curRedis
            , suiQueue::MQClient::ptr _mqClienrt, suiRemoveCache::RemoveCache::ptr curRemoveCache
        , const std::string& user_id)
{
    (void) _mqClienrt;
    try
    {
        //创建事务
        odb::transaction t(curDb->begin());
        auto rtx = curRedis->transaction(false, false);

        //创建操作对象
        auto handle = std::make_shared<suiUserInformation::suiUserInformationOperation>(t, rtx, curRemoveCache);
        auto ret = handle->getUserInfoById(user_id);
        if(!ret)
        {
            INFO("用户 {} 不存在", user_id);
            return;
        }
        //统一修改成一个东西
        ret->setUserName("newUserName");
        ret->setAdministratorName(odb::nullable<std::string>("newAdministratorName"));
        ret->setHeadImageFileId(odb::nullable<std::string>("newHeadImageFileId"));
        ret->setUserDescription(odb::nullable<std::string>("newUserDescription"));
        handle->updateUser(ret);

        //密码统一修改成1111
        handle->setUserPassward(user_id, "1111");
        //统一设置头像路径为222222
        handle->setAvatar(user_id, "222222");
        //提交事务
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
}

//进行密码验证
void authenticationByPassword(std::shared_ptr<odb::database> curDb, std::shared_ptr<sw::redis::Redis> curRedis
            , suiQueue::MQClient::ptr _mqClienrt, suiRemoveCache::RemoveCache::ptr curRemoveCache
        , const std::string& user_id, const std::string& password)
{
    (void) _mqClienrt;
    try
    {
        //创建事务
        odb::transaction t(curDb->begin());
        auto rtx = curRedis->transaction(false, false);

        //创建操作对象
        auto handle = std::make_shared<suiUserInformation::suiUserInformationOperation>(t, rtx, curRemoveCache);
        if(!handle->authenticationByPassword(user_id, password))
        {
            //验证失败
            INFO("用户 {} 密码验证失败", user_id);
        }
        else
        {
            INFO("用户 {} 密码验证成功", user_id);
        }
        //提交事务
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
}

//注销账号
void deleteUserAccount(std::shared_ptr<odb::database> curDb, std::shared_ptr<sw::redis::Redis> curRedis
            , suiQueue::MQClient::ptr _mqClienrt, suiRemoveCache::RemoveCache::ptr curRemoveCache
        , const std::string& user_id)
{
    (void) _mqClienrt;
    try
    {
        //创建事务
        odb::transaction t(curDb->begin());
        auto rtx = curRedis->transaction(false, false);

        {
            //删除用户元信息
            //创建操作对象
            auto handle = std::make_shared<suiUserInformation::suiUserInformationOperation>(t, rtx, curRemoveCache);
            handle->deleteUser(user_id);
        }

        {
            //删除用户身份信息表
            //创建操作对象
            auto& db = t.database();
            auto redis = rtx.redis();
            auto handle = std::make_shared<suiUserIdentityRole::UserIdentityRole>(db, redis, curRemoveCache);
            handle->remove(user_id);
        }
        //提交事务
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
}

void selectusers(std::shared_ptr<odb::database> curDb, std::shared_ptr<sw::redis::Redis> curRedis
            , suiQueue::MQClient::ptr _mqClienrt, suiRemoveCache::RemoveCache::ptr curRemoveCache
        , const std::string& username)
{
    try
    {
        //创建事务
        odb::transaction t(curDb->begin());
        auto rtx = curRedis->transaction(false, false);

        {
            //删除用户元信息
            //创建操作对象
            auto handle = std::make_shared<suiUserInformation::suiUserInformationOperation>(t, rtx, curRemoveCache);
            auto ret = handle->getUserInfoListByUsername(username);
            INFO("用户一共有： {} 个", ret->total);
            for(auto& it : ret->list)
            {
                INFO("用户id: {} ", it.getUserId());
            }
        }
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

    //缓存删除队列
    suiRemoveCache::RemoveCache::ptr curRemoveCache = suiRemoveCache::RemoveCacheFactory::create(curRedis, _mqClienrt);
    
    INFO("开始进行数据添加测试");
    {
        //添加身份为四个普通用户，两个管理用户
        //    角色四个普通角色，两个管理员角色
        curInsert(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1001", "alice@test.com", suiDataSql::identityType::identityTypeNormal, suiDataSql::roleType::roleTypeNormal);
        curInsert(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1002", "bob@test.com", suiDataSql::identityType::identityTypeNormal, suiDataSql::roleType::roleTypeNormal);
        curInsert(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1003", "charlie@test.com", suiDataSql::identityType::identityTypeNormal, suiDataSql::roleType::roleTypeNormal);
        curInsert(curDb, curRedis, _mqClienrt, curRemoveCache, "u_2001", "admin_d@test.com", suiDataSql::identityType::identityTypeAdmin, suiDataSql::roleType::roleTypeAdmin);
        curInsert(curDb, curRedis, _mqClienrt, curRemoveCache, "u_2002", "admin_e@test.com", suiDataSql::identityType::identityTypeAdmin, suiDataSql::roleType::roleTypeAdmin);
        curInsert(curDb, curRedis, _mqClienrt, curRemoveCache, "u_9999", "delete_me@test.com", suiDataSql::identityType::identityTypeNormal, suiDataSql::roleType::roleTypeNormal);
    }
    INFO("数据添加测试结束，点击回车开始下一步");
    std::cin.get();
    INFO("开始进行多数据查询");
    {
        //查询所有用户
        selectusers(curDb, curRedis, _mqClienrt, curRemoveCache, "新用户");
    }

    INFO("多数据查询结束，点击回车开始下一步");
    std::cin.get();
    INFO("开始进行数据获取判断测试");
    {
        //判断邮箱是否注册过了
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "alice@test.com");
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "bob@test.com");
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "charlie@test.com");
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "admin_d@test.com");
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "admin_e@test.com");
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "delete_me@test.com");
        //六个注册过的，六个没注册过的
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "alice1@test.com");
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "bob1@test.com");
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "charlie1@test.com");
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "admin_d1@test.com");
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "admin_e1@test.com");
        judgeEmail(curDb, curRedis, _mqClienrt, curRemoveCache, "delete_m1e@test.com");
    }
    INFO("邮箱注册测试测试完毕，点击回车开始下一步");
    std::cin.get();
    INFO("开始测试数据获取接口");
    {
        //通过用户id获取用户信息
        INFO("开始获取单个角色信息");
        curGetUserById(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1001");
        curGetUserById(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1002");
        curGetUserById(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1003");
        curGetUserById(curDb, curRedis, _mqClienrt, curRemoveCache, "u_2001");
        curGetUserById(curDb, curRedis, _mqClienrt, curRemoveCache, "u_2002");
        curGetUserById(curDb, curRedis, _mqClienrt, curRemoveCache, "u_9999");
        curGetUserById(curDb, curRedis, _mqClienrt, curRemoveCache, "u_99299");
        curGetUserById(curDb, curRedis, _mqClienrt, curRemoveCache, "u_9199");

        
        //测试连续获取用户信息
        INFO("开始获取连续用户信息");
        curGetUserByIdPagination(curDb, curRedis, _mqClienrt, curRemoveCache, suiDataSql::roleType::roleTypeNormal, suiDataSql::identityType::identityTypeNormal, 0, 10);
        curGetUserByIdPagination(curDb, curRedis, _mqClienrt, curRemoveCache, suiDataSql::roleType::roleTypeNormal, suiDataSql::identityType::identityTypeNormal, 1, 1);
        INFO("连续获取管理员信息") ;
        curGetUserByIdPagination(curDb, curRedis, _mqClienrt, curRemoveCache, suiDataSql::roleType::roleTypeNormal, suiDataSql::identityType::identityTypeNormal, 0, 10);
        //此时数据都会添加到缓存中
    }

    INFO("数据获取测试完毕，点击回车开始下一步");
    std::cin.get();
    INFO("开始修改用户信息");

    {
        //全部修改成统一样式
        updateUserInfo(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1001");
        updateUserInfo(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1002");
        updateUserInfo(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1003");
        updateUserInfo(curDb, curRedis, _mqClienrt, curRemoveCache, "u_2001");
        updateUserInfo(curDb, curRedis, _mqClienrt, curRemoveCache, "u_2002");
        updateUserInfo(curDb, curRedis, _mqClienrt, curRemoveCache, "u_9999");
    }

    INFO("用户信息修改完成，点击回车开始下一步");
    std::cin.get();
    INFO("开始进行密码验证");
    {   
        //验证密码
        authenticationByPassword(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1003", "123456");
        authenticationByPassword(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1003", "1111");
    }
    INFO("密码验证成功，点击回车开始下一步");
    std::cin.get();
    INFO("开始注销账号");

    {
        deleteUserAccount(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1001");
        deleteUserAccount(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1002");
        deleteUserAccount(curDb, curRedis, _mqClienrt, curRemoveCache, "u_1003");
        deleteUserAccount(curDb, curRedis, _mqClienrt, curRemoveCache, "u_2001");
        deleteUserAccount(curDb, curRedis, _mqClienrt, curRemoveCache, "u_2002");
        deleteUserAccount(curDb, curRedis, _mqClienrt, curRemoveCache, "u_9999");
    }

    //测试结束
    INFO("测试结束，点击回车退出");
    std::cin.get();
    return 0;
}
