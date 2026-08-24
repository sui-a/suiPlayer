#pragma once
#include <suiScaffold/suiFastdfs.hpp>
#include "fileServiceData.hpp"

namespace suiFileService
{
    class fileRemoveMq
    {
    public:
        //根据发布请求异步删除文件元信息以及上传的文件（消息处理）
        using ptr = std::shared_ptr<fileRemoveMq>;
        fileRemoveMq(fileMetaService::ptr fileService, suiQueue::MQClient::ptr mq, const suiQueue::queueSetting& queueSetting);

    private:
        bool callback(std::string msg);

    private:
        //文件元信息控制对象
        fileMetaService::ptr _fileMetaService;
        //消息订阅对象
        suiQueue::suiSubscriber::ptr _subscriber;

    };
}