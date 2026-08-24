#include "fileRpcService.hpp"

namespace suiFileService
{
    suiFileRpcService::suiFileRpcService(fileMetaService::ptr fileMetaServicePtr)
        : _fileMetaServicePtr(fileMetaServicePtr)
    {

    }

    suiFileRpcService::~suiFileRpcService()
    {

    }



    void suiFileRpcService::uploadImage(::google::protobuf::RpcController* controller,
        const ::suiApi::uploadImageReq* request,
        ::suiApi::uploadImageRsp* response,
        ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        
        //验证权限
        std::string sessionId = request->sessionid();
        std::string serviceId = request->id();
        //
        auto curUserId = _fileMetaServicePtr->getSessionUser(sessionId);
        if(!curUserId.has_value())
        {
            //为空
            //用户数据为空，说明临时用户，无权限
            ERROR("会话id: {} 请求上传图像失败, 错误： {}", sessionId, "无上传权限");
            ERROR("用户数据为空");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_USER_NO_PERMISSION);
            response->set_errormsg("无上传权限");
            response->set_id(serviceId);
            return;
        }

        //验证用户会话匹配
        const ::suiApi::FileInfo& curFile = request->fileinfo();
        if(curFile.usrid() != curUserId.value())
        {
            //用户id不匹配
            ERROR("会话id: {} 请求上传图像失败, 错误： {}", sessionId, "用户id不匹配");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_USER_MISMATCH);
            response->set_errormsg("用户id不匹配");
            response->set_id(serviceId);
            return;
        }

        //开始上传
        std::string filePath;
        auto uploadResult = suifd::suiFastdfs::upload_from_buff(curFile.filedata(), filePath);
        if(uploadResult.has_value())
        {
            //出现错误
            ERROR("会话id: {} 请求上传图像失败, 错误： {}", sessionId, uploadResult.value());
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_FAILED); //上传失败
            response->set_errormsg(uploadResult.value());
            response->set_id(serviceId);
            return;
        }

        //上传成功 添加进数据库
        //生成随机文件id
        std::string fileId;
        createFileId(fileId);  //生成随机文件id

        //构建odb数据对象
        suiDataSql::suiFileMeta fileMeta(fileId, curUserId.value(), filePath, curFile.filesize(), curFile.filemime());
        fileMeta.setFileStatus(suiDataSql::fileStatus::fileStatusSuccess);
        //添加进数据库
        _fileMetaServicePtr->newFileMeta(fileMeta);

        //构建返回消息
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        response->set_id(serviceId);
        auto* curResult = response->mutable_result();
        curResult->set_fileid(fileId);
        return;
    }

    //下载图像
    void suiFileRpcService::downloadImage(::google::protobuf::RpcController* controller,
        const ::suiApi::downloadImageReq* request,
        ::suiApi::downloadImageRsp* response,
        ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;

        //获取请求信息
        std::string sessionId = request->sessionid();
        std::string serviceId = request->id();
        std::string fileId = request->fileid();
        INFO("下载图像请求，文件id为： {}", fileId);

        //验证权限
        auto curUserId = _fileMetaServicePtr->getSessionUser(sessionId);
        if(!curUserId.has_value())
        {
            //为空
            //用户数据为空，说明临时用户，无权限
            ERROR("会话id: {} 请求下载图像失败, 错误： {}", sessionId, "无下载权限");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_USER_NO_PERMISSION);
            response->set_errormsg("无上传权限");
            response->set_id(serviceId);
            return;
        }

        //权限验证成功
        //开始获取图片元信息
        auto fileMeta = _fileMetaServicePtr->getFileMeta(fileId);
        if(!fileMeta)
        {
            //文件元信息为空
            ERROR("会话id: {} 请求下载图像失败, 错误： {}", sessionId, "文件元信息不存在");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_NOT_FOUND);
            response->set_errormsg("图像不存在");
            response->set_id(serviceId);
            return;
        }
        if(fileMeta->getFileStatus() != suiDataSql::fileStatus::fileStatusSuccess)
        {
            //文件状态不是成功
            ERROR("会话id: {} 请求下载图像失败, 错误： {}", sessionId, "文件状态不对");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_STATUS_ERROR);
            response->set_errormsg("文件状态不对");
            response->set_id(serviceId);
            return;
        }

        auto filePath = fileMeta->getPath();
        if(filePath.null())
        {
            //文件路径为空
            ERROR("会话id: {} 请求下载图像失败, 错误： {}", sessionId, "文件元信息路径为空");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_NOT_FOUND);
            response->set_errormsg("图像不存在");
            response->set_id(serviceId);
            return;
        }

        //元信息获取成功
        //开始获取图像数据
        std::string filePathData = filePath.get();
        std::string fileData;
        INFO("图像路径为： {}", filePathData);
        auto downloadErrorResult = suifd::suiFastdfs::download_to_buff(filePathData, fileData);
        if(downloadErrorResult.has_value())
        {
            //存在错误
            ERROR("会话id: {} 请求下载图像失败, 错误： {}", sessionId, downloadErrorResult.value());
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_DOWNLOAD_FAILED);
            response->set_errormsg("文件下载失败");
            response->set_id(serviceId);
            return;
        }
        
        //下载成功
        //构建返回消息
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        response->set_id(serviceId);
        //文件返回结构
        auto* curResult = response->mutable_fileinfo();
        curResult->set_fileid(fileId);
        curResult->set_usrid(fileMeta->getUploadUserId());
        curResult->set_filedata(fileData);
        if(!fileMeta->getMimeType().null())
            curResult->set_filemime(fileMeta->getMimeType().get());
        curResult->set_filesize(fileMeta->getSize());
        return;
    }

    //分片上传
    void suiFileRpcService::uploadFile(::google::protobuf::RpcController* controller,
        const ::suiApi::uploadFileReq* request,
        ::suiApi::uploadFileRsp* response,
        ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;
        //获取请求信息
        std::string sessionId = request->sessionid();
        std::string serviceId = request->id();
        //验证权限
        auto curUserId = _fileMetaServicePtr->getSessionUser(sessionId);
        if(!curUserId.has_value())
        {
            //为空
            //用户数据为空，说明临时用户，无权限
            ERROR("会话id: {} 请求上传文件失败, 错误： {}", sessionId, "无上传权限");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_USER_NO_PERMISSION);
            response->set_errormsg("无上传权限");
            response->set_id(serviceId);
            return;
        }

        //获取调用参数
        auto& param = request->param();
        std::string fileId = param.fileid();
        std::string data = param.chunkdata();
        const std::string& chunkMd5 = param.chunkmd5();
        int32_t fileSize = param.filesize();
        const std::string& filemime = param.filemime();
        std::string uploadId = param.uploadid();
        int32_t chunkTotal = param.chunktotal();
        int32_t chunkIndex = param.chunkindex();

        //直接验证数据
        if(!dataValidation(data, chunkMd5))  //牺牲性能完成解耦
        {
            //数据验证失败
            ERROR("会话id: {} 请求上传文件失败, 错误： {}", sessionId, "数据验证失败");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_DATA_INVALID);
            response->set_errormsg("数据验证失败");
            response->set_id(serviceId);
            return;
        }
        if(fileId.empty())
        {
            if(chunkIndex != 0)
            {
                //索引不对
                ERROR("会话id: {} 请求上传文件失败, 错误： {}, 要求序号为0，当前序号为: {}", sessionId, "上传分片的序号不对", chunkIndex);
                response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_CHUNK_INDEX_INVALID);
                response->set_id(serviceId);
                response->set_errormsg("上传分片的序号不对");
                auto rspResult = response->mutable_result();
                rspResult->set_curoperationsize(0);
                return;
            }

            //为空则首次添加
            //创建上传id
            while(1)
            {
                createUploadId(chunkMd5, uploadId);
                //防止碰撞
                if(_fileTrackerMap.find(uploadId) == _fileTrackerMap.end())
                    break; //不存在碰撞，寻找成功
            }
            //创建文件id
            createFileId(fileId);

            //开始上传
            std::string filePath;
            auto uploadResult = suifd::suiFastdfs::upload_appender_frist_from_buff(data, filePath);
            if(uploadResult.has_value())
            {
                //出现错误
                ERROR("会话id: {} 请求上传文件失败, 错误： {}", sessionId, uploadResult.value());
                response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_FAILED); //上传失败
                response->set_errormsg(uploadResult.value());
                response->set_id(serviceId);
                return;
            }
            //添加数据库
            //构建odb数据对象
            suiDataSql::suiFileMeta fileMeta(fileId, curUserId.value(), filePath, fileSize, filemime);
            fileMeta.setFileStatus(suiDataSql::fileStatus::fileStatusUploading);
            //添加进数据库
            _fileMetaServicePtr->newFileMeta(fileMeta);

            //创建操作维护
            suifileTarcker::FileTracker fileTracker;
            fileTracker.setFileId(fileId);
            fileTracker.setOperationType(suifileTarcker::FileTrackerOperationType::Upload);
            fileTracker.setFileSize(fileSize);
            fileTracker.setCurOperationSize(data.size());
            fileTracker.setChunkSize(chunkTotal);
            fileTracker.setChunkIndex(1);
            _fileTrackerMap[uploadId] = fileTracker;

            //定义输出
            response->set_errorcode(suiErrorCodeDef::SUCCESS);
            response->set_id(serviceId);
            auto rspResult = response->mutable_result();
            rspResult->set_uploadid(uploadId);
            rspResult->set_fileid(fileId);
            return;
        }
        //追加
        //验证操作存在和顺序性
        
        if(_fileTrackerMap.find(uploadId) == _fileTrackerMap.end())
        {
            //说明操作不存在
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_ID_INVALID);
            response->set_id(serviceId);
            response->set_errormsg("上传id失效");
            ERROR("会话id: {} 请求上传文件失败, 错误： {}", sessionId, "上传id失效");
            return;
        }
        //
        auto& tarckerIt = _fileTrackerMap[uploadId];
        if(chunkIndex != tarckerIt.getChunkIndex())
        {
            //索引不对
            ERROR("会话id: {} 请求上传文件失败, 错误： {}, 上传序号：{}， 当前需求序号：{}", sessionId, "上传分片的序号不对", chunkIndex, tarckerIt.getChunkIndex());
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_CHUNK_INDEX_INVALID);
            response->set_id(serviceId);
            response->set_errormsg("上传分片的序号不对");
            auto rspResult = response->mutable_result();
            rspResult->set_curoperationsize(tarckerIt.getChunkIndex());
            return;
        }
        std::string filePath;
        //通过文件id查询文件路径
        auto fileMeta = _fileMetaServicePtr->getFileMeta(fileId);
        if(!fileMeta)
        {
            //文件不存在
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_NOT_FOUND);
            response->set_id(serviceId);
            response->set_errormsg("文件不存在");
            ERROR("会话id: {} 请求上传文件失败, 错误： {}", sessionId, "文件不存在");
            return;
        }
        if(fileMeta->getPath().null())
        {
            //文件路径为空
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_NOT_FOUND);
            response->set_id(serviceId);
            response->set_errormsg("文件不存在");  //统一返回给客户端文件不存在
            ERROR("会话id: {} 请求上传文件失败, 错误： {}", sessionId, "文件路径为空"); //本地记录真实错误
            return;
        }
        filePath = fileMeta->getPath().get();
        //追加文件
        auto uploadResult = suifd::suiFastdfs::upload_appender_from_buff(filePath, data);
        if(uploadResult.has_value())
        {
            //出现错误
            ERROR("会话id: {} 请求上传文件失败, 错误： {}", sessionId, uploadResult.value());
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_FAILED); //上传失败
            response->set_errormsg(uploadResult.value());
            response->set_id(serviceId);
            return;
        }

        //上传成功
        tarckerIt.setChunkIndex(chunkIndex + 1);
        tarckerIt.setCurOperationSize(tarckerIt.getCurOperationSize() + data.size());

        //定义输出
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        response->set_id(serviceId);
        auto rspResult = response->mutable_result();
        rspResult->set_uploadid(uploadId);
        rspResult->set_fileid(fileId);
        rspResult->set_curoperationsize(chunkIndex + 1);
        return;
    }

    void suiFileRpcService::completeFileUpload(::google::protobuf::RpcController* controller,
        const ::suiApi::completeFileUploadReq* request,
        ::suiApi::completeFileUploadRsp* response,
        ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;

        //验证权限
        //获取请求信息
        std::string sessionId = request->sessionid();
        std::string serviceId = request->id();
        //验证权限
        auto curUserId = _fileMetaServicePtr->getSessionUser(sessionId);
        if(!curUserId.has_value())
        {
            //为空
            //用户数据为空，说明临时用户，无权限
            ERROR("会话id: {} 请求上传文件失败, 错误： {}", sessionId, "无上传权限");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_USER_NO_PERMISSION);
            response->set_errormsg("无上传权限");
            response->set_id(serviceId);
            return;
        }
        //获取调用参数
        auto& param = request->param();
        std::string uploadId = param.uploadid();
        std::string fileId = param.fileid();

        //获取文件元数据
        auto fileMeta = _fileMetaServicePtr->getFileMeta(fileId);
        if(fileMeta == nullptr)
        {
            //文件不存在
            ERROR("会话id: {} 请求上传文件失败, 错误： {}", sessionId, "文件不存在");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_NOT_FOUND);
            response->set_errormsg("文件不存在");
            response->set_id(serviceId);
            return;
        }
        if(fileMeta->getFileStatus() != suiDataSql::fileStatus::fileStatusUploading)
        {
            //文件状态不对
            ERROR("会话id: {} 请求上传文件失败, 错误： {}, 当前文件状态是", sessionId, "文件状态不对");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_STATUS_ERROR);
            response->set_errormsg("文件状态不对");
            response->set_id(serviceId);
            return;
        }

        //删除原有的
        _fileMetaServicePtr->deleteFileMeta(fileId);
        //修改文件状态
        fileMeta->setFileStatus(suiDataSql::fileStatus::fileStatusSuccess);
        //新增
        _fileMetaServicePtr->newFileMeta(*fileMeta);

        //定义输出
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        response->set_id(serviceId);
        return;
    }

    void suiFileRpcService::getFileData(::google::protobuf::RpcController* controller,
        const ::suiApi::InitGetFileDataReq* request,
        ::suiApi::InitGetFileDataRsp* response,
        ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;

        //返回元数据
        //无需创建下载句柄，多次申请不会对本地造成缓存积累
        //获取请求参数
        auto& param = request->param();
        const std::string& sessionid = request->sessionid();
        const std::string& serverid = request->id();

        //获取元数据
        auto fileMeta = _fileMetaServicePtr->getFileMeta(param.fileid());
        if(fileMeta == nullptr)
        {
            //文件不存在
            ERROR("会话id: {} 请求获取文件元数据失败, 错误： {}", sessionid, "文件不存在");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_NOT_FOUND);
            response->set_errormsg("文件不存在");
            response->set_id(serverid);
            return;
        }
        size_t fileSize = fileMeta->getSize();
        int32_t downChunkTotal = fileSize / _standardChunkSize + (fileSize % _standardChunkSize != 0 ? 1 : 0);

        //定义输出
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        response->set_id(serverid);
        auto rspResult = response->mutable_result();
        rspResult->set_filesize(fileSize);
        rspResult->set_downchunktotal(downChunkTotal);
        return;
    }

    void suiFileRpcService::downloadFile(::google::protobuf::RpcController* controller,
        const ::suiApi::downloadFileReq* request,
        ::suiApi::downloadFileRsp* response,
        ::google::protobuf::Closure* done)
    {
        brpc::ClosureGuard doneGuard(done);
        (void)controller;

        //直接发回文件流
        //获取调用参数
        std::string serviceId = request->id();
        std::string sessionid = request->sessionid();
        auto& param = request->param();
        std::string fileid = param.fileid();
        int32_t chunkIndex = param.chunkindex();

        //验证权限
        auto curUserId = _fileMetaServicePtr->getSessionUser(sessionid);
        if(!curUserId.has_value())
        {
            //为空
            //用户数据为空，说明临时用户，无权限
            ERROR("会话id: {} 请求上传文件失败, 错误： {}", sessionid, "无上传权限");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_UPLOAD_USER_NO_PERMISSION);
            response->set_errormsg("无上传权限");
            response->set_id(serviceId);
            return;
        }

        //验证成功 获取数据库文件元数据
        auto fileMeta = _fileMetaServicePtr->getFileMeta(fileid);
        if(fileMeta == nullptr)
        {
            //文件不存在
            ERROR("会话id: {} 请求下载文件失败, 错误： {}", sessionid, "文件不存在");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_NOT_FOUND);
            response->set_errormsg("文件不存在");
            response->set_id(serviceId);
            return;
        }
        if(fileMeta->getFileStatus() != suiDataSql::fileStatus::fileStatusSuccess)
        {
            //文件状态不对
            ERROR("会话id: {} 请求下载文件失败, 错误： {}", sessionid, "文件状态不对");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_STATUS_ERROR);
            response->set_errormsg("文件状态不对");
            response->set_id(serviceId);
            return;
        }
        if(fileMeta->getSize() == 0)
        {
            //文件大小为0
            ERROR("会话id: {} 请求下载文件失败, 错误： {}", sessionid, "文件大小为0");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_SIZE_ERROR);
            response->set_errormsg("文件大小为0");
            response->set_id(serviceId);
            return;
        }
        if(fileMeta->getPath().null())
        {
            //路径不存在
            ERROR("会话id: {} 请求下载文件失败, 错误： {}", sessionid, "路径不存在");
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_FILE_NOT_FOUND);
            response->set_errormsg("文件不存在");
            response->set_id(serviceId);
            return;
        }

        //计算下载块起始位置和偏移量
        size_t downChunkStart = chunkIndex * _standardChunkSize;
        //下载数据
        std::string downChunkData;
        auto downret = suifd::suiFastdfs::download_chunk_to_buff(fileMeta->getPath().get(), downChunkStart, _standardChunkSize, downChunkData);
        if(downret.has_value())
        {
            //存在错误
            ERROR("会话id: {} 请求下载文件失败, 错误： {}", sessionid, downret.value());
            response->set_errorcode(suiErrorCodeDef::ERR_FILE_SERVICE_DOWNLOAD_FAILED);
            response->set_errormsg("下载失败");
            response->set_id(serviceId);
            return;
        }
        INFO("下载块大小: {}", downChunkData.size());
        //验证成功 转发文件信息
        response->set_errorcode(suiErrorCodeDef::SUCCESS);
        response->set_id(serviceId);
        auto rspResult = response->mutable_result();
        rspResult->set_chunkindex(chunkIndex);
        rspResult->set_chunkdata(downChunkData);
        rspResult->set_chunkmd5(::suiHash::hashQperation::calculateMD5(downChunkData));
        return;
    }

    //

    void suiFileRpcService::createFileId(std::string& fileid)
    {
        fileid = suiRandom::RandomUtil::uuid(suiRandom::RandomUtil::UuidType::ALL);
    }

    bool suiFileRpcService::dataValidation(const std::string& data, const std::string& Md5)
    {
        //把data转换成md5
        std::string dataMd5 = ::suiHash::hashQperation::calculateMD5(data);
        return dataMd5 == Md5;
    }

    void suiFileRpcService::createUploadId(const std::string& Md5, std::string& uploadId)
    {
        uploadId = Md5;
        uploadId += "_" + suiRandom::RandomUtil::uuid(suiRandom::RandomUtil::UuidType::ALL, 10);
    }

    void suiFileRpcService::createDownloadId(const std::string& Md5, std::string& downloadId)
    {
        return createUploadId(Md5, downloadId);
    }

    size_t suiFileRpcService::_standardChunkSize = 1024 * 1024 * 1024; //默认单次下载大小为1GB

}