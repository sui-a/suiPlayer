#include "userServerRpc.hpp"

namespace suiUser
{
    const std::string suiUserServerRpc::_emailHtmlTemplateFieldTitle = "{{title}}";
    const std::string suiUserServerRpc::_emailHtmlTemplateFieldCode = "{{ emailVerifyCode }}";
    const std::string suiUserServerRpc::_emailHtmlTemplateFieldTargetEmail = "{{targetEmail}}";
    const std::string suiUserServerRpc::_emailHtmlTemplateFieldEmailfrom = "{{emailfrom}}";

    suiUserServerRpc::suiUserServerRpc(suiUserServerData::ptr userMetaOperation, suiMail::suiMailClient::ptr mail)
        : _userMetaOperation(userMetaOperation)
        , _mail(mail)
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

        //直接返回生成的临时会话id
        auto* result = response->mutable_result();
        auto sessionId = _userMetaOperation->touristLogin();
        if(sessionId.empty())
        {
            //出现错误，直接返回错误值
            response->set_id(request->id());
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID);
            response->set_errormsg("临时会话申请失败");
        }
        else
        {
            response->set_id(request->id());
            response->set_errorcode(suiErrorCodeDef::SUCCESS);
            result->set_id(sessionId);
        }
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

        //验证会话登录
        std::string userId;
        auto sessionid = request->sessionid();
        auto sessionUserId = request->userid();
        if(sessionid.empty())
        {
            //会话id格式不正确
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_ID_INVALID);
            response->set_errormsg("会话id格式错误");
            INFO("会话id格式错误, 会话id: {}", sessionid);
            return;
        }

        auto sessionStatus = _userMetaOperation->sessionLogin(sessionid, &userId);
        auto result = response->mutable_result();
        if(sessionStatus == suiDataSql::SessionStatus::Guest)
        {
            //临时会话
            if(userId.empty() && sessionUserId.empty())
            {
                //正确返回
                response->set_errorcode(suiErrorCodeDef::SUCCESS);
                result->set_isguest(true);
                return;
            }
            else
            {
                //此时状态对不上，注销会话并清除会话
                _userMetaOperation->removeSessionId(sessionid);
                response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID);
                response->set_errormsg("会话验证失败，请重新申请会话");
                return;
            }
        }
        else if(sessionStatus == suiDataSql::SessionStatus::Normal)
        {
            //正常会话
            if(!userId.empty() && !sessionUserId.empty()&& userId == sessionUserId)
            {
                //返回正确
                response->set_errorcode(suiErrorCodeDef::SUCCESS);
                result->set_isguest(false);
                result->set_userid(userId);
                return;
            }
            else
            {
                //会话状态异常
                _userMetaOperation->removeSessionId(sessionid);
                response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID);
                response->set_errormsg("会话失效，请重新申请会话");
                return;
            }
        }
        else
        {
            //会话状态异常
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID);
            response->set_errormsg("会话状态异常，请重新申请会话");
        }
        //
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

        std::string targetEmail = request->emailnumber();
        if(targetEmail.empty())
        {
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_EMAIL_INVALID);
            response->set_errormsg("邮箱格式错误");
            return;
        }
        
        //
        std::string code;
        auto codeId = _userMetaOperation->addVerifyCode(request->sessionid(), &code);
        if(codeId.empty() || code.empty())
        {
            //创建或者获取失败
            INFO("邮箱验证码申请失败，验证码id： {}, 验证码： {}", codeId, code);
            if(!codeId.empty())
                _userMetaOperation->removeVerifyCode(codeId); //获取失败，用户会重新获取，此时直接删除原验证码
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_VERIFY_CODE_MISMATCH);
            response->set_errormsg("验证码申请失败");
            return;
        }
        
        //此时验证码申请和获取成功
        //开始发送验证码到指定邮箱
        auto emailbody = getEmailHtmlContent("登录验证码", code, targetEmail, _mail->getFrom());
        if(emailbody.empty())
        {
            //验证码发送失败
            _userMetaOperation->removeVerifyCode(codeId); //发送失败，删除验证码
            //返回的验证码获取失败错误 //转移责任
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_VERIFY_CODE_GET_FAILED);
            response->set_errormsg("验证信息构建失败");
            return;
        }
        if(!_mail || _mail->send(targetEmail, emailbody) == false)
        {
            //验证码发送失败
            INFO("验证码发送失败，目标邮箱： {}", targetEmail);
            _userMetaOperation->removeVerifyCode(codeId); //发送失败，删除验证码
            //返回的验证码获取失败错误 //转移责任
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_VERIFY_CODE_GET_FAILED);
            response->set_errormsg("验证码发送失败");
            return;
        }

        //发送成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        //记录验证码id
        auto* result = response->mutable_result();
        result->set_codeid(codeId);
        return;
    }

    //邮箱验证码登录
    void suiUserServerRpc::emailCodeLogin(::google::protobuf::RpcController* controller,
                            const ::suiApi::emailNumberLoginReq* request,
                            ::suiApi::emailNumberLoginRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;

        //提取参数
        response->set_id(request->id());
        std::string code = request->verifycode();
        std::string codeId = request->codeid();
        std::string sessionid = request->sessionid();
        std::string targetEmail = request->emailnumber();
        std::string userId;

        //判断是否是临时会话
        auto selogret = _userMetaOperation->sessionLogin(sessionid);
        if(selogret != suiDataSql::SessionStatus::Guest)
        {
            //此时不是临时会话，直接返回false
            INFO("邮箱登录，会话id： {} 不属于临时会话， 状态： {}", sessionid, suiDataSql::suiSessionStatus::toString(selogret));
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_STATUS_INVALID);
            response->set_errormsg("会话状态异常");
            return;
        }
        //判断用户是否被禁用
        auto enableret = _userMetaOperation->isEnable(targetEmail);
        if(!enableret)
        {
            //此时用户被禁用，直接返回false
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_EMAIL_DISABLE);
            response->set_errormsg("用户被禁用");
            return;
        }

        //进行验证码验证，防止外部忘记验证
        if(!_userMetaOperation->verifyAndRemoveVerifyCode(sessionid, codeId, code))
        {
            //此时验证码验证失败，直接返回false
            INFO("邮箱登录，验证码验证失败， 验证码id： {}", codeId);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_VERIFY_CODE_INVALID);
            response->set_errormsg("验证码错误");
            return;
        }

        //直接登录
        bool ret = _userMetaOperation->emailLogin(sessionid, targetEmail, &userId);
        if(!ret)
        {
            //登录失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_EMAIL_LOGIN_FAILED);
            response->set_errormsg("邮箱登录失败");
            return;
        }
        //登录成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        auto result = response->mutable_result();
        result->set_userid(userId);
        return ;    
    }

    //
    void suiUserServerRpc::passwordLogin(::google::protobuf::RpcController* controller,
                            const ::suiApi::passwordLoginReq* request,
                            ::suiApi::passwordLoginRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;

        //提取参数
        response->set_id(request->id());
        std::string sessionid = request->sessionid();
        std::string targetEmail = request->emailnumber();
        std::string password = request->password();
        std::string userId;

        //判断是否是临时会话
        auto selogret = _userMetaOperation->sessionLogin(sessionid);
        if(selogret != suiDataSql::SessionStatus::Guest)
        {
            //此时不是临时会话，直接返回false
            INFO("邮箱登录，会话id： {} 不属于临时会话， 状态： {}", sessionid, suiDataSql::suiSessionStatus::toString(selogret));
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_STATUS_INVALID);
            response->set_errormsg("会话状态异常");
            return;
        }
        //判断用户是否被禁用
        auto enableret = _userMetaOperation->isEnable(targetEmail);
        if(!enableret)
        {
            //此时用户被禁用，直接返回false
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_EMAIL_DISABLE);
            response->set_errormsg("用户被禁用");
            return;
        }

        //开始登录
        auto ret = _userMetaOperation->emailPasswordLogin(sessionid, targetEmail, password, &userId);
        if(!ret)
        {
            //登录失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_EMAIL_LOGIN_FAILED);
            response->set_errormsg("邮箱登录失败");
            return;
        }
        //成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        auto result = response->mutable_result();
        result->set_userid(userId);
        return ;    
    }

    //注销登录
    void suiUserServerRpc::logout(::google::protobuf::RpcController* controller,
                            const ::suiApi::logoutReq* request,
                            ::suiApi::logoutRsp* response,
                            ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        //提取参数
        response->set_id(request->id());
        std::string sessionid = request->sessionid();
        std::string userId = request->userid();

        //验证用户是否有效
        if(!_userMetaOperation->verifySession(sessionid, userId))
        {
            INFO("会话验证身份失败， 会话id： {}， 用户id： {}", sessionid, userId);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED);
            response->set_errormsg("会话身份验证失败");
            return;
        }

        //注销
        _userMetaOperation->logout(sessionid);
        //不会出现错误，直接返回 会话后期不变，即便过期按流程申请
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        return ;
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
        //提取参数
        auto sessionId = request->sessionid();
        auto userId = request->userid();
        auto fileId = request->fileid();

        //验证用户是否有效
        if(!_userMetaOperation->verifySession(sessionId, userId))
        {
            INFO("会话验证身份失败， 会话id： {}， 用户id： {}", sessionId, userId);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED);
            response->set_errormsg("会话身份验证失败");
            return;
        }

        auto ret = _userMetaOperation->changeAvatar(userId, fileId);
        if(!ret)
        {
            //修改失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED);
            response->set_errormsg("用户身份验证失败");
            return;
        }
        //成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        return ;
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
        //获取参数
        auto sessionId = request->sessionid();
        auto userId = request->userid();
        auto nickname = request->nickname();
        //验证用户是否有效
        if(!_userMetaOperation->verifySession(sessionId, userId))
        {
            INFO("会话验证身份失败， 会话id： {}， 用户id： {}", sessionId, userId);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED);
            response->set_errormsg("会话身份验证失败");
            return;
        }

        //直接修改
        auto ret = _userMetaOperation->changeName(userId, nickname);
        if(!ret)
        {
            //修改失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED);
            response->set_errormsg("用户身份验证失败");
            return;
        }
        //成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        return ;
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

        //获取参数
        auto sessionId = request->sessionid();
        auto userId = request->userid();
        auto password = request->password();

        //验证用户是否有效
        if(!_userMetaOperation->verifySession(sessionId, userId))
        {
            INFO("会话验证身份失败， 会话id： {}， 用户id： {}", sessionId, userId);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED);
            response->set_errormsg("会话身份验证失败");
            return;
        }

        auto ret = _userMetaOperation->changePassword(userId, password);
        if(!ret)
        {
            //修改失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED);
            response->set_errormsg("用户身份验证失败");
            return;
        }
        //成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        return ;
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

        auto sessionId = request->sessionid();
        auto userId = request->userid();
        auto targetUserId = request->targetuserid();
        auto userStatus = userStatusTransformation(request->userstatus());

        //验证该会话用户是否有修改用户状态的权限
        if(!_userMetaOperation->hasPermissionBySessionId(sessionId, userId, suiDataSql::identityType::identityTypeAdmin, suiDataSql::roleType::roleTypeAdmin))
        {
            INFO("无修改用户状态的权限， 修改者会话id: {}， 被修改用户id： {}", sessionId, userId);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }

        auto ret = _userMetaOperation->changeStatus(targetUserId, userStatus);
        if(!ret)
        {
            //修改失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED);
            response->set_errormsg("用户身份验证失败");
            return;
        }
        //成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        return ;
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

        //获取参数
        auto targetUserId = request->targetuserid();
        auto userId = request->userid();
        auto sessionId = request->sessionid();
        if(!_userMetaOperation->verifySession(sessionId))
        {
            //会话验证失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID);
            response->set_errormsg("会话失效");
            return;
        }

        //开始获取目标
        auto user = _userMetaOperation->getUserInfo(targetUserId); //用户基础信息
        auto userType = _userMetaOperation->getIdentityRole(targetUserId); //用户身份信息
        auto userStatistics = _userMetaOperation->getUserStatistics(targetUserId); //用户统计信息
        bool isFollow = false;
        if(!userId.empty())
            isFollow = _userMetaOperation->isFollow(userId, targetUserId);
        if(!user || !userType || !userStatistics)
        {
            //此时获取失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND);
            response->set_errormsg("用户不存在");
            return;
        }

        //获取成功，开始构造响应
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        {
            auto result = response->mutable_result();
            auto userInfo = result->mutable_userinfo();
            userInfo->set_userid(user->getUserId());
            userInfo->set_email(user->getBindEmail());
            userInfo->set_nickname(user->getUserName());
            userInfo->set_role(RoleTransformation(userType->getRoleType()));
            userInfo->set_identify(identityTransformation(userType->getIdentityType()));
            userInfo->set_likecount(userStatistics->videoLikeCount);
            userInfo->set_playcount(userStatistics->videoPlayCount);
            userInfo->set_followcount(userStatistics->followCount);
            userInfo->set_funscount(userStatistics->fansCount);
            userInfo->set_status(userStatusTransformation(user->getUserStatus()));
            userInfo->set_isfollowing(isFollowTransformation(isFollow));
            if(!user->getUserDescription().null())
                userInfo->set_usermemo(user->getUserDescription().get());
            userInfo->set_userctime(std::to_string(user->getUploadTime()));
            if(!user->getHeadImageFileId().null())
                userInfo->set_avatarfileid(user->getHeadImageFileId().get());
        }
        return;
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

        //获取参数
        auto sessionId = request->sessionid();
        auto userId = request->userid();
        auto targetUserId = request->targetuserid();

        //验证用户是否有效
        if(!_userMetaOperation->verifySession(sessionId, userId))
        {
            INFO("会话验证身份失败， 会话id： {}， 用户id： {}", sessionId, userId);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED);
            response->set_errormsg("会话身份验证失败");
            return;
        }

        //验证目标用户是否有效
        if(!_userMetaOperation->verifyUser(targetUserId))
        {
            INFO("目标用户验证身份失败， 目标用户id： {}", targetUserId);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND);
            response->set_errormsg("目标用户不存在");
            return;
        }

        //新增关注
        _userMetaOperation->addFollow(userId, targetUserId);
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        return;
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

        //获取参数
        auto sessionId = request->sessionid();
        auto userId = request->userid();
        auto targetUserId = request->targetuserid();

        //验证用户是否有效
        if(!_userMetaOperation->verifySession(sessionId, userId))
        {
            INFO("会话验证身份失败， 会话id： {}， 用户id： {}", sessionId, userId);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SET_AVATAR_FAILED);
            response->set_errormsg("会话身份验证失败");
            return;
        }

        //无论是否关注，不需要验证目标用户是否存在，直接取消关注即可
        _userMetaOperation->removeFollow(userId, targetUserId);
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        return;
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

        //获取参数
        auto sessionId = request->sessionid();
        auto userId = request->userid();
        auto targetUserId = request->targetuserid();
        
        //验证权限
        if(!_userMetaOperation->hasPermissionBySessionId(sessionId, userId, suiDataSql::identityType::identityTypeAdmin, suiDataSql::roleType::roleTypeSuperAdmin)) //需要最高级管理员才可以新增管理员
        {
            INFO("用户： {} 设置管理员权限不足", userId);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }

        //开始业务操作
        if(_userMetaOperation->addAdmin(targetUserId) == false)
        {
            //新增管理员失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_ADMIN_FAILED);
            response->set_errormsg("新增管理员失败");
            return;
        }
        //新增管理员成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        return;
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
        
        //获取参数
        auto sessionId = request->sessionid();
        auto userId = request->userid();
        auto targetUserId = request->targetuserid();

        //验证权限
        if(!_userMetaOperation->hasPermissionBySessionId(sessionId, userId
            , suiDataSql::identityType::identityTypeAdmin, suiDataSql::roleType::roleTypeSuperAdmin)) //需要最高级管理员才可以新增管理员
        {
            INFO("用户： {} 设置管理员权限不足", userId);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }

        //开始业务操作
        if(_userMetaOperation->removeAdmin(targetUserId) == false)
        {
            //删除管理员失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_ADMIN_REMOVE_FAILED);
            response->set_errormsg("删除管理员失败");
            return;
        }
        //新增管理员成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        return;
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

        //获取参数
        auto sessionId = request->sessionid();
        auto userId = request->userid();
        auto& userInfo = request->userinfo();
        auto targetUserId = userInfo.userid();

        //判断参数属性
        if(targetUserId != userId)
        {
            //验证权限
            if(!_userMetaOperation->hasPermissionBySessionId(sessionId, userId
            , suiDataSql::identityType::identityTypeAdmin, suiDataSql::roleType::roleTypeSuperAdmin)) //需要最高级管理员才可以新增管理员
            {
                INFO("用户： {} 设置管理员权限不足", userId);
                response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
                response->set_errormsg("用户权限不足");
                return;
            }
        }
        //开始设置属性
        INFO("修改的目标id:{}", targetUserId);
        INFO("管理员名称： {}", userInfo.nickname());
        INFO("管理员备注： {}", userInfo.usermemo());
        auto setret = _userMetaOperation->setAdminProperty(targetUserId, userInfo.nickname(), userInfo.usermemo());
        if(setret == false)
        {
            //修改失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_ADMIN_SET_FAILED);
            response->set_errormsg("修改管理员失败");
            return;
        }
        //成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        return;
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

        //获取参数
        auto sessionId = request->sessionid(); //只需要验证会话的有效性
        auto targetEmail = request->email();

        if(_userMetaOperation->verifySession(sessionId) == false)
        {
            //会话无效
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID);
            response->set_errormsg("会话无效");
            return;
        }

        //直接查询管理员信息
        auto userInfo = _userMetaOperation->getUserInfoByEmail(targetEmail);
        if(userInfo == nullptr)
        {
            //查询目标错误
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_NOT_FOUND);
            response->set_errormsg("管理员不存在");
            return ;
        }
        //查询权限信息
        auto userAuthority = _userMetaOperation->getIdentityRole(userInfo->getUserId());
        if(userAuthority == nullptr || userAuthority->getIdentityType() != suiDataSql::identityType::identityTypeAdmin)
        {
            //权限异常
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_ADMIN_PERMISSION_INVALID);
            response->set_errormsg("用户权限异常");
            return;
        }
        //填充输出
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        auto result = response->mutable_result();
        auto info = result->mutable_userinfo();

        {
            info->set_userid(userInfo->getUserId());
            if(!userInfo->getAdministratorName().null())
                info->set_nickname(userInfo->getAdministratorName().get());
            info->set_role(RoleTransformation(userAuthority->getRoleType()));
            info->set_identify(identityTransformation(userAuthority->getIdentityType()));
            info->set_status(userStatusTransformation(userInfo->getUserStatus()));
            if(!userInfo->getUserDescription().null())
                info->set_usermemo(userInfo->getUserDescription().get());
            info->set_email(userInfo->getBindEmail());
        }
        return ;
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

        //获取参数
        auto sessionId = request->sessionid(); //只需要验证会话的有效性
        int pageIndex = request->pageindex();
        int pageSize = request->pagecount();
        auto role = RoleTransformation(request->role());

        //只需要验证会话有效性
        if(_userMetaOperation->verifySession(sessionId) == false)
        {
            //会话无效
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID);
            response->set_errormsg("会话无效");
            return;
        }
        if(role == suiDataSql::roleType::roleTypeUnknown || role == suiDataSql::roleType::roleTypeNormal)
        {
            //角色类型不对
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_ADMIN_INVALID);
            response->set_errormsg("申请参数错误");
            return;
        }

        //直接获取信息
        auto userlist = _userMetaOperation->getAdminList(suiDataSql::userStatus::userStatusEnable, suiDataSql::identityType::identityTypeAdmin
                                                        , role, pageIndex, pageSize);

        
        if(userlist != nullptr)
        {
            auto result = response->mutable_result();
            //填充输出
            for(auto userInfo : userlist->list)
            {
                ::suiApi::AdminInfo* info = result->add_userlist();
                //不需要查询用户权限信息，身份信息固定，权限信息由传入信息决定
                {
                    info->set_userid(userInfo.getUserId());
                    if(!userInfo.getAdministratorName().null())
                        info->set_nickname(userInfo.getAdministratorName().get());
                    info->set_role(RoleTransformation(role));
                    info->set_identify(identityTransformation(suiDataSql::identityType::identityTypeAdmin));
                    info->set_status(userStatusTransformation(suiDataSql::userStatus::userStatusEnable));
                    if(!userInfo.getUserDescription().null())
                        info->set_usermemo(userInfo.getUserDescription().get());
                    info->set_email(userInfo.getBindEmail());
                }
            } 
            result->set_totalcount(userlist->total);
        }
        //为空也正常输出
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        
        return ;
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

        //获取参数
        auto userId = request->userid();
        auto salt = request->salt();
        auto sessionId = request->sessionid();

        //验证会话与用户合法性
        if(!_userMetaOperation->verifySession(sessionId, userId))
        {
            //会话无效
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID);
            response->set_errormsg("会话无效");
            return;
        }

        //
        auto ret = _userMetaOperation->setSalt(userId, salt);
        if(ret == false)
        {
            //设置失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_SALT_SET);
            response->set_errormsg("设置失败");
            return;
        }
        //成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        return ;
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

        //获取参数
        auto userId = request->userid();
        auto sessionId = request->sessionid();

        //验证信息
        if(!_userMetaOperation->verifySession(sessionId, userId))
        {
            //会话无效
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_SESSION_INVALID);
            response->set_errormsg("会话无效");
            return;
        }

        std::string salt;
        auto ret = _userMetaOperation->getSalt(userId, salt);
        if(ret == false)
        {
            //获取失败
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_GET_USER_INFO_FAILED);
            response->set_errormsg("获取失败");
            return;
        }
        //成功
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        auto result = response->mutable_result();
        result->set_salt(salt);
        return ;
    }


    void suiUserServerRpc::replaceAll(std::string& str, const std::string& from, const std::string& to)
    {
        if (from.empty()) 
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) 
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); 
        }
    }

    std::string suiUserServerRpc::getEmailHtmlContent(const std::string& title, const std::string& body, const std::string& targetEmail, const std::string& emailfrom)
    {
        //读取html模板
        std::ifstream file("./views/emailVerifyCode.html");
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string htmlContent = buffer.str();
        //替换模板字段
        replaceAll(htmlContent, _emailHtmlTemplateFieldTitle, title);
        replaceAll(htmlContent, _emailHtmlTemplateFieldCode, body);
        replaceAll(htmlContent, _emailHtmlTemplateFieldTargetEmail, targetEmail);
        replaceAll(htmlContent, _emailHtmlTemplateFieldEmailfrom, emailfrom);
        return htmlContent;
    }

    ::suiDataSql::userStatus suiUserServerRpc::userStatusTransformation(::suiApi::userStatus userStatus)
    {
        switch (userStatus)
        {
            case ::suiApi::userStatus::userStatusUnknow:
                return ::suiDataSql::userStatus::userStatusUnknow;
            case ::suiApi::userStatus::userStatusEnable:
                return ::suiDataSql::userStatus::userStatusEnable;
            case ::suiApi::userStatus::userStatusDisable:
                return ::suiDataSql::userStatus::userStatusDisable;
            default:
                return ::suiDataSql::userStatus::userStatusUnknow;
        }
    }

    //用户身份
    ::suiDataSql::identityType suiUserServerRpc::identityTransformation(::suiApi::identityType identity)
    {
        switch (identity)
        {
            case ::suiApi::identityType::identityUnknow:
                return ::suiDataSql::identityType::identityTypeUnknown;
            case ::suiApi::identityType::identityNormal:
                return ::suiDataSql::identityType::identityTypeNormal;
            case ::suiApi::identityType::identityAdmin:
                return ::suiDataSql::identityType::identityTypeAdmin;
            default:
                return ::suiDataSql::identityType::identityTypeUnknown;
        }
    }

    //用户角色类型转换
    ::suiDataSql::roleType suiUserServerRpc::RoleTransformation(::suiApi::roleType Role)
    {
        switch (Role)
        {
            //本地访客也切换成未知用户，因为无法和数据库中的用户属性映射起来
            case ::suiApi::roleType::roleUnknow:
                return ::suiDataSql::roleType::roleTypeUnknown;
            case ::suiApi::roleType::roleUserNormal:
                return ::suiDataSql::roleType::roleTypeNormal;
            case ::suiApi::roleType::roleAdminNormal:
                return ::suiDataSql::roleType::roleTypeAdmin;
            case ::suiApi::roleType::roleAdminSuper:
                return ::suiDataSql::roleType::roleTypeSuperAdmin;
            default:
                return ::suiDataSql::roleType::roleTypeUnknown;
        }
    }

    bool suiUserServerRpc::isFollowTransformation(::suiApi::followStatus value)
    {
        switch (value)
        {
            case ::suiApi::followStatus::followStatusTrue:
                return true;
            default:
                return false;
        }
    }

    //反转
    //用户状态转换
    ::suiApi::userStatus suiUserServerRpc::userStatusTransformation(::suiDataSql::userStatus userStatus)
    {
        switch (userStatus)
        {
            case ::suiDataSql::userStatus::userStatusUnknow:
                return ::suiApi::userStatus::userStatusUnknow;
            case ::suiDataSql::userStatus::userStatusEnable:
                return ::suiApi::userStatus::userStatusEnable;
            case ::suiDataSql::userStatus::userStatusDisable:
                return ::suiApi::userStatus::userStatusDisable;
            default:
                return ::suiApi::userStatus::userStatusUnknow;
        }
    }

    //用户身份
    ::suiApi::identityType suiUserServerRpc::identityTransformation(::suiDataSql::identityType identity)
    {
        switch (identity)
        {
            case ::suiDataSql::identityType::identityTypeUnknown:
                return ::suiApi::identityType::identityUnknow;
            case ::suiDataSql::identityType::identityTypeNormal:
                return ::suiApi::identityType::identityNormal;
            case ::suiDataSql::identityType::identityTypeAdmin:
                return ::suiApi::identityType::identityAdmin;
            default:
                return ::suiApi::identityType::identityUnknow;
        }
    }

    //用户角色类型转换
    ::suiApi::roleType suiUserServerRpc::RoleTransformation(::suiDataSql::roleType Role)
    {
        switch (Role)
        {
            case ::suiDataSql::roleType::roleTypeUnknown:
                return ::suiApi::roleType::roleUnknow;
            case ::suiDataSql::roleType::roleTypeNormal:
                return ::suiApi::roleType::roleUserNormal;
            case ::suiDataSql::roleType::roleTypeAdmin:
                return ::suiApi::roleType::roleAdminNormal;
            case ::suiDataSql::roleType::roleTypeSuperAdmin:
                return ::suiApi::roleType::roleAdminSuper;
            default:
                return ::suiApi::roleType::roleUnknow;
        }
    }

    ::suiApi::followStatus suiUserServerRpc::isFollowTransformation(bool value)
    {
        if (value)
        {
            return ::suiApi::followStatus::followStatusTrue;
        }
        return ::suiApi::followStatus::followStatusFalse;
    }

    

}