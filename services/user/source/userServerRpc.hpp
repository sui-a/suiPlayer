#pragma once
#include <iostream>
#include "data.hpp"
#include "data-odb.hxx"
#include "base.pb.h"
#include "user.pb.h"
#include "error.proto.hpp"
#include <brpc/closure_guard.h>
#include "userServerData.hpp"

namespace suiUser
{
    class suiUserServerRpc : public suiApi::userServices
    {
    public:
        suiUserServerRpc(suiUserServerData::ptr userMetaOperation);

        ~suiUserServerRpc();

        virtual void tempLogin(::google::protobuf::RpcController* controller,
                       const ::suiApi::tempLoginReq* request,
                       ::suiApi::tempLoginRsp* response,
                       ::google::protobuf::Closure* done) override;

        virtual void sessionLogin(::google::protobuf::RpcController* controller,
                            const ::suiApi::sessionLoginReq* request,
                            ::suiApi::sessionLoginRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void getEmailCode(::google::protobuf::RpcController* controller,
                            const ::suiApi::getEmailCodeReq* request,
                            ::suiApi::getEmailCodeRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void emailCodeLogin(::google::protobuf::RpcController* controller,
                            const ::suiApi::emailNumberLoginReq* request,
                            ::suiApi::emailNumberLoginRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void passwordLogin(::google::protobuf::RpcController* controller,
                            const ::suiApi::passwordLoginReq* request,
                            ::suiApi::passwordLoginRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void logout(::google::protobuf::RpcController* controller,
                            const ::suiApi::logoutReq* request,
                            ::suiApi::logoutRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void setUserAvatar(::google::protobuf::RpcController* controller,
                            const ::suiApi::setUserAvatarReq* request,
                            ::suiApi::setUserAvatarRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void setUserNickname(::google::protobuf::RpcController* controller,
                            const ::suiApi::setUserNicknameReq* request,
                            ::suiApi::setUserNicknameRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void setPassword(::google::protobuf::RpcController* controller,
                            const ::suiApi::setPasswordReq* request,
                            ::suiApi::setPasswordRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void setUserStatus(::google::protobuf::RpcController* controller,
                            const ::suiApi::setUserStatusReq* request,
                            ::suiApi::setUserStatusRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void getUserInfo(::google::protobuf::RpcController* controller,
                            const ::suiApi::userInfoReq* request,
                            ::suiApi::userInfoRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void newFollow(::google::protobuf::RpcController* controller,
                            const ::suiApi::newFollowReq* request,
                            ::suiApi::newFollowRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void DelFollow(::google::protobuf::RpcController* controller,
                            const ::suiApi::DelFollowReq* request,
                            ::suiApi::DelFollowRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void NewAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::NewAdminReq* request,
                            ::suiApi::NewAdminRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void DelAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::DelAdminReq* request,
                            ::suiApi::DelAdminRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void SetAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::SetAdminReq* request,
                            ::suiApi::SetAdminRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void GetAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::GetAdminReq* request,
                            ::suiApi::GetAdminRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void GetAdminList(::google::protobuf::RpcController* controller,
                            const ::suiApi::GetAdminListReq* request,
                            ::suiApi::GetAdminListRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void SetSalt(::google::protobuf::RpcController* controller,
                       const ::suiApi::SetSaltReq* request,
                       ::suiApi::SetSaltRsp* response,
                       ::google::protobuf::Closure* done) override;

        virtual void GetSalt(::google::protobuf::RpcController* controller,
                            const ::suiApi::GetSaltReq* request,
                            ::suiApi::GetSaltRsp* response,
                            ::google::protobuf::Closure* done) override;

    private:
        
    private:
        suiUserServerData::ptr _userMetaOperation;
        
    };








}





