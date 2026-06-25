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

//消息队列地址
DEFINE_string(file_server_amqp_addr, "amqp://sui:suisuipingan@localhost:8082//", "amqp地址");

//文件删除队列配置
DEFINE_string(file_server_remove_exchange, "file_remove_exchange", "文件删除交换机名称");
DEFINE_string(file_server_remove_exchange_type, "direct", "文件删除队列交换机类型");
DEFINE_string(file_server_remove_queue, "file_remove_queue", "文件删除队列名称");
DEFINE_string(file_server_remove_bind_key, "file_remove_key", "文件删除队列绑定键");

//etcd地址
DEFINE_string(file_server_registry_center, "127.0.0.1:8084", "file_server注册中心地址");
DEFINE_string(file_server_name, "file_server", "file_server服务名称");

int main(int argc, char* argv[])
{
    //初始化gflags
    google::ParseCommandLineFlags(&argc, &argv, true);
    //初始化日志模块
    suiUtil::suiLogInitDefault();
    //创建clientMq
    suiQueue::MQClient::ptr _mqClienrt = std::make_shared<suiQueue::MQClient>(FLAGS_file_server_amqp_addr);
    //创建发布者
    suiQueue::queueSetting pubset;
    pubset.exchange = FLAGS_file_server_remove_exchange;
    pubset.exchangeType = FLAGS_file_server_remove_exchange_type;
    pubset.queue = FLAGS_file_server_remove_queue;
    pubset.bindKey = FLAGS_file_server_remove_bind_key;
    suiQueue::suiPublisher::ptr _publisher = std::make_shared<suiQueue::suiPublisher>(_mqClienrt, pubset);
    //创建服务搜索
    suiRpc::Channels::Ptr _channels = std::make_shared<suiRpc::Channels>(FLAGS_file_server_name);
    suiEtcd::serSearch::ptr _search = std::make_shared<suiEtcd::serSearch>(FLAGS_file_server_name, FLAGS_file_server_registry_center
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
    {
        //开始读取文件







    }

    INFO("按下回车退出");
    std::cin.get();
    return 0;
}