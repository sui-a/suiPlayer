#include "fileServicemq.hpp"

namespace suiFileService
{
    fileRemoveMq::fileRemoveMq(fileMetaService::ptr fileService, suiQueue::MQClient::ptr mq, const suiQueue::queueSetting& queueSetting)
        :_fileMetaService(fileService)
    {
        //创建消息订阅对象
        _subscriber = std::make_shared<suiQueue::suiSubscriber>(mq, queueSetting);
        //开始绑定处理
        _subscriber->consume(std::bind(&suiFileService::fileRemoveMq::callback, this, std::placeholders::_1));

        //fdfs服务全局都可能用，放在类外初始化
    }

    bool fileRemoveMq::callback(std::string body)
    {
        //解析消息
        suiApi::DeleteFileMsg msg;
        std::vector<std::string> pathList;
        bool ret = msg.ParseFromString(body);
        if(!ret) {
            ERROR("收到缓存同步消息，但反序列化失败");
            return true;  //反序列化失败，但返回true 目的是不中断消息队列的消费 无效消息丢弃即可
        }
        int sz = msg.fileid_size();
        INFO("收到删除文件消息，共{}个file", sz);
        //开始删除元数据
        for(int i = 0; i < sz; i++) 
        {
            INFO("删除文件id: {}", msg.fileid(i));
            auto curMate = _fileMetaService->getFileMeta(msg.fileid(i));
            if(!curMate) 
            {
                ERROR("文件id: {} 不存在于数据库中", msg.fileid(i));
                continue;
            }
            std::string fid = msg.fileid(i);
            suifd::suiFastdfs::delete_file(fid); //删除文件
            _fileMetaService->deleteFileMeta(fid); //删除元数据
        }
        INFO("删除文件消息处理完成");
        return true;
    }




}