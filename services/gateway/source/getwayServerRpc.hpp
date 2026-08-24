#pragma once
#include <memory>
#include <string>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiRpc.hpp>
#include "base.pb.h"
#include "http.pb.h"
#include "error.proto.hpp"
#include "getwayServerData.hpp"

namespace suiGetwayServer
{
    class getwayServerRpc : public suiApi::httpServices
    {
    public:
        using ptr = std::shared_ptr<getwayServerRpc>;
        getwayServerRpc(const std::string& user_server_name, const std::string& video_server_name, const std::string& file_server_name
                        , suiEtcd::serSearch::ptr search, suiRpc::svcChannels::ptr srvChannels, getwayServerData::ptr dataHandle);
        
        void tempLogin(::google::protobuf::RpcController* controller,
                       const ::suiApi::tempLoginReq* request,
                       ::suiApi::tempLoginRsp* response,
                       ::google::protobuf::Closure* done) override;

        void sessionLogin(::google::protobuf::RpcController* controller,
                            const ::suiApi::sessionLoginReq* request,
                            ::suiApi::sessionLoginRsp* response,
                            ::google::protobuf::Closure* done) override;

        void getEmailCode(::google::protobuf::RpcController* controller,
                            const ::suiApi::getEmailCodeReq* request,
                            ::suiApi::getEmailCodeRsp* response,
                            ::google::protobuf::Closure* done) override;

        void emailCodeLogin(::google::protobuf::RpcController* controller,
                            const ::suiApi::emailNumberLoginReq* request,
                            ::suiApi::emailNumberLoginRsp* response,
                            ::google::protobuf::Closure* done) override;

        void passwordLogin(::google::protobuf::RpcController* controller,
                            const ::suiApi::passwordLoginReq* request,
                            ::suiApi::passwordLoginRsp* response,
                            ::google::protobuf::Closure* done) override;

        void logout(::google::protobuf::RpcController* controller,
                            const ::suiApi::logoutReq* request,
                            ::suiApi::logoutRsp* response,
                            ::google::protobuf::Closure* done) override;

        void setUserAvatar(::google::protobuf::RpcController* controller,
                            const ::suiApi::setUserAvatarReq* request,
                            ::suiApi::setUserAvatarRsp* response,
                            ::google::protobuf::Closure* done) override;

        void setUserNickname(::google::protobuf::RpcController* controller,
                            const ::suiApi::setUserNicknameReq* request,
                            ::suiApi::setUserNicknameRsp* response,
                            ::google::protobuf::Closure* done) override;

        void setPassword(::google::protobuf::RpcController* controller,
                            const ::suiApi::setPasswordReq* request,
                            ::suiApi::setPasswordRsp* response,
                            ::google::protobuf::Closure* done) override;

        void setUserStatus(::google::protobuf::RpcController* controller,
                            const ::suiApi::setUserStatusReq* request,
                            ::suiApi::setUserStatusRsp* response,
                            ::google::protobuf::Closure* done) override;

        void getUserInfo(::google::protobuf::RpcController* controller,
                            const ::suiApi::userInfoReq* request,
                            ::suiApi::userInfoRsp* response,
                            ::google::protobuf::Closure* done) override;

        void newFollow(::google::protobuf::RpcController* controller,
                            const ::suiApi::newFollowReq* request,
                            ::suiApi::newFollowRsp* response,
                            ::google::protobuf::Closure* done) override;

        void DelFollow(::google::protobuf::RpcController* controller,
                            const ::suiApi::DelFollowReq* request,
                            ::suiApi::DelFollowRsp* response,
                            ::google::protobuf::Closure* done) override;

        void NewAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::NewAdminReq* request,
                            ::suiApi::NewAdminRsp* response,
                            ::google::protobuf::Closure* done) override;

        void DelAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::DelAdminReq* request,
                            ::suiApi::DelAdminRsp* response,
                            ::google::protobuf::Closure* done) override;

        void SetAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::SetAdminReq* request,
                            ::suiApi::SetAdminRsp* response,
                            ::google::protobuf::Closure* done) override;

        void GetAdmin(::google::protobuf::RpcController* controller,
                            const ::suiApi::GetAdminReq* request,
                            ::suiApi::GetAdminRsp* response,
                            ::google::protobuf::Closure* done) override;

        void GetAdminList(::google::protobuf::RpcController* controller,
                            const ::suiApi::GetAdminListReq* request,
                            ::suiApi::GetAdminListRsp* response,
                            ::google::protobuf::Closure* done) override;

        void SetSalt(::google::protobuf::RpcController* controller,
                            const ::suiApi::SetSaltReq* request,
                            ::suiApi::SetSaltRsp* response,
                            ::google::protobuf::Closure* done) override;

        void GetSalt(::google::protobuf::RpcController* controller,
                            const ::suiApi::GetSaltReq* request,
                            ::suiApi::GetSaltRsp* response,
                            ::google::protobuf::Closure* done) override;

        void uploadImage(::google::protobuf::RpcController* controller,
                            const ::suiApi::httpUploadImageReq* request,
                            ::suiApi::HttpBody* response,
                            ::google::protobuf::Closure* done) override;

        void downloadImage(::google::protobuf::RpcController* controller,
                            const ::suiApi::httpDownloadImageReq* request,
                            ::suiApi::HttpBody* response,
                            ::google::protobuf::Closure* done) override;

        void uploadFile(::google::protobuf::RpcController* controller,
                            const ::suiApi::httpUploadFileReq* request,
                            ::suiApi::HttpBody* response,
                            ::google::protobuf::Closure* done) override;

        void completeFileUpload(::google::protobuf::RpcController* controller,
                            const ::suiApi::httpCompleteFileUploadReq* request,
                            ::suiApi::HttpBody* response,
                            ::google::protobuf::Closure* done) override;

        void getFileData(::google::protobuf::RpcController* controller,
                            const ::suiApi::httpInitGetFileDataReq* request,
                            ::suiApi::HttpBody* response,
                            ::google::protobuf::Closure* done) override;

        void downloadFile(::google::protobuf::RpcController* controller,
                            const ::suiApi::httpDownloadFileReq* request,
                            ::suiApi::HttpBody* response,
                            ::google::protobuf::Closure* done) override;

        void newVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::newVideoReq* request,
                            ::suiApi::newVideoRsp* response,
                            ::google::protobuf::Closure* done) override;

        void deleteVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::deleteVideoReq* request,
                            ::suiApi::deleteVideoRsp* response,
                            ::google::protobuf::Closure* done) override;

        void isLikeVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::isLikeVideoReq* request,
                            ::suiApi::isLikeVideoRsp* response,
                            ::google::protobuf::Closure* done) override;

        void setLikeVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::setLikeVideoReq* request,
                            ::suiApi::setLikeVideoRsp* response,
                            ::google::protobuf::Closure* done) override;

        void playVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::playVideoReq* request,
                            ::suiApi::playVideoRsp* response,
                            ::google::protobuf::Closure* done) override;

        void addVideoSubtitle(::google::protobuf::RpcController* controller,
                            const ::suiApi::addVideoSubtitleReq* request,
                            ::suiApi::addVideoSubtitleRsp* response,
                            ::google::protobuf::Closure* done) override;

        void getVideoSubtitle(::google::protobuf::RpcController* controller,
                            const ::suiApi::getVideoSubtitleReq* request,
                            ::suiApi::getVideoSubtitleRsp* response,
                            ::google::protobuf::Closure* done) override;

        void reviewVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::checkVideoReq* request,
                            ::suiApi::checkVideoRsp* response,
                            ::google::protobuf::Closure* done) override;

        void publishVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::publishVideoReq* request,
                            ::suiApi::publishVideoRsp* response,
                            ::google::protobuf::Closure* done) override;

        void removeShelvesVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::removeShelvesVideoReq* request,
                            ::suiApi::removeShelvesVideoRsp* response,
                            ::google::protobuf::Closure* done) override;

        void getUserVideoList(::google::protobuf::RpcController* controller,
                            const ::suiApi::getUserVideoListReq* request,
                            ::suiApi::getUserVideoListRsp* response,
                            ::google::protobuf::Closure* done) override;

        void getStatusVideoList(::google::protobuf::RpcController* controller,
                            const ::suiApi::getStatusVideoListReq* request,
                            ::suiApi::getStatusVideoListRsp* response,
                            ::google::protobuf::Closure* done) override;

        void getHomeRandomVideoList(::google::protobuf::RpcController* controller,
                            const ::suiApi::getHomeRandomVideoListReq* request,
                            ::suiApi::getHomeRandomVideoListRsp* response,
                            ::google::protobuf::Closure* done) override;

        void getTagVideoList(::google::protobuf::RpcController* controller,
                            const ::suiApi::getTagVideoListReq* request,
                            ::suiApi::getTagVideoListRsp* response,
                            ::google::protobuf::Closure* done) override;

        void searchVideoList(::google::protobuf::RpcController* controller,
                            const ::suiApi::searchVideoListReq* request,
                            ::suiApi::searchVideoListRsp* response,
                            ::google::protobuf::Closure* done) override;
        
    private:
        //获取服务信道
        suiRpc::ChannelPtr getChannel(std::string& srvName);

    private:
        std::string _user_server_name;
        std::string _video_server_name;
        std::string _file_server_name;
        suiEtcd::serSearch::ptr _search;
        suiRpc::svcChannels::ptr _srvChannels;
        //数据业务封装接口
        getwayServerData::ptr _dataHandle;
    };
}