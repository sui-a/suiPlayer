#include "getwayServerRpc.hpp"

namespace suiGetwayServer
{
    getwayServerRpc::getwayServerRpc(const std::string& user_server_name, const std::string& video_server_name, const std::string& file_server_name
                        , suiEtcd::serSearch::ptr search, suiRpc::svcChannels::ptr srvChannels, getwayServerData::ptr dataHandle)
        : _user_server_name(user_server_name)
        , _video_server_name(video_server_name)
        , _file_server_name(file_server_name)
        , _search(search)
        , _srvChannels(srvChannels)
        , _dataHandle(dataHandle)
    {
        _search->search();
    }

    void getwayServerRpc::tempLogin(::google::protobuf::RpcController* controller,
                       const ::suiApi::tempLoginReq* request,
                       ::suiApi::tempLoginRsp* response,
                       ::google::protobuf::Closure* done)
    {
        INFO("临时会话申请请求");
        (void)controller;
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //不需要权限验证,直接转发
        //发起实例化服务对象与控制对象
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.tempLogin(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::sessionLogin(::google::protobuf::RpcController* controller,
                        const ::suiApi::sessionLoginReq* request,
                        ::suiApi::sessionLoginRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("会话登录请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.sessionLogin(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::getEmailCode(::google::protobuf::RpcController* controller,
                        const ::suiApi::getEmailCodeReq* request,
                        ::suiApi::getEmailCodeRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("获取邮箱验证码申请请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.getEmailCode(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::emailCodeLogin(::google::protobuf::RpcController* controller,
                        const ::suiApi::emailNumberLoginReq* request,
                        ::suiApi::emailNumberLoginRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("邮箱验证码登录请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.emailCodeLogin(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::passwordLogin(::google::protobuf::RpcController* controller,
                        const ::suiApi::passwordLoginReq* request,
                        ::suiApi::passwordLoginRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("密码登录请求，会话id：{}，请求路径：{}", ssid, http_url);
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.passwordLogin(cntl, request, response, closure);
        }   
    }
    
    void getwayServerRpc::logout(::google::protobuf::RpcController* controller,
                        const ::suiApi::logoutReq* request,
                        ::suiApi::logoutRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("会话退出请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.logout(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::setUserAvatar(::google::protobuf::RpcController* controller,
                        const ::suiApi::setUserAvatarReq* request,
                        ::suiApi::setUserAvatarRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("设置用户像像请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.setUserAvatar(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::setUserNickname(::google::protobuf::RpcController* controller,
                        const ::suiApi::setUserNicknameReq* request,
                        ::suiApi::setUserNicknameRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("设置用户昵称请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.setUserNickname(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::setPassword(::google::protobuf::RpcController* controller,
                        const ::suiApi::setPasswordReq* request,
                        ::suiApi::setPasswordRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("设置用户密码请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.setPassword(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::setUserStatus(::google::protobuf::RpcController* controller,
                        const ::suiApi::setUserStatusReq* request,
                        ::suiApi::setUserStatusRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("设置用户状态请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.setUserStatus(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::getUserInfo(::google::protobuf::RpcController* controller,
                        const ::suiApi::userInfoReq* request,
                        ::suiApi::userInfoRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("获取用户信息请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.getUserInfo(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::newFollow(::google::protobuf::RpcController* controller,
                        const ::suiApi::newFollowReq* request,
                        ::suiApi::newFollowRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("新增关注请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.newFollow(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::DelFollow(::google::protobuf::RpcController* controller,
                        const ::suiApi::DelFollowReq* request,
                        ::suiApi::DelFollowRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("删除关注请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.DelFollow(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::NewAdmin(::google::protobuf::RpcController* controller,
                        const ::suiApi::NewAdminReq* request,
                        ::suiApi::NewAdminRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("新增管理员请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.NewAdmin(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::DelAdmin(::google::protobuf::RpcController* controller,
                        const ::suiApi::DelAdminReq* request,
                        ::suiApi::DelAdminRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("删除管理员请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.DelAdmin(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::SetAdmin(::google::protobuf::RpcController* controller,
                        const ::suiApi::SetAdminReq* request,
                        ::suiApi::SetAdminRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("设置管理员状态请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.SetAdmin(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::GetAdmin(::google::protobuf::RpcController* controller,
                        const ::suiApi::GetAdminReq* request,
                        ::suiApi::GetAdminRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("获取管理员状态请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.GetAdmin(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::GetAdminList(::google::protobuf::RpcController* controller,
                        const ::suiApi::GetAdminListReq* request,
                        ::suiApi::GetAdminListRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("获取管理员列表请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.GetAdminList(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::SetSalt(::google::protobuf::RpcController* controller,
                        const ::suiApi::SetSaltReq* request,
                        ::suiApi::SetSaltRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("设置管理员盐请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.SetSalt(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::GetSalt(::google::protobuf::RpcController* controller,
                        const ::suiApi::GetSaltReq* request,
                        ::suiApi::GetSaltRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("获取管理员盐请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::userServices_Stub stub(channel.get());
            stub.GetSalt(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::uploadImage(::google::protobuf::RpcController* controller,
                        const ::suiApi::httpUploadImageReq* request,
                        ::suiApi::HttpBody* response,
                        ::google::protobuf::Closure* done)
    {
        INFO("上传图片请求");
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        //获取会话id
        const std::string* ssid = http_cntl->http_request().uri().GetQuery("sessionId");
        //鉴定权限
        if(ssid == nullptr || _dataHandle->verificationPermission(*ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);

            return;
        }
        std::string mime = http_cntl->http_request().content_type();
        //获取请求正文
        auto& body = http_cntl->request_attachment();
    }
    
    void getwayServerRpc::downloadImage(::google::protobuf::RpcController* controller,
                        const ::suiApi::httpDownloadImageReq* request,
                        ::suiApi::HttpBody* response,
                        ::google::protobuf::Closure* done)
    {
        INFO("下载图片请求");
    }
    
    void getwayServerRpc::uploadFile(::google::protobuf::RpcController* controller,
                        const ::suiApi::httpUploadFileReq* request,
                        ::suiApi::HttpBody* response,
                        ::google::protobuf::Closure* done)
    {
        INFO("上传文件请求");
    }
    
    void getwayServerRpc::completeFileUpload(::google::protobuf::RpcController* controller,
                        const ::suiApi::httpCompleteFileUploadReq* request,
                        ::suiApi::HttpBody* response,
                        ::google::protobuf::Closure* done)
    {
        INFO("完成文件上传请求");
    }
    
    void getwayServerRpc::getFileData(::google::protobuf::RpcController* controller,
                        const ::suiApi::httpInitGetFileDataReq* request,
                        ::suiApi::HttpBody* response,
                        ::google::protobuf::Closure* done)
    {
        INFO("获取文件数据请求");
    }
    
    void getwayServerRpc::downloadFile(::google::protobuf::RpcController* controller,
                        const ::suiApi::httpDownloadFileReq* request,
                        ::suiApi::HttpBody* response,
                        ::google::protobuf::Closure* done)
    {
        INFO("下载文件请求");
    }
    
    void getwayServerRpc::newVideo(::google::protobuf::RpcController* controller,
                        const ::suiApi::newVideoReq* request,
                        ::suiApi::newVideoRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("创建新视频请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.newVideo(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::deleteVideo(::google::protobuf::RpcController* controller,
                        const ::suiApi::deleteVideoReq* request,
                        ::suiApi::deleteVideoRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("删除视频请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.deleteVideo(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::isLikeVideo(::google::protobuf::RpcController* controller,
                        const ::suiApi::isLikeVideoReq* request,
                        ::suiApi::isLikeVideoRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("判断视频点赞请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.isLikeVideo(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::setLikeVideo(::google::protobuf::RpcController* controller,
                        const ::suiApi::setLikeVideoReq* request,
                        ::suiApi::setLikeVideoRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("设置视频点赞请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_user_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.setLikeVideo(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::playVideo(::google::protobuf::RpcController* controller,
                        const ::suiApi::playVideoReq* request,
                        ::suiApi::playVideoRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("播放视频请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.playVideo(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::addVideoSubtitle(::google::protobuf::RpcController* controller,
                        const ::suiApi::addVideoSubtitleReq* request,
                        ::suiApi::addVideoSubtitleRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("添加视频字幕请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.addVideoSubtitle(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::getVideoSubtitle(::google::protobuf::RpcController* controller,
                        const ::suiApi::getVideoSubtitleReq* request,
                        ::suiApi::getVideoSubtitleRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("获取视频字幕请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.getVideoSubtitle(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::reviewVideo(::google::protobuf::RpcController* controller,
                        const ::suiApi::checkVideoReq* request,
                        ::suiApi::checkVideoRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("判断视频点赞请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.reviewVideo(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::publishVideo(::google::protobuf::RpcController* controller,
                        const ::suiApi::publishVideoReq* request,
                        ::suiApi::publishVideoRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("发布视频请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.publishVideo(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::removeShelvesVideo(::google::protobuf::RpcController* controller,
                        const ::suiApi::removeShelvesVideoReq* request,
                        ::suiApi::removeShelvesVideoRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("从视频架移除视频请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.removeShelvesVideo(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::getUserVideoList(::google::protobuf::RpcController* controller,
                        const ::suiApi::getUserVideoListReq* request,
                        ::suiApi::getUserVideoListRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("获取用户视频列表请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.getUserVideoList(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::getStatusVideoList(::google::protobuf::RpcController* controller,
                        const ::suiApi::getStatusVideoListReq* request,
                        ::suiApi::getStatusVideoListRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("获取用户视频列表请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.getStatusVideoList(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::getHomeRandomVideoList(::google::protobuf::RpcController* controller,
                        const ::suiApi::getHomeRandomVideoListReq* request,
                        ::suiApi::getHomeRandomVideoListRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("获取随机随机视频列表请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.getHomeRandomVideoList(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::getTagVideoList(::google::protobuf::RpcController* controller,
                        const ::suiApi::getTagVideoListReq* request,
                        ::suiApi::getTagVideoListRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("获取标签下视频列表请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.getTagVideoList(cntl, request, response, closure);
        }
    }
    
    void getwayServerRpc::searchVideoList(::google::protobuf::RpcController* controller,
                        const ::suiApi::searchVideoListReq* request,
                        ::suiApi::searchVideoListRsp* response,
                        ::google::protobuf::Closure* done)
    {
        //提取会话id
        std::string ssid = request->sessionid();
        //提取请求路径
        brpc::Controller* http_cntl = (brpc::Controller*)controller;
        std::string http_url = http_cntl->http_request().uri().path();
        INFO("搜索视频列表请求，会话id：{}，请求路径：{}", ssid, http_url);
        
        if(_dataHandle->verificationPermission(ssid, http_url) == false)
        {
            //权限验证失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_USER_SERVICE_USER_PERMISSION_INVALID);
            response->set_errormsg("用户权限不足");
            return;
        }
        //验证成功
        //获取服务信道
        auto channel = getChannel(_video_server_name);
        if(channel == nullptr)
        {
            //服务未上线，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("服务未上线");
            return;
        }
        //开始调用 
        brpc::Controller* cntl = new brpc::Controller();
        auto closure = suiRpc::ClosureFactory::create([=]()->void{
            brpc::ClosureGuard doneGuard(done);
            std::shared_ptr<brpc::Controller> cntlPtr(cntl); //自动销毁
            if(cntl->Failed())
            {
                //调用失败,无任何错误信息，直接构造错误信息返回
                response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
                response->set_errormsg("服务调用失败");
                return;
            }
            //即便是调用失败，也是逻辑上失败，已经存在错误信息，直接返回即可
        });
        if(closure == nullptr)
        {
            //创建闭包失败，返回服务器错误
            brpc::ClosureGuard doneGuard(done);
            response->set_errorcode(suiErrorCodeDef::ERR_SERVER);
            response->set_errormsg("创建闭包失败");
            return;
        }
        else
        {
            //直接调用运行
            suiApi::VideoService_Stub stub(channel.get());
            stub.searchVideoList(cntl, request, response, closure);
        }
    }
    

    suiRpc::ChannelPtr getwayServerRpc::getChannel(std::string& srvName)
    {
        int retry = 5;
        suiRpc::ChannelPtr curChannel;
        while(retry--)
        {
            curChannel = _srvChannels->getNode(srvName);
            if(curChannel)
                return curChannel;
            INFO("等待 {} 服务上线", srvName);
            sleep(1);
        }
        return nullptr;
    }
}