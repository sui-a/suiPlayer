#pragma once
#include <optional>
#include <memory>
#include <string>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiRandom.hpp>
#include "session.hpp"
#include "verify_code.hpp"
#include "saltOperation.hpp"
#include "user_follow.hpp"
#include "user_data_statistics.hpp"
#include "user_Identity_role.hpp"
#include "user_information_operation.hpp"

namespace suiUser
{
    //业务集合
    class suiUserServerData
    {
    public:
        using ptr = std::shared_ptr<suiUserServerData>;

        suiUserServerData(const suiOdb::odbSetting &ms,
                const suiRedis::redisSettings &rs, suiQueue::MQClient::ptr mq);

        ~suiUserServerData();

        //访客申请会话
        std::string touristLogin();  //这里直接返回生成的session，并存储进session表中

        //验证用户会话是否匹配
        bool verifySession(const std::string& session_id, const std::string& user_id);
        //验证会话是否有效
        bool verifySession(const std::string& session_id, std::string* user_id = nullptr); //此时会话id是访客会话id，还是正式id都无所谓
        //判断用户是否关注
        bool isFollow(const std::string& user_id, const std::string& targetUserId);
        //验证用户是否有效
        bool verifyUser(const std::string& user_id);
        //通过邮箱验证用户是否被禁用
        bool isEnable(const std::string& email);
        //工具id判断当前用户是否有权限执行某个操作
        bool hasPermission(const std::string& user_id, const std::string& operationId);

        //新增验证码
        std::string addVerifyCode(const std::string& session_id, std::string* codeptr = nullptr);
        //验证码获取操作
        std::string getVerifyCode(const std::string& code_id);  //根据会话id获取验证码
        //校验并删除验证码
        bool verifyAndRemoveVerifyCode(const std::string& session_id, const std::string& code_id, const std::string& code);
        //直接删除验证码
        void removeVerifyCode(const std::string& code_id);

        //设置盐
        bool setSalt(const std::string& user_id, const std::string& salt);
        //获取盐
        bool getSalt(const std::string& user_id, std::string& salt);
        //修改/新增盐
        void changeSalt(const std::string& user_id, const std::string& salt);

        //会话登录，返回是否是临时会话
        suiDataSql::SessionStatus sessionLogin(const std::string& session_id, std::string* userId = nullptr);
        //邮箱登录，新增用户信息或者修改会话信息
        bool emailLogin(const std::string& session_id, const std::string& email, std::string* outUserId = nullptr);
        //邮箱密码登录
        bool emailPasswordLogin(const std::string& session_id, const std::string& email, const std::string& password, std::string* outUserId = nullptr);
        //退出登录
        void logout(const std::string& session_id);
        //删除用户id
        void removeSessionId(const std::string& session_id);

        //修改名称
        bool changeName(const std::string& user_id, const std::string& name);
        //修改密码
        bool changePassword(const std::string& user_id, const std::string& password);
        //修改头像
        bool changeAvatar(const std::string& user_id, const std::string& avatarid);
        //修改状态
        bool changeStatus(const std::string& user_id, const suiDataSql::userStatus status);

        //获取用户详细信息
        suiDataSql::suiUsrMeta::ptr getUserInfo(const std::string& user_id);
        suiDataSql::suiUsrMeta::ptr getUserInfoByEmail(const std::string& email);
        //获取用户身份角色信息
        suiDataSql::suiUserIdIdentityRoleMeta::ptr getIdentityRole(const std::string& user_id);
        //获取用户统计信息
        suiDataSql::UserHomepageBasicData::ptr getUserStatistics(const std::string& user_id);
        //新增关注关系
        void addFollow(const std::string& user_id, const std::string& follow_id);
        //删除关注关系
        void removeFollow(const std::string& user_id, const std::string& follow_id);
        //新增管理员 设置角色类型，身份类型切换成B端
        bool addAdmin(const std::string& user_id);
        //删除管理员 角色类型切换成普通角色，身份类型切换成普通用户
        bool removeAdmin(const std::string& user_id);
        //设置管理员属性
        bool setAdminProperty(const std::string& user_id, const std::string& nickname, const std::string& userMemo);

        //获取指定类型的管理员列表 参数：身份类型，角色类型，页数，页大小
        suiDataSql::userInfoList::ptr getAdminList(suiDataSql::userStatus status, const suiDataSql::identityType identity
            , const suiDataSql::roleType role,  const int page, const int page_size);
        
        //判断是否大于等于指定权限
        bool hasPermissionBySessionId(const std::string& session_id, const std::string& user_id
            , const suiDataSql::identityType identity, const suiDataSql::roleType role);

       private:
        //创建会话id
        std::string createRandomId();
        //创建验证码
        std::string createVerifyCode();

        //邮箱不存在操作
        bool _emailLoginNotExist(const std::string& session_id, const std::string& email, std::string* outUserId = nullptr);
        //邮箱存在操作
        bool _emailLoginExist(const std::string& session_id, const std::string& user_id);

    private:
        std::shared_ptr<odb::database> _mysql;
        std::shared_ptr<sw::redis::Redis> _redis;
        suiRemoveCache::RemoveCache::ptr _cache_sync;
    };

    



}