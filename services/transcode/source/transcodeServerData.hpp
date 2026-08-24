#pragma once
#include <suiScaffold/log.h>
#include "file.hpp"
#include "video_operation_meta.hpp"

namespace suiTranscodeServer
{
    class transcodeServerData
    {
    public:
        using ptr = std::shared_ptr<transcodeServerData>;
        transcodeServerData(std::shared_ptr<odb::database> mysql);

        //文件操作
        void insertFile(suiDataSql::suiFileMeta& fileMeta);
        bool selectFile(const std::string fileid, suiDataSql::suiFileMeta::ptr& out);
        bool updateFile(suiDataSql::suiFileMeta::ptr& fileMeta);

        //视频操作
        bool selectVideo(const std::string videoId, suiDataSql::suiVideoMeta::ptr& out);
        bool updateVideo(suiDataSql::suiVideoMeta::ptr& videoMeta);

    private:
        //操作句柄
        std::shared_ptr<odb::database> _mysql;
    };





}