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

    bool fileRemoveMq::callback(std::string msg)
    {
        suiApi::DeleteCacheMsg body;
        bool ret = body.ParseFromString(msg);
        if(!ret)
        {
            INFO("解析消息失败, 消息内容: {}", msg);
            return true;
        }
        //删除文件元信息，并获取路径
        auto fileMeta = _fileMetaService->getFileMeta(body.key(0));
        INFO("删除文件{}的元信息", body.key(0));
        if(fileMeta != nullptr)
        {
            std::string filePath = fileMeta->getPath().get();
            //删除fdfs里的文件
            suifd::suiFastdfs::delete_file(filePath);
            INFO("删除文件{}完成, 路径: {}", body.key(0), filePath);
        }
        else
        {
            INFO("删除文件{}失败, 未找到文件元信息", body.key(0));
        }
        
        return true;
    }




}