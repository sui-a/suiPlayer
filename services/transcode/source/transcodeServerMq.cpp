#include "transcodeServerMq.hpp"

namespace suiTranscodeServer
{
    transcodeServerMq::transcodeServerMq(suiQueue::MQClient::ptr clientPtr, const std::string& tempFilePath, transcodeServerData::ptr dataHandle
            , suiQueue::suiSubscriber::ptr subscribeQueue, std::shared_ptr<suiPeg::HLSTranscoder> transcoder
            , const std::string& requestPrefix, suiCacheSync::CacheSyncClient::ptr fileRemoveSyncClient)
        : _clientPtr(clientPtr)
        , _temp_file_path(tempFilePath)
        , _dataHandle(dataHandle)
        , _subscribeQueue(subscribeQueue)
        , _transcoder(transcoder)
        , _requestPrefix(requestPrefix)
        , _fileRemoveSyncClient(fileRemoveSyncClient)
    {
        _subscribeQueue->consume(std::bind(&transcodeServerMq::callback, this, std::placeholders::_1));
    }

    void transcodeServerMq::start()
    {
        _clientPtr->wait();
    }

    bool transcodeServerMq::callback(const std::string& videoid)
    {
        if(videoid.empty())
            return true; //为空返回处理成功，不影响后续
        int tryCount = 3;
        while(tryCount--)
        {
            //获取视频元信息
            suiDataSql::suiVideoMeta::ptr videoMeta = nullptr;
            if(_dataHandle->selectVideo(videoid, videoMeta) == false)
            {
                //说明出现错误，或者网络波动，导致没获取到视频元信息，休眠一会继续执行到尝试次数结束
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue; 
            }
            if(videoMeta == nullptr)
                return true; //说明数据库没有视频元信息，返回处理成功，不影响后续
            //元信息获取成功
            std::string VideofileId = videoMeta->getVideoFileId();
            if(VideofileId.empty())
                return true;
            //下载文件
            std::string curFilePath = _temp_file_path + "/" + VideofileId;
            if(ensureDir(curFilePath) == false)
            {
                INFO("创建目录失败");
                continue;
            }
            if(suifd::suiFastdfs::download_to_file(curFilePath, VideofileId).has_value())
            {
                INFO("文件下载失败，错误为: {}", suifd::suiFastdfs::download_to_file(curFilePath, VideofileId).value());
                deletePath(curFilePath);
                continue;
            }
            //进行转码
            std::string m3u8FilePath = curFilePath + ".m3u8";
            if(_transcoder->transcode(curFilePath, m3u8FilePath) == false)
            {
                INFO("转码失败");
                deletePath(curFilePath);
                continue;
            }
            //成功转码，修改文件
            suiPeg::M3U8Info m3u8Info(m3u8FilePath);
            if(m3u8Info.parse() == false)
            {
                //解析失败
                deletePath(m3u8FilePath);
                continue;
            }
            auto& pieces = m3u8Info.pieces();
            for(auto& piece : pieces)
            {
                std::string cur_path = curFilePath + "/" + piece.second;
                //进行上传
                std::string newfilePath;
                std::string newfileId = ::suiRandom::RandomUtil::uuid(suiRandom::RandomUtil::UuidType::ALL);
                {
                    auto ret = suifd::suiFastdfs::upload_from_file(cur_path, newfilePath);
                    if(ret.has_value())
                    {
                        INFO("文件上传失败，错误为: {}, 路径为: {}", ret.value(), cur_path);
                        deletePath(curFilePath);
                        return false;
                    }
                    piece.second = _requestPrefix + newfileId;
                }
                //新增文件信息
                suiDataSql::suiFileMeta fileMeta;
                fileMeta.setFileId(newfileId);
                fileMeta.setPath(newfilePath);
                fileMeta.setSize(std::filesystem::file_size(cur_path));
                fileMeta.setMimeType("video/MP2T");
                fileMeta.setUploadUserId(videoMeta->getUploadUserId());
                fileMeta.setUploadTime();
                fileMeta.setFileStatus(suiDataSql::fileStatus::fileStatusSuccess);
                _dataHandle->insertFile(fileMeta);
            }
            //此后由于可能上传部分文件，所以出现错误交给人工处理
            //写入文件
            if(m3u8Info.write() == false)
            {
                INFO("写入文件失败"); //说明系统内存等有问题，不continue
                return true;
            }
            //上传info文件
            std::string infofileId = ::suiRandom::RandomUtil::uuid(suiRandom::RandomUtil::UuidType::ALL);
            std::string infofilepath;
            auto ret = suifd::suiFastdfs::upload_from_file(m3u8FilePath, infofilepath);
            if(ret.has_value())
            {
                INFO("文件上传失败，错误为: {}, 路径为: {}", ret.value(), m3u8FilePath);
                return true;
            }
            {
                //添加文件元信息
                suiDataSql::suiFileMeta fileMeta;
                fileMeta.setFileId(infofileId);
                fileMeta.setPath(infofilepath);
                fileMeta.setSize(std::filesystem::file_size(m3u8FilePath));
                fileMeta.setMimeType("video/MP2T");
                fileMeta.setUploadUserId(videoMeta->getUploadUserId());
                fileMeta.setUploadTime();
                fileMeta.setFileStatus(suiDataSql::fileStatus::fileStatusSuccess);
                _dataHandle->insertFile(fileMeta);
            }
            //更新视频元信息
            videoMeta->setVideoFileId(infofileId);
            //切换待审核状态
            videoMeta->setVideoStatus(suiDataSql::videoStatus::videoStatusPendingReview);
            //更新
            _dataHandle->updateVideo(videoMeta);
            //删除原文件
            _fileRemoveSyncClient->syncCache(VideofileId);
            //删除临时文件
            deletePath(curFilePath);
            return true;
        }
        return false;
    }

    void transcodeServerMq::deletePath(const std::string& pathStr)
    {
        std::error_code ec;
        std::filesystem::path p(pathStr);

        // 路径不存在，直接返回成功（什么都不做，不报错）
        if (!std::filesystem::exists(p, ec)) {
            return;
        }

        // 递归删除（目录及里面所有内容都会删掉）
        std::filesystem::remove_all(p, ec);
        return;
    }

    bool transcodeServerMq::ensureDir(const std::string& filepath) 
    {
        std::error_code ec;
        std::filesystem::path p(filepath);
        std::filesystem::create_directories(p.parent_path(), ec);
        return !ec;
    }
}
