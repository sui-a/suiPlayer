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

//etcd地址
DEFINE_string(registry_center, "127.0.0.1:8084", "注册中心地址");
DEFINE_string(user_server_name, "user_server", "user_server服务名称");

//注册临时会话
std::string getTemporarySessionId(suiRpc::Channels::Ptr _channels)
{
    //获取服务地址
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象
    suiApi::userServices_Stub stub(&(*curChannel));
    //实例化控制对象
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::tempLoginReq req;
    suiApi::tempLoginRsp rsp;
    req.set_id("1");
    //调用服务
    stub.tempLogin(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        return "";
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("临时会话申请失败, 错误为: {}", rsp.errormsg());
        return "";
    }
    return rsp.result().id();
}

bool sessionLogin(suiRpc::Channels::Ptr _channels, const std::string& sessionId)
{
    //获取服务地址
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::sessionLoginReq req;
    suiApi::sessionLoginRsp rsp;
    req.set_id("2");
    req.set_sessionid(sessionId);
    //调用服务
    stub.sessionLogin(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        return false;
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("临时会话申请失败, 错误为: {}", rsp.errormsg());
        return false;
    }
    return true;
}

//申请验证码
std::string getEmailCode(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& email)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待

    //定义调用参数
    suiApi::getEmailCodeReq req;
    suiApi::getEmailCodeRsp rsp;
    req.set_id("3");
    req.set_sessionid(sessionId);
    req.set_emailnumber(email);
    //调用服务
    stub.getEmailCode(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        return "";
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("临时会话申请失败, 错误为: {}", rsp.errormsg());
        return "";
    }
    return rsp.result().codeid();
}

std::string LoginByCode(suiRpc::Channels::Ptr _channels
    , const std::string& sessionId, const std::string& codeid, const std::string& code
    , const std::string& email)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::emailNumberLoginReq req;
    suiApi::emailNumberLoginRsp rsp;
    req.set_id("4");
    req.set_sessionid(sessionId);
    req.set_codeid(codeid);
    req.set_verifycode(code);
    req.set_emailnumber(email);
    //调用服务
    stub.emailCodeLogin(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        return "";
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("邮箱号码登录失败, 错误为: {}", rsp.errormsg());
        return "";
    }
    return rsp.result().userid();
}

//注销登录
bool logout(suiRpc::Channels::Ptr _channels, const std::string& sessionId)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::logoutReq req;
    suiApi::logoutRsp rsp;
    req.set_id("5");
    req.set_sessionid(sessionId);
    //调用服务
    stub.logout(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        return false;
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("注销登录失败, 错误为: {}", rsp.errormsg());
        return false;
    }
    return true;
}

//设置盐
void setSalt(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& salt)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::SetSaltReq req;
    suiApi::SetSaltRsp rsp;
    req.set_id("6");
    req.set_sessionid(sessionId);
    req.set_salt(salt);
    //调用服务
    stub.SetSalt(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("设置盐失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("设置盐成功");
    return;
}

//获取盐
void getSalt(suiRpc::Channels::Ptr _channels, const std::string& sessionId)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::GetSaltReq req;
    suiApi::GetSaltRsp rsp;
    req.set_id("7");
    req.set_sessionid(sessionId);
    //调用服务
    stub.GetSalt(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("获取盐失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("获取盐成功, 盐为: {}", rsp.result().salt());
    return;
}

//设置用户名
void setUserName(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& userName)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::setUserNicknameReq req;
    suiApi::setUserNicknameRsp rsp;
    req.set_id("8");
    req.set_sessionid(sessionId);
    req.set_nickname(userName);
    //调用服务
    stub.setUserNickname(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("设置用户名失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("设置用户名成功");
    return;
}

//设置密码
void setPassword(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& password)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::setPasswordReq req;
    suiApi::setPasswordRsp rsp;
    req.set_id("8");
    req.set_sessionid(sessionId);
    req.set_password(password);
    //调用服务
    stub.setPassword(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("设置密码失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("设置密码成功");
    return;
}

std::string LoginByPassword(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& email, const std::string& password)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::passwordLoginReq req;
    suiApi::passwordLoginRsp rsp;
    req.set_id("9");
    req.set_sessionid(sessionId);
    req.set_emailnumber(email);
    req.set_password(password);
    //调用服务
    stub.passwordLogin(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("邮箱密码登录失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("邮箱密码登录成功, 用户ID为: {}", rsp.result().userid());
    return rsp.result().userid();
}

//角色类型翻译
std::string roleTypeTransformation(suiApi::roleType roleType)
{
    switch(roleType)
    {
        case suiApi::roleType::roleUserNormal:
            return "普通用户";
        case suiApi::roleType::roleAdminNormal:
            return "普通管理员";
        case suiApi::roleType::roleAdminSuper:
            return "超级管理员";
        default:
            return "未知角色类型";
    }
}
//身份类型翻译
std::string identityTypeTransformation(suiApi::identityType identityType)
{
    switch(identityType)
    {
        case suiApi::identityType::identityNormal:
            return "普通用户";
        case suiApi::identityType::identityAdmin:
            return "B端用户";
        default:
            return "未知身份类型";
    }
}

//获取某用户信息
void getUserInfo(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& targetuserid)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::userInfoReq req;
    suiApi::userInfoRsp rsp;
    req.set_id("10");
    req.set_sessionid(sessionId);
    req.set_userid(targetuserid);
    //调用服务
    stub.getUserInfo(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("获取用户信息失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("获取用户信息成功");
    auto userInfo = rsp.result().userinfo();
    INFO("用户id: {}", userInfo.userid());
    INFO("用户名: {}", userInfo.nickname());
    INFO("绑定的邮箱: {}", userInfo.email());
    INFO("用户点赞数量: {}", userInfo.likecount());
    INFO("播放数量: {}", userInfo.playcount());
    INFO("用户关注数量: {}", userInfo.followcount());
    INFO("用户粉丝数量: {}", userInfo.funscount());
    INFO("用户备注信息： {}", userInfo.usermemo());
    INFO("用户创建时间: {}", userInfo.userctime());
    INFO("本用户是否关注目标用户： {}", (userInfo.isfollowing() == suiApi::followStatus::followStatusTrue ? "是" : "否"));
    INFO("用户角色类型： {}", roleTypeTransformation(userInfo.role()));
    INFO("用户身份类型： {}", identityTypeTransformation(userInfo.identify()));
    INFO("用户状态： {}", (userInfo.status() == suiApi::userStatus::userStatusEnable ? "正常" : "禁用"));
    return;
}

//修改头像
void setUserHand(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& head_id)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::setUserAvatarReq req;
    suiApi::setUserAvatarRsp rsp;
    req.set_id("9");
    req.set_sessionid(sessionId);
    req.set_fileid(head_id);
    //调用服务
    stub.setUserAvatar(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("获取用户信息失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("修改头像成功");
    return;
}

//关注用户
void followUser(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& targetuserid)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::newFollowReq req;
    suiApi::newFollowRsp rsp;
    req.set_id("12");
    req.set_sessionid(sessionId);
    req.set_targetuserid(targetuserid);
    //调用服务
    stub.newFollow(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("关注用户失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("关注用户成功");
    return;
}

//取消关注
void notfollowUser(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& targetuserid)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::DelFollowReq req;
    suiApi::DelFollowRsp rsp;
    req.set_id("13");
    req.set_sessionid(sessionId);
    req.set_targetuserid(targetuserid);
    //调用服务
    stub.DelFollow(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("取消关注用户失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("取消关注用户成功");
    return;
}

//新增管理员
void newAdmin(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& targetuserid)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::NewAdminReq req;
    suiApi::NewAdminRsp rsp;
    req.set_id("14");
    req.set_sessionid(sessionId);
    req.set_targetuserid(targetuserid);
    //调用服务
    stub.NewAdmin(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("新增管理员失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("新增管理员成功");
    return;
}

//获取管理员列表
std::string getAdmiInfoList(suiRpc::Channels::Ptr _channels, const std::string& sessionId)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::GetAdminListReq req;
    suiApi::GetAdminListRsp rsp;
    req.set_id("15");
    req.set_sessionid(sessionId);
    req.set_pageindex(0);
    req.set_pagecount(10);
    req.set_role(suiApi::roleType::roleAdminNormal); //获取普通管理员
    //调用服务
    stub.GetAdminList(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("获取管理员列表失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    const auto& result = rsp.result();

    // 1. 获取数组大小：使用 userlist_size()
    INFO("获取管理员列表成功, 一共{}个普通管理员, 当前获取到{}位", result.totalcount(), result.userlist_size());

    // 2. 边界检查：防止数组为空导致下标越界崩溃
    if (result.userlist_size() == 0) 
    {
        ERROR("管理员列表为空！");
        return "";
    }
    return result.userlist(0).email();
}

//通过邮箱获取管理员信息
suiApi::AdminInfo getAdminInfo(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& email)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::GetAdminReq req;
    suiApi::GetAdminRsp rsp;
    req.set_id("16");
    req.set_sessionid(sessionId);
    req.set_email(email);
    //调用服务
    stub.GetAdmin(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("获取管理员信息失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("获取管理员信息成功");
    const auto& resule = rsp.result().userinfo();
    INFO("管理员id: {}", resule.userid());
    INFO("管理员用户名: {}", resule.nickname());
    INFO("管理员角色{}普通管理员", (resule.role() == suiApi::roleType::roleAdminNormal ? "是" : "否"));
    INFO("管理员身份{}B端用户", (resule.identify() == suiApi::identityType::identityAdmin ? "是" : "否"));
    INFO("管理员状态{}已启用", (resule.status() == suiApi::userStatus::userStatusEnable ? "是" : "否"));
    INFO("管理员备注信息： {}", resule.usermemo());
    INFO("管理员邮箱: {}", resule.email());
    return resule;
}

void setAdminInfo(suiRpc::Channels::Ptr _channels, const std::string& sessionId, suiApi::AdminInfo adminInfo)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::SetAdminReq req;
    suiApi::SetAdminRsp rsp;
    req.set_id("17");
    req.set_sessionid(sessionId);
    INFO("修改的目标id:{}", adminInfo.userid());
    auto setAdmin = req.mutable_userinfo();
    setAdmin->set_userid(adminInfo.userid());
    setAdmin->set_nickname("测试名称");
    setAdmin->set_role(adminInfo.role());
    setAdmin->set_identify(adminInfo.identify());
    setAdmin->set_status(adminInfo.status());
    setAdmin->set_usermemo("这是一个测试备注");
    setAdmin->set_email(adminInfo.email());
    //调用服务
    stub.SetAdmin(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("设置管理员信息失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("设置管理员信息成功");
}

//删除普通管理员
void delNormalAdmin(suiRpc::Channels::Ptr _channels, const std::string& sessionId, const std::string& targetuserid)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::DelAdminReq req;
    suiApi::DelAdminRsp rsp;
    req.set_id("18");
    req.set_sessionid(sessionId);
    req.set_targetuserid(targetuserid);
    //调用服务
    stub.DelAdmin(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("删除管理员失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("删除管理员成功");
    return;
}

//设置某个用户状态
void setStatus(suiRpc::Channels::Ptr _channels, const std::string& sessionId
    , const std::string& targetuserid, bool isEnable)
{
    //获取rpc通道
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::userServices_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::setUserStatusReq req;
    suiApi::setUserStatusRsp rsp;
    req.set_id("19");
    req.set_sessionid(sessionId);
    req.set_targetuserid(targetuserid);
    req.set_userstatus(isEnable ? suiApi::userStatus::userStatusEnable : suiApi::userStatus::userStatusDisable);
    //调用服务
    stub.setUserStatus(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        exit(0);
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("设置用户状态失败, 错误为: {}", rsp.errormsg());
        exit(0);
    }
    INFO("设置用户状态成功");
    delete cntl;
    return;
}

int main(int argc, char* argv[])
{
    //初始化日志模块
    suiUtil::suiLogInitDefault();
    //初始化gflags
    google::ParseCommandLineFlags(&argc, &argv, true);
    
    //创建搜索服务
    suiRpc::Channels::Ptr _channels = std::make_shared<suiRpc::Channels>(FLAGS_user_server_name);
    suiEtcd::serSearch::ptr _search = std::make_shared<suiEtcd::serSearch>("", FLAGS_registry_center
        , [_channels](std::string serName, std::string addr){
        _channels->insert(addr);
        INFO("添加服务： {} 的节点: {}", serName, addr);
    }, [_channels](std::string serName, std::string addr){
        INFO("删除服务： {} 的节点: {}", serName, addr);
        _channels->remove(addr);
    });
    //开始服务发现
    _search->search();
    INFO("点击回车，开始测试");
    std::cin.get();
    INFO("开始注册临时会话与会话登录");
    std::string sessionId;
    {
        //注册临时会话
        sessionId = getTemporarySessionId(_channels);
        if(sessionId.empty())
        {
            ERROR("临时会话注册失败");
            return -1;
        }
        INFO("临时会话注册成功，sessionId为: {}", sessionId);
    }
    INFO("按下回车开始下一步");
    //std::cin.get();
    INFO("开始验证码注册登录");
    std::string user_id;
    {
        auto codeid = getEmailCode(_channels, sessionId, "1953114602@qq.com");
        if(codeid.empty())
        {
            ERROR("验证码申请失败");
            return -1;
        }
        INFO("验证码申请成功, 开始获取验证码");
        std::string code;
        std::cin >> code;
        std::cin.ignore(); //清空缓冲区
        //获取到验证码以后，开始登录会话
        user_id = LoginByCode(_channels, sessionId, codeid, code, "1953114602@qq.com");
        if(user_id.empty())
        {
            ERROR("邮箱号码登录失败");
            return -1;
        }
        INFO("邮箱号码登录成功，用户id为: {}", user_id);
    }
    INFO("按下回车开始下一步");
    //std::cin.get();
    INFO("开始自定义密码");

    {
        //设置盐
        setSalt(_channels, sessionId, "123456");
        //获取盐
        getSalt(_channels, sessionId);
        //修改用户名
        setUserName(_channels, sessionId, "穗");
        //修改密码
        setPassword(_channels, sessionId, "123456");
        //获取管理员信息
        getUserInfo(_channels, sessionId, "111111");
        //修改头像
        setUserHand(_channels, sessionId, "123456");
    }

    INFO("自定义密码设置完成，点击回车继续");
    //std::cin.get();
    INFO("开始注销登录");
    {
        auto logoutret = logout(_channels, sessionId);
        if(logoutret)
        {
            INFO("注销登录成功");
        }
        else
        {
            ERROR("注销登录失败");
        }
    }
    INFO("注销登录完成， 点击回车继续");
    //std::cin.get();
    INFO("开始邮箱密码登录");
    {
        LoginByPassword(_channels, sessionId, "1953114602@qq.com", "123456");
    }
    INFO("邮箱密码登录完成， 点击回车继续");
    //std::cin.get();
    INFO("再次注销登录");
    {
        logout(_channels, sessionId);
    }
    INFO("注销登录成功, 点击回车继续");
    //std::cin.get();
    INFO("开始尝试登录管理员账号");
    std::string admin_user_id;
    {
        admin_user_id = LoginByPassword(_channels, sessionId, "111111", "suisuipingan");
    }

    INFO("管理员账号登录完成， 点击回车继续");
    //std::cin.get();
    INFO("开始获取某个用户信息");
    {
        getUserInfo(_channels, sessionId, admin_user_id);
    }
    INFO("获取某个用户信息完毕");
    //std::cin.get();
    INFO("管理员用户关注原用户");
    {
        followUser(_channels, sessionId, admin_user_id);
    }
    INFO("关注完成，点击回车继续吧");
    //std::cin.get();
    INFO("开始取消关注");
    {
        notfollowUser(_channels, sessionId, admin_user_id);
    }
    INFO("管理员用户取消关注原用户, 点击回车继续");
    //std::cin.get();
    INFO("开始新增普通管理员");
    {
        newAdmin(_channels, sessionId, user_id);
    }
    INFO("新增普通管理员成功，点击回车继续");
    //std::cin.get();
    std::string normalAdminEmail;
    INFO("获取管理员列表");
    {
        normalAdminEmail = getAdmiInfoList(_channels, sessionId);
    }
    INFO("获取管理员列表成功，点击回车继续");
    //std::cin.get();
    suiApi::AdminInfo adminInfo;
    INFO("获取管理员信息");
    {
        adminInfo = getAdminInfo(_channels, sessionId, normalAdminEmail);
    }
    INFO("获取普通管理员信息完成，点击回车继续");
    //std::cin.get();
    INFO("编辑管理员信息");
    {
        setAdminInfo(_channels, sessionId, adminInfo);
    }
    INFO("编辑成功，点击回车继续");
    //std::cin.get();
    INFO("再次获取管理员信息");
    {
        getAdminInfo(_channels, sessionId, normalAdminEmail);
    }
    INFO("获取完成，点击回车继续");
    //std::cin.get();
    INFO("删除普通管理员");
    {
        delNormalAdmin(_channels, sessionId, admin_user_id);
    }
    INFO("删除普通管理员成功，点击回车继续");
    //std::cin.get();
    INFO("再次获取管理员列表");
    {
        normalAdminEmail = getAdmiInfoList(_channels, sessionId);
    }
    INFO("获取管理员列表成功，点击回车继续");
    //std::cin.get();
    INFO("以管理员身份设置某用户的状态");
    {
        setStatus(_channels, sessionId, admin_user_id, false);
    }
    INFO("设置某用户状态完成，点击回车继续");
    //std::cin.get();
    INFO("注销登录");
    {
        auto logoutret = logout(_channels, sessionId);
        if(logoutret)
        {
            INFO("注销登录成功");
        }
        else
        {
            ERROR("注销登录失败");
        }
    }
    INFO("注销登录成功, 点击回车继续");
    //std::cin.get();
    INFO("用户服务客户端测试完成，点击回车退出");
    std::cin.get();
    return 0;
}
