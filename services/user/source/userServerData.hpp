#pragma once
#include <optional>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiRandom.hpp>
#include <suiScaffold/suiHash.hpp>
#include <suiScaffold/suiMail.hpp>
#include <suiScaffold/suiFastdfs.hpp>
#include "data.hpp"
#include "data-odb.hxx"
#include "base.pb.h"
#include "user.pb.h"
#include "file.hpp"
#include "session.hpp"
#include "verify_code.hpp"
#include "saltOperation.hpp"
#include "user_follow.hpp"
#include "user_data_statistics.hpp"
#include "user_Identity_role.hpp"
#include "user_information_operation.hpp"
#include "cache_sync.hpp"
#include "optionalaAuthority.hpp"
#include "error.proto.hpp"

namespace suiUser
{
    //业务集合
    class suiUserServerData
    {
    public:
        using ptr = std::shared_ptr<suiUserServerData>;

        suiUserServerData(const suiOdb::odbSetting &ms,
                const suiRedis::redisSettings &rs, suiQueue::MQClient::ptr mq, suiMail::suiMailClient::ptr mail, suiCacheSync::CacheSyncClient::ptr file_remove_syne);

        ~suiUserServerData();

        //访客登录
        void tempLogin(int32_t& error_code, std::string& error_msg, suiApi::tempLoginResult& result);
        //会话登录
        void sessionLogin(int32_t& error_code, std::string& error_msg, const std::string& ssid, suiApi::sessionLoginResult& result);
        //新增验证码
        void addVerifyCode(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& email, suiApi::getEmailCodeResult& result);
        //验证码登录
        void verifyCodeLogin(const suiApi::emailNumberLoginReq& request, suiApi::emailNumberLoginRsp& rsp);
        //邮箱密码登录
        void emailPasswordLogin(const std::string& ssid, const std::string& email, const std::string& passward, suiApi::passwordLoginResult& result
                            , int32_t& error_code, std::string& error_msg);
        //退出登录
        void logout(int32_t& error_code, std::string& error_msg, const std::string& ssid);
        //设置用户头像
        void setUserAvatar(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& avatar);
        //设置用户名称
        void setUserName(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& newName);
        //设置登录密码
        void setPassWord(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& passward);
        //设置用户状态
        void setUserStatus(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId, suiApi::userStatus newSatus);
        //获取某用户信息
        void getUserInfo(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId, suiApi::userInfoResult& result);
        //新增关注
        void addFollowing(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId);
        //取消关注
        void removeFollowing(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId);
        //新增管理员
        void addAdmin(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId);
        //删除管理员
        void removeAdmin(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& targetUserId);
        //编辑管理员信息
        void editAdmin(int32_t& error_code, std::string& error_msg, const std::string& ssid, const suiApi::AdminInfo& userInfo);
        //通过邮箱获取管理员信息
        void getAdminByEmail(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& email, suiApi::AdminInfo& result);
        //获取管理员列表
        void getAminList(int32_t& error_code, std::string& error_msg, const std::string& ssid, suiApi::roleType role
                                    , int32_t pageIndex, int32_t pageCount, suiApi::GetAdminListResult& result);
        //设置加密信息
        void setSalt(int32_t& error_code, std::string& error_msg, const std::string& ssid, const std::string& salt);
        //获取加密信息
        void getSalt(int32_t& error_code, std::string& error_msg, const std::string& ssid, suiApi::GetSaltResult& result);


    private:
        //创建会话id
        std::string createRandomId();
        //创建验证码
        std::string createVerifyCode();

        //模板更换
        void replaceAll(std::string& str, const std::string& from, const std::string& to);
        //获取邮箱html内容
        std::string getEmailHtmlContent(const std::string& title, const std::string& body, const std::string& targetEmail, const std::string& emailfrom);
            
        //用户状态转换
        ::suiDataSql::userStatus userStatusTransformation(::suiApi::userStatus userStatus);
        //用户身份
        ::suiDataSql::identityType identityTransformation(::suiApi::identityType identity);
        //用户角色类型转换
        ::suiDataSql::roleType RoleTransformation(::suiApi::roleType Role);
        //关注状态
        bool isFollowTransformation(::suiApi::followStatus value);

        //反转
        //用户状态转换
        ::suiApi::userStatus userStatusTransformation(::suiDataSql::userStatus userStatus);
        //用户身份
        ::suiApi::identityType identityTransformation(::suiDataSql::identityType identity);
        //用户角色类型转换
        ::suiApi::roleType RoleTransformation(::suiDataSql::roleType Role);
        //关注状态
        ::suiApi::followStatus isFollowTransformation(bool value);

    private:
        std::shared_ptr<odb::database> _mysql;
        std::shared_ptr<sw::redis::Redis> _redis;
        suiRemoveCache::RemoveCache::ptr _cache_sync;
        //邮箱客户端
        suiMail::suiMailClient::ptr _mail;
        //文件异步删除接口
        suiCacheSync::CacheSyncClient::ptr _file_remove_syne;

        //html更换模板字段
        static const std::string _emailHtmlTemplateFieldTitle;
        static const std::string _emailHtmlTemplateFieldCode;
        static const std::string _emailHtmlTemplateFieldTargetEmail;
        static const std::string _emailHtmlTemplateFieldEmailfrom;
    };

    



}