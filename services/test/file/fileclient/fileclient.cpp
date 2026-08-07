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
#include "file.pb.h"

//etcd地址
DEFINE_string(registry_center, "127.0.0.1:8084", "注册中心地址");
DEFINE_string(file_server_name, "file_server", "file_server服务名称");

int main(int argc, char* argv[])
{
    //初始化日志模块
    suiUtil::suiLogInitDefault();
    //初始化gflags
    google::ParseCommandLineFlags(&argc, &argv, true);
    //创建服务搜索
    suiRpc::Channels::Ptr _channels = std::make_shared<suiRpc::Channels>(FLAGS_file_server_name);
    suiEtcd::serSearch::ptr _search = std::make_shared<suiEtcd::serSearch>(FLAGS_file_server_name, FLAGS_registry_center
        , [_channels](std::string serName, std::string addr){
        _channels->insert(addr);
        INFO("添加节点: {}", addr);
    }, [_channels](std::string serName, std::string addr){
        INFO("删除节点: {}", addr);
        _channels->remove(addr);
    });

    //开始服务发现
    _search->search();

    //存储图像数据
    std::string fileImageContent;
    //存放图像id
    std::string imageFileId;
    
    {
        //上传图片
        {
            std::string filePath = "./testfile/texas_head.jpg";
            std::ifstream file(filePath, std::ios::binary | std::ios::ate);
            if(!file.is_open()) {
                ERROR("打开文件失败, 文件路径为: {}", filePath);
                return -1;
            }
            
            // 获取文件大小
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);  // 回到文件开头
            
            // 调整string大小
            fileImageContent.resize(size);
            
            // 读取文件内容
            if (!file.read(fileImageContent.data(), size)) {
                ERROR("读取文件失败, 文件路径为: {}", filePath);
                return -1;
            }
            
            file.close();
            INFO("文件读取成功，大小: {} bytes", size);
        }
        
        //获取rpc通道
        suiRpc::ChannelPtr curChannel;
        while(1)
        {
            curChannel = _channels->select();   
            if(curChannel)
                break;
            INFO("等待服务上线");
            sleep(5);
        }
        //发起实例化服务对象
        suiApi::fileServices_Stub stub(&(*curChannel));
        brpc::Controller* cntl = new brpc::Controller();
        cntl->set_timeout_ms(-1);
        //定义调用参数
        suiApi::uploadImageReq req;
        req.set_id("123");
        req.set_sessionid("session_id_1");
        req.mutable_fileinfo()->set_filesize(fileImageContent.size());
        req.mutable_fileinfo()->set_filemime("image/jpeg");
        req.mutable_fileinfo()->set_filedata(fileImageContent);
        req.mutable_fileinfo()->set_usrid("sui");

        suiApi::uploadImageRsp rsp;
        stub.uploadImage(cntl, &req, &rsp, nullptr);
        if(cntl->Failed())
        {
            //调用失败
            ERROR("stub请求失败: {}", cntl->ErrorText());
            return -1;
        }
        if(rsp.errorcode() != 0)
        {
            //出现错误
            ERROR("上传图片失败, 错误为: {}", rsp.errormsg());
            return -1;
        }
        //成功
        INFO("上传图片成功, 图片id为: {}", rsp.result().fileid());
        imageFileId = rsp.result().fileid();
    }
    INFO("按下回车开始下一步");
    std::cin.get();
    INFO("开始下载头像");
    {
        //开始下载头像
        //获取rpc通道
        suiRpc::ChannelPtr curChannel;
        while(1)
        {
            curChannel = _channels->select();   
            if(curChannel)
                break;
            INFO("等待服务上线");
            sleep(5);
        }

        //获得通道，开始准备下载
        suiApi::fileServices_Stub stub(&(*curChannel));
        brpc::Controller* cntl = new brpc::Controller();
        cntl->set_timeout_ms(-1);
        //定义调用参数
        suiApi::downloadImageReq req;
        req.set_id("123");
        req.set_sessionid("session_id_1");
        req.set_fileid(imageFileId);
        suiApi::downloadImageRsp rsp;

        //开始调用
        stub.downloadImage(cntl, &req, &rsp, nullptr);
        if(cntl->Failed())
        {
            //调用失败
            ERROR("stub请求失败: {}", cntl->ErrorText());
            return -1;
        }
        if(rsp.errorcode() != 0)
        {
            //出现错误
            ERROR("下载图片失败, 错误为: {}", rsp.errormsg());
            return -1;
        }
        //开始使用md5验证图片的准确性
        std::string curfileData = rsp.fileinfo().filedata();
        std::string one = suiHash::hashQperation::calculateMD5(fileImageContent);
        std::string two = suiHash::hashQperation::calculateMD5(curfileData);
        if(one != two)
        {
            //说明下载的喝上传的值不一样
            ERROR("下载的图片和上传的图片不一致");
            return -1;
        }
        INFO("下载图片成功");
    }
    INFO("按下回车开始下一步");
    std::cin.get();
    INFO("开始分片上传文件");
    //存储文本文件内容
    std::string textFileContent;
    std::string textFileId;
    {
        //开始读取文本文件
        {
            std::string filePath = "./testfile/test.txt";
            std::ifstream file(filePath, std::ios::binary | std::ios::ate);
            if(!file.is_open()) {
                ERROR("打开文件失败, 文件路径为: {}", filePath);
                return -1;
            }
            
            // 获取文件大小
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);  // 回到文件开头
            
            // 调整string大小
            textFileContent.resize(size);
            
            // 读取文件内容
            if (!file.read(textFileContent.data(), size)) {
                ERROR("读取文件失败, 文件路径为: {}", filePath);
                return -1;
            }
            
            file.close();
            INFO("文件读取成功，大小: {} bytes", size);
        }

        //开始上传
        //分批上传，无论文件大小如何，都分批上传，一共分五次
        //计算一次的大小
        INFO("开始分批上传");
        int chunkSize = textFileContent.size() / 5;
        std::string uploadId;
        std::string fileId;
        int count = 0;
        for(int i = 0; i < textFileContent.size();)
        {
            //计算当前的大小
            int curSize = i + chunkSize;
            if(curSize > textFileContent.size())
                curSize = textFileContent.size(); //避免越界

            //构建此时上传的内容
            std::string curtextFileContent = textFileContent.substr(i, curSize - i);
            i = curSize;

            //开始获取rpc连接
            suiRpc::ChannelPtr curChannel;
            while(1)
            {
                curChannel = _channels->select();   
                if(curChannel)
                    break;
                INFO("等待服务上线");
                sleep(5);
            }

            //创建stub
            suiApi::fileServices_Stub stub(&(*curChannel));
            brpc::Controller* cntl = new brpc::Controller();
            cntl->set_timeout_ms(-1);

            //开始构建请求参数
            suiApi::uploadFileReq req;
            req.set_id("123");
            req.set_sessionid("session_id_1");
            req.mutable_param()->set_fileid(fileId);
            req.mutable_param()->set_uploadid(uploadId);
            req.mutable_param()->set_chunkindex(count);
            req.mutable_param()->set_chunktotal(5);
            req.mutable_param()->set_filesize(textFileContent.size());
            req.mutable_param()->set_filemime("text");
            req.mutable_param()->set_chunkdata(curtextFileContent);
            req.mutable_param()->set_chunkmd5(suiHash::hashQperation::calculateMD5(curtextFileContent));
            
            suiApi::uploadFileRsp rsp;
            //开始调用
            stub.uploadFile(cntl, &req, &rsp, nullptr);
            if(cntl->Failed())
            {
                //调用失败
                ERROR("stub请求失败: {}", cntl->ErrorText());
                return -1;
            }
            if(rsp.errorcode() != 0)
            {
                //出现错误
                ERROR("上传文件失败, 错误为: {}", rsp.errormsg());
                if(rsp.result().has_curoperationsize())
                {
                    //说明上传的分片大小有错误
                    INFO("上传的分片大小有错误，要求上传的分片是: {}, 当前上传的分片是: {}", rsp.result().curoperationsize(), count);
                }
                return -1;
            }
            INFO("上传分片成功，分片序号： {}, 文件id: {}", count, rsp.result().fileid());
            //开始复制
            fileId = rsp.result().fileid();
            uploadId = rsp.result().uploadid();
            count++;
        }
        INFO("上传分片完成, 开始申请完成上传");
        //开始获取rpc连接
        suiRpc::ChannelPtr curChannel;
        while(1)
        {
            curChannel = _channels->select();   
            if(curChannel)
                break;
            INFO("等待服务上线");
            sleep(5);
        }

        //创建stub
        suiApi::fileServices_Stub stub(&(*curChannel));
        brpc::Controller* cntl = new brpc::Controller();
        cntl->set_timeout_ms(-1);

        //开始构建请求参数
        suiApi::completeFileUploadReq req;
        req.set_id("123");
        req.set_sessionid("session_id_1");
        req.mutable_param()->set_fileid(fileId);
        req.mutable_param()->set_uploadid(uploadId);

        suiApi::completeFileUploadRsp rsp;
        //开始调用
        stub.completeFileUpload(cntl, &req, &rsp, nullptr);
        if(cntl->Failed())
        {
            //调用失败
            ERROR("stub请求失败: {}", cntl->ErrorText());
            return -1;
        }
        if(rsp.errorcode() != 0)
        {
            //出现错误
            ERROR("上传文件完成请求, 错误为: {}", rsp.errormsg());
            return -1;
        }

        INFO("分片上传文件成功");
        textFileId = fileId;
    }
    INFO("按下回车开始下一步");
    std::cin.get();
    INFO("开始下载文件");
    {
        int downCount;
        int64_t downSize;
        //获取文件元信息
        {
            //开始获取rpc连接
            suiRpc::ChannelPtr curChannel;
            while(1)
            {
                curChannel = _channels->select();   
                if(curChannel)
                    break;
                INFO("等待服务上线");
                sleep(5);
            }

            //创建stub
            suiApi::fileServices_Stub stub(&(*curChannel));
            brpc::Controller* cntl = new brpc::Controller();
            cntl->set_timeout_ms(-1);

            //开始创建请求参数
            suiApi::InitGetFileDataReq req;
            req.set_id("123");
            req.set_sessionid("session_id_1");
            req.mutable_param()->set_fileid(textFileId);
            suiApi::InitGetFileDataRsp rsp;
            stub.getFileData(cntl, &req, &rsp, nullptr);
            if(cntl->Failed())
            {
                //调用失败
                ERROR("stub请求失败: {}", cntl->ErrorText());
                return -1;
            }
            if(rsp.errorcode() != 0)
            {
                //出现错误
                ERROR("获取文件元信息失败, 错误为: {}", rsp.errormsg());
                return -1;
            }
            INFO("获取文件元信息成功，文件大小: {}, 分片总数: {}", rsp.result().filesize(), rsp.result().downchunktotal());
            downCount = rsp.result().downchunktotal();
            downSize = rsp.result().filesize();
        }

        std::string curTextFileContents;
        INFO("开始正式下载文件");
        {
            for(int i = 0; i < downCount; i++)
            {
                //获取rpc连接
                suiRpc::ChannelPtr curChannel;
                while(1)
                {
                    curChannel = _channels->select();   
                    if(curChannel)
                        break;
                    INFO("等待服务上线");
                    sleep(5);
                }

                //创建stub
                suiApi::fileServices_Stub stub(&(*curChannel));
                brpc::Controller* cntl = new brpc::Controller();
                cntl->set_timeout_ms(-1);

                //构建消息
                suiApi::downloadFileReq req;
                req.set_id("123");
                req.set_sessionid("session_id_1");
                req.mutable_param()->set_fileid(textFileId);
                req.mutable_param()->set_chunkindex(i);
                suiApi::downloadFileRsp rsp;
                stub.downloadFile(cntl, &req, &rsp, nullptr);
                if(cntl->Failed())
                {
                    //调用失败
                    ERROR("stub请求失败: {}", cntl->ErrorText());
                    return -1;
                }
                if(rsp.errorcode() != 0)
                {
                    //出现错误
                    ERROR("下载文件失败, 错误为: {}", rsp.errormsg());
                    return -1;
                }
                curTextFileContents += rsp.result().chunkdata();
                INFO("下载分片成功，分片序号： {}", i);
            }
            if(curTextFileContents.size() != downSize)
            {
                ERROR("下载文件失败，文件大小不一致, 下载大小: {}, 期望大小: {}", curTextFileContents.size(), downSize);
                return -1;
            }
            else if(curTextFileContents != textFileContent)
            {
                ERROR("下载文件失败，文件内容不一致");
                return -1;
            }
            INFO("下载文件成功");
            //
        }

    }
    INFO("按下回车退出");
    std::cin.get();
    return 0;
}