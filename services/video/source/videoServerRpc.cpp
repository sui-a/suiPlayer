#include "videoServerRpc.hpp"

namespace suiVideoServer
{
    videoServerRpc::videoServerRpc(videoServerData::ptr serverHandle)
        : _serverHandle(serverHandle)
    {

    }

    void videoServerRpc::newVideo(::google::protobuf::RpcController* controller,
                    const ::suiApi::newVideoReq* request,
                    ::suiApi::newVideoRsp* response,
                    ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->newVideo(request->sessionid(), request->videoinfo(), errorCode, errorMsg);
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::deleteVideo(::google::protobuf::RpcController* controller,
                const ::suiApi::deleteVideoReq* request,
                ::suiApi::deleteVideoRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->deleteVideo(request->sessionid(), request->videoid(), errorCode, errorMsg);
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::isLikeVideo(::google::protobuf::RpcController* controller,
                const ::suiApi::isLikeVideoReq* request,
                ::suiApi::isLikeVideoRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        bool islike = false;
        _serverHandle->videoLikeJudge(request->sessionid(), request->videoid(), errorCode, errorMsg, islike);
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        auto result = response->mutable_result();
        result->set_isliked(islike);
        return;
    }

    void videoServerRpc::setLikeVideo(::google::protobuf::RpcController* controller,
                const ::suiApi::setLikeVideoReq* request,
                ::suiApi::setLikeVideoRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->videoLikeOperation(request->sessionid(), request->videoid(), errorCode, errorMsg);
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::playVideo(::google::protobuf::RpcController* controller,
                const ::suiApi::playVideoReq* request,
                ::suiApi::playVideoRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->videoPlayOperation(request->sessionid(), request->videoid(), errorCode, errorMsg);
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::addVideoSubtitle(::google::protobuf::RpcController* controller,
                const ::suiApi::addVideoSubtitleReq* request,
                ::suiApi::addVideoSubtitleRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->addVideoSubtitle(request->sessionid(), request->videosubtitleinfo(), errorCode, errorMsg);
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::getVideoSubtitle(::google::protobuf::RpcController* controller,
                const ::suiApi::getVideoSubtitleReq* request,
                ::suiApi::getVideoSubtitleRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->getVideoSubtitle(request->sessionid(), request->videoid(), errorCode, errorMsg, *response->mutable_result());
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::reviewVideo(::google::protobuf::RpcController* controller,
                const ::suiApi::checkVideoReq* request,
                ::suiApi::checkVideoRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->checkVideo(request->sessionid(), request->videoid(), request->isapproved(), errorCode, errorMsg);
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::publishVideo(::google::protobuf::RpcController* controller,
                const ::suiApi::publishVideoReq* request,
                ::suiApi::publishVideoRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->onlineVideo(request->sessionid(), request->videoid(), errorCode, errorMsg);
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::removeShelvesVideo(::google::protobuf::RpcController* controller,
                const ::suiApi::removeShelvesVideoReq* request,
                ::suiApi::removeShelvesVideoRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->offlineVideo(request->sessionid(), request->videoid(), errorCode, errorMsg);
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::getUserVideoList(::google::protobuf::RpcController* controller,
                const ::suiApi::getUserVideoListReq* request,
                ::suiApi::getUserVideoListRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->getUserVideoList(request->sessionid(), request->targetuserid(), request->pageindex(), request->pagecount(), errorCode, errorMsg, *response->mutable_result());
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::getStatusVideoList(::google::protobuf::RpcController* controller,
                const ::suiApi::getStatusVideoListReq* request,
                ::suiApi::getStatusVideoListRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->getVideoListByStatus(request->sessionid(), request->videostatus(), request->pageindex()
                                            , request->pagecount(), errorCode, errorMsg, *response->mutable_result());
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::getHomeRandomVideoList(::google::protobuf::RpcController* controller,
                const ::suiApi::getHomeRandomVideoListReq* request,
                ::suiApi::getHomeRandomVideoListRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->getMainVideoList(request->sessionid(), request->pageindex(), request->pagecount(), errorCode, errorMsg, *response->mutable_result());
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::getTagVideoList(::google::protobuf::RpcController* controller,
                const ::suiApi::getTagVideoListReq* request,
                ::suiApi::getTagVideoListRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->getVideoListByTag(request->sessionid(), request->videotagid(), request->pageindex(), request->pagecount(), errorCode, errorMsg, *response->mutable_result());
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::searchVideoList(::google::protobuf::RpcController* controller,
                const ::suiApi::searchVideoListReq* request,
                ::suiApi::searchVideoListRsp* response,
                ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->getVideoListBySearch(request->sessionid(), request->searchkey(), request->pageindex()
                                    , request->pagecount(), errorCode, errorMsg, *response->mutable_result());
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

    void videoServerRpc::getTagList(::google::protobuf::RpcController* controller,
                       const ::suiApi::getTagInfoReq* request,
                       ::suiApi::getTagInfoRsp* response,
                       ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        response->set_id(request->id());
        //调用服务接口
        int32_t errorCode = suiErrorCodeDef::SUCCESS;
        std::string errorMsg = "";
        _serverHandle->getTagList(errorCode, errorMsg, *response);
        response->set_errorcode(errorCode);
        response->set_errormsg(errorMsg);
        return;
    }

}