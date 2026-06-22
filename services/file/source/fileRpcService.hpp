#pragma once
#include "data.hpp"
#include "data-odb.hxx"
#include "file.pb.h"
#include "fileServiceData.hpp"
#include "fileServicemq.hpp"
#include "error.proto.hpp"
#include <brpc/closure_guard.h>
#include <suiScaffold/suiRandom.hpp>
#include <suiScaffold/suiHash.hpp>
#include "FileTarcker.hpp"

namespace suiFileService
{
    class suiFileRpcService : public suiApi::fileServices
    {
    public:
        using ptr = std::shared_ptr<suiFileRpcService>;

        suiFileRpcService(fileMetaService::ptr fileMetaServicePtr);
        virtual ~suiFileRpcService();

        //接口重写
        virtual void uploadImage(::google::protobuf::RpcController* controller,
                       const ::suiApi::uploadImageReq* request,
                       ::suiApi::uploadImageRsp* response,
                       ::google::protobuf::Closure* done) override;

        virtual void downloadImage(::google::protobuf::RpcController* controller,
                            const ::suiApi::downloadImageReq* request,
                            ::suiApi::downloadImageRsp* response,
                            ::google::protobuf::Closure* done) override;

        virtual void uploadFile(::google::protobuf::RpcController* controller,
                       const ::suiApi::uploadFileReq* request,
                       ::suiApi::uploadFileRsp* response,
                       ::google::protobuf::Closure* done);
        virtual void completeVideoUpload(::google::protobuf::RpcController* controller,
                            const ::suiApi::completeVideoUploadReq* request,
                            ::suiApi::completeVideoUploadRsp* response,
                            ::google::protobuf::Closure* done);
        virtual void getVideoData(::google::protobuf::RpcController* controller,
                            const ::suiApi::InitGetVideoDataReq* request,
                            ::suiApi::InitGetVideoDataRsp* response,
                            ::google::protobuf::Closure* done);
        virtual void downloadVideo(::google::protobuf::RpcController* controller,
                            const ::suiApi::downloadVideoReq* request,
                            ::suiApi::downloadVideoRsp* response,
                            ::google::protobuf::Closure* done);

    private:
        //创建文件id
        void createFileId(std::string& fileid);

        //数据校验
        bool dataValidation(const std::string& data, const std::string& Md5);

        //创建上传id
        void createUploadId(const std::string& Md5, std::string& uploadId);
        //创建下载id
        void createDownloadId(const std::string& Md5, std::string& downloadId);
    private:
        //文件元数据管理服务
        fileMetaService::ptr _fileMetaServicePtr;
        //文件操作跟踪
        std::unordered_map<std::string, suifileTarcker::FileTracker> _fileTrackerMap;  //缺少过期策略，容易内存溢出
        //标准分片大小
        static size_t _standardChunkSize;
    };





}