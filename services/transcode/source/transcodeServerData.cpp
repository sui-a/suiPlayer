#include "transcodeServerData.hpp"

namespace suiTranscodeServer
{
    transcodeServerData::transcodeServerData(std::shared_ptr<odb::database> mysql)
        : _mysql(mysql)
    {

    }

    void transcodeServerData::insertFile(suiDataSql::suiFileMeta& fileMeta)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto& dbHandler = tx.database();
            {
                suiFile::FileData fileHandle(dbHandler);
                fileHandle.insert(fileMeta);
            }
            tx.commit();
            return;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        return;
    }

    bool transcodeServerData::selectFile(const std::string fileid, suiDataSql::suiFileMeta::ptr& out)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto& dbHandler = tx.database();
            {
                suiFile::FileData fileHandle(dbHandler);
                out = fileHandle.selectFileByFileId(fileid);
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        return false;
    }

    bool transcodeServerData::updateFile(suiDataSql::suiFileMeta::ptr& fileMeta)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto& dbHandler = tx.database();
            {
                suiFile::FileData fileHandle(dbHandler);
                fileHandle.update(*fileMeta);
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        return false;
    }

    bool transcodeServerData::selectVideo(const std::string videoId, suiDataSql::suiVideoMeta::ptr& out)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto& dbHandler = tx.database();
            {
                suiVideoOperation::videoOperationDb videoHandle(dbHandler);
                out = videoHandle.select(videoId);
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        return false;
    }

    bool transcodeServerData::updateVideo(suiDataSql::suiVideoMeta::ptr& videoMeta)
    {
        try
        {
            odb::transaction tx(_mysql->begin());
            auto& dbHandler = tx.database();
            {
                suiVideoOperation::videoOperationDb videoHandle(dbHandler);
                videoHandle.update(*videoMeta);
            }
            tx.commit();
            return true;
        }
        catch (const odb::exception& e)
        {
            // 捕获 ODB 数据库异常
            ERROR("odb数据库异常： {}", e.what());
        }
        catch (const sw::redis::Error& e) 
        {
            // 捕获 Redis 异常
            ERROR("缓存库异常： {}", e.what());
        }
        catch (...)
        {
            ERROR("未知异常");
        }
        return false;
    }

}