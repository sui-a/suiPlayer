#include "userServerRpc.hpp"

namespace suiUser
{
    suiUserServerRpc::suiUserServerRpc(suiUserServerData::ptr userMetaOperation)
        : _userMetaOperation(userMetaOperation)
    {

    }

    suiUserServerRpc::~suiUserServerRpc()
    {

    }

    //临时会话申请接口
    void suiUserServerRpc::tempLogin(::google::protobuf::RpcController* controller,
                       const ::suiApi::tempLoginReq* request,
                       ::suiApi::tempLoginRsp* response,
                       ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->tempLogin(error_code, error_msg, *response->mutable_result());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //会话验证接口
    void suiUserServerRpc::sessionLogin(::google::protobuf::RpcController* controller,
                            const ::suiApi::sessionLoginReq* request,
                            ::suiApi::sessionLoginRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->sessionLogin(error_code, error_msg, request->sessionid(), *response->mutable_result());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //申请验证码
    void suiUserServerRpc::getEmailCode(::google::protobuf::RpcController* controller,
                                const ::suiApi::getEmailCodeReq* request,
                                ::suiApi::getEmailCodeRsp* response,
                                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        response->set_id(request->id());
        (void)controller;
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->addVerifyCode(error_code, error_msg, request->sessionid(), request->emailnumber(), *response->mutable_result());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //邮箱验证码登录
    void suiUserServerRpc::emailCodeLogin(::google::protobuf::RpcController* controller,
                            const ::suiApi::emailNumberLoginReq* request,
                            ::suiApi::emailNumberLoginRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        _userMetaOperation->verifyCodeLogin(*request, *response);
    }

    //
    void suiUserServerRpc::passwordLogin(::google::protobuf::RpcController* controller,
                            const ::suiApi::passwordLoginReq* request,
                            ::suiApi::passwordLoginRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->emailPasswordLogin(request->sessionid(), request->emailnumber(), request->password(), *response->mutable_result(), error_code, error_msg);
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //注销登录
    void suiUserServerRpc::logout(::google::protobuf::RpcController* controller,
                            const ::suiApi::logoutReq* request,
                            ::suiApi::logoutRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->logout(error_code, error_msg, request->sessionid());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //设置头像
    void suiUserServerRpc::setUserAvatar(::google::protobuf::RpcController* controller,
                            const ::suiApi::setUserAvatarReq* request,
                            ::suiApi::setUserAvatarRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->setUserAvatar(error_code, error_msg, request->sessionid(), request->fileid());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //设置用户名称
    void suiUserServerRpc::setUserNickname(::google::protobuf::RpcController* controller,
                            const ::suiApi::setUserNicknameReq* request,
                            ::suiApi::setUserNicknameRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->setUserName(error_code, error_msg, request->sessionid(), request->nickname());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //重置密码
    void suiUserServerRpc::setPassword(::google::protobuf::RpcController* controller,
                            const ::suiApi::setPasswordReq* request,
                            ::suiApi::setPasswordRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->setPassWord(error_code, error_msg, request->sessionid(), request->password());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //修改用户状态
    void suiUserServerRpc::setUserStatus(::google::protobuf::RpcController* controller,
                            const ::suiApi::setUserStatusReq* request,
                            ::suiApi::setUserStatusRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->setUserStatus(error_code, error_msg, request->sessionid(), request->targetuserid(), request->userstatus());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }


    //获取用户基本信息
    void suiUserServerRpc::getUserInfo(::google::protobuf::RpcController* controller,
                            const ::suiApi::userInfoReq* request,
                            ::suiApi::userInfoRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        INFO("开始获取用户{}的信息", request->userid());
        _userMetaOperation->getUserInfo(error_code, error_msg, request->sessionid(), request->userid(), *response->mutable_result());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //新增关注
    void suiUserServerRpc::newFollow(::google::protobuf::RpcController* controller,
                            const ::suiApi::newFollowReq* request,
                            ::suiApi::newFollowRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->addFollowing(error_code, error_msg, request->sessionid(), request->targetuserid());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //取消关注
    void suiUserServerRpc::DelFollow(::google::protobuf::RpcController* controller,
                            const ::suiApi::DelFollowReq* request,
                            ::suiApi::DelFollowRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->removeFollowing(error_code, error_msg, request->sessionid(), request->targetuserid());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }


    //新增管理员
    void suiUserServerRpc::NewAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::NewAdminReq* request,
                            ::suiApi::NewAdminRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->addAdmin(error_code, error_msg, request->sessionid(), request->targetuserid());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //删除管理员    
    void suiUserServerRpc::DelAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::DelAdminReq* request,
                            ::suiApi::DelAdminRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->removeAdmin(error_code, error_msg, request->sessionid(), request->targetuserid());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }


    //修改管理员信息
    void suiUserServerRpc::SetAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::SetAdminReq* request,
                            ::suiApi::SetAdminRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->editAdmin(error_code, error_msg, request->sessionid(), request->userinfo());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //通过邮箱获取管理员信息
    void suiUserServerRpc::GetAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::GetAdminReq* request,
                            ::suiApi::GetAdminRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->getAdminByEmail(error_code, error_msg, request->sessionid(), request->email(), *(response->mutable_result()->mutable_userinfo()));
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //获取管理员列表
    void suiUserServerRpc::GetAdminList(::google::protobuf::RpcController* controller,
                            const ::suiApi::GetAdminListReq* request,
                            ::suiApi::GetAdminListRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->getAminList(error_code, error_msg, request->sessionid(), request->role(), request->pageindex(), request->pagecount(), *response->mutable_result());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }

    //设置加密信息
    void suiUserServerRpc::SetSalt(::google::protobuf::RpcController* controller,
                       const ::suiApi::SetSaltReq* request,
                       ::suiApi::SetSaltRsp* response,
                       ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->setSalt(error_code, error_msg, request->sessionid(), request->salt());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
        
    }

    //获取加密信息
    void suiUserServerRpc::GetSalt(::google::protobuf::RpcController* controller,
                            const ::suiApi::GetSaltReq* request,
                            ::suiApi::GetSaltRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        int32_t error_code;
        std::string error_msg;
        _userMetaOperation->getSalt(error_code, error_msg, request->sessionid(), *response->mutable_result());
        response->set_errorcode(error_code);
        response->set_errormsg(error_msg);
    }
}