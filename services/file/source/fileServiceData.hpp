#pragma once
#include <optional>
#include <memory>
#include <string>
#include "file.hpp"
#include "removeCache.hpp"
#include "session.hpp"

namespace suiFileService {
    class fileMetaService 
    {
    public:
        using ptr = std::shared_ptr<fileMetaService>;
        fileMetaService(const suiOdb::odbSetting &ms,
                const suiRedis::redisSettings &rs, suiQueue::MQClient::ptr mq);

        //根据session_id获取用户id
        std::optional<std::string> getSessionUser(const std::string& session_id);
        //文件元信息
        void newFileMeta(suiDataSql::suiFileMeta& meta);
        void deleteFileMeta(const std::string& file_id);
        suiDataSql::suiFileMeta::ptr getFileMeta(const std::string& file_id);

    private:
            

    private:
        std::shared_ptr<sw::redis::Redis> _redis;
        std::shared_ptr<odb::database> _db;
        suiRemoveCache::RemoveCache::ptr _removeCache;
    };

}