#pragma once
#include <brpc/closure_guard.h>
#include "videoServerData.hpp"

namespace suiVideoServer
{
    class videoServerRpc : public suiApi::VideoService
    {
    public:
        using ptr = std::shared_ptr<videoServerRpc>;
        videoServerRpc(videoServerData::ptr serverHandle);


        virtual void newVideo(::google::protobuf::RpcController* controller,
                       const ::suiApi::newVideoReq* request,
                       ::suiApi::newVideoRsp* response,
                       ::google::protobuf::Closure* done) override;
        virtual void deleteVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::deleteVideoReq* request,
                            ::suiApi::deleteVideoRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void isLikeVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::isLikeVideoReq* request,
                            ::suiApi::isLikeVideoRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void setLikeVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::setLikeVideoReq* request,
                            ::suiApi::setLikeVideoRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void playVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::playVideoReq* request,
                            ::suiApi::playVideoRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void addVideoSubtitle(::google::protobuf::RpcController* controller,
                            const ::suiApi::addVideoSubtitleReq* request,
                            ::suiApi::addVideoSubtitleRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void getVideoSubtitle(::google::protobuf::RpcController* controller,
                            const ::suiApi::getVideoSubtitleReq* request,
                            ::suiApi::getVideoSubtitleRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void reviewVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::checkVideoReq* request,
                            ::suiApi::checkVideoRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void publishVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::publishVideoReq* request,
                            ::suiApi::publishVideoRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void removeShelvesVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::removeShelvesVideoReq* request,
                            ::suiApi::removeShelvesVideoRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void getUserVideoList(::google::protobuf::RpcController* controller,
                            const ::suiApi::getUserVideoListReq* request,
                            ::suiApi::getUserVideoListRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void getStatusVideoList(::google::protobuf::RpcController* controller,
                            const ::suiApi::getStatusVideoListReq* request,
                            ::suiApi::getStatusVideoListRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void getHomeRandomVideoList(::google::protobuf::RpcController* controller,
                            const ::suiApi::getHomeRandomVideoListReq* request,
                            ::suiApi::getHomeRandomVideoListRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void getTagVideoList(::google::protobuf::RpcController* controller,
                            const ::suiApi::getTagVideoListReq* request,
                            ::suiApi::getTagVideoListRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void searchVideoList(::google::protobuf::RpcController* controller,
                            const ::suiApi::searchVideoListReq* request,
                            ::suiApi::searchVideoListRsp* response,
                            ::google::protobuf::Closure* done) override;
        virtual void getTagList(::google::protobuf::RpcController* controller,
                       const ::suiApi::getTagInfoReq* request,
                       ::suiApi::getTagInfoRsp* response,
                       ::google::protobuf::Closure* done) override;
    private:
        videoServerData::ptr _serverHandle; //服务操作句柄
    };
}