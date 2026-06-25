#include "fileServiceData.hpp"

namespace suiFileService 
{
    fileMetaService::fileMetaService(const suiOdb::odbSetting &ms,
                const suiRedis::redisSettings &rs, suiQueue::MQClient::ptr mq)
    {
        _redis = suiRedis::RedisFactory::create(rs);
        _db = suiOdb::dbFactory::create(ms);
        _removeCache = std::make_shared<suiRemoveCache::RemoveCache>(_redis, mq);
    }

    std::optional<std::string> fileMetaService::getSessionUser(const std::string& session_id)
    {
        try
        {
            odb::transaction t(_db->begin());
            auto& dbHandler = t.database();
            auto rtx = _redis->transaction(false, false);
            auto rehandler = rtx.redis();
            suiSession::sessionData sedata(dbHandler, rehandler, _removeCache);
            auto session = sedata.selectBySessionId(session_id);
            //INFO("查询的会话id: {}", session_id);
            t.commit();
            if (session == nullptr)
            {
                ERROR("会话id: {} 不存在", session_id);
                return std::nullopt; //为空或者用户id为空直接返回
            }
            if(session->getUserId().null())
            {
                ERROR("会话id: {} 用户id为空", session_id);
                return std::nullopt; //为空或者用户id为空直接返回   
            }
            return session->getUserId().get();
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获取用户id： {}， 异常： {}", session_id, e.what());
            return std::nullopt;
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库获取用户id： {}， 异常： {}", session_id, e.what());
            return std::nullopt;
        }
        catch (...)
        {
            ERROR("获取用户id： {}， 未知异常", session_id);
            return std::nullopt;
        }
    }

    void fileMetaService::newFileMeta(suiDataSql::suiFileMeta& meta)
    {
        try
        {
            odb::transaction t(_db->begin());
            auto& dbHandler = t.database();
            suiFile::FileData sedata(dbHandler);
            sedata.insert(meta);
            t.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库创建文件元数据异常， 错误： {}", e.what());
            return;
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库创建文件元数据异常， 错误： {}", e.what());
            return;
        }
        catch (...)
        {
            ERROR("未知异常");
            return;
        }
    }

    void fileMetaService::deleteFileMeta(const std::string& file_id)
    {
        try
        {
            odb::transaction t(_db->begin());
            auto& dbHandler = t.database();
            suiFile::FileData sedata(dbHandler);
            sedata.removeByFileId(file_id);
            t.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库删除文件元数据异常，文件id： {}， 错误： {}", file_id, e.what());
            return;
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库删除文件元数据异常，文件id： {}， 错误： {}", file_id, e.what());
            return;
        }
        catch (...)
        {
            ERROR("删除文件元数据异常，文件id： {}， 未知异常", file_id);
            return;
        }
    }


    suiDataSql::suiFileMeta::ptr fileMetaService::getFileMeta(const std::string& file_id)
    {
        try
        {
            odb::transaction t(_db->begin());
            auto& dbHandler = t.database();
            suiFile::FileData sedata(dbHandler);
            auto fileMeta = sedata.selectFileByFileId(file_id);
            t.commit();
            return fileMeta;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库获取元数据异常，文件id： {}， 错误： {}", file_id, e.what());
            return nullptr;
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("redis数据库获取元数据异常，文件id： {}， 错误： {}", file_id, e.what());
            return nullptr;
        }
        catch (...)
        {
            ERROR("获取元数据异常，文件id： {}， 未知异常", file_id);
            return nullptr;
        }
    }

}