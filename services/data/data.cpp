#include "data.hpp"
#include <chrono>

namespace suiDataSql
{

    /*session ===============================================================================================*/
    suiSessionMeta::suiSessionMeta(const std::string& session_id, const std::string& user_id)
        : _session_id(session_id)
        , _user_id(user_id)
    {

    }
    suiSessionMeta::suiSessionMeta(const std::string& session_id)
        : _session_id(session_id)
    {

    }
    suiSessionMeta::~suiSessionMeta()
    {

    }

    unsigned long long suiSessionMeta::getPrimaryKey() const
    {
        return _primaryKey;
    }
    std::string suiSessionMeta::getSessionId() const
    {
        return _session_id;
    }
    odb::nullable<std::string> suiSessionMeta::getUserId() const
    {
        return _user_id;
    }
    std::string suiSessionMeta::getUploadTimeString() const
    {
        if (_upload_time == 0) return "";

        time_t time_val = static_cast<time_t>(_upload_time);
        struct tm *tm_info = localtime(&time_val);
        if (!tm_info)
        {
            ERROR("时间解析失败");
            return "";
        }

        //开始构造
        std::stringstream ss;

        ss << tm_info->tm_year + 1900 << "-" << tm_info->tm_mon + 1 << "-" << tm_info->tm_mday << ' ';
        ss << tm_info->tm_hour << ':' << tm_info->tm_min << ':' << tm_info->tm_sec;
        return ss.str();
    }

    void suiSessionMeta::setUploadTime()
    {
        _upload_time = static_cast<std::uint64_t>(std::time(nullptr));
    }
    
    std::uint64_t suiSessionMeta::getUploadTime() const
    {
        return _upload_time;
    }

    void suiSessionMeta::setSessionId(const std::string& session_id)
    {
        _session_id = session_id;
    }
    void suiSessionMeta::setUserId(odb::nullable<std::string> user_id)
    {
        _user_id = user_id;
    }
    /*session end ===============================================================================================*/

    /*file ===============================================================================================*/
    suiFileMeta::suiFileMeta(const std::string& file_id, const std::string& _upload_user_id)
        : _file_id(file_id)
        , _upload_user_id(_upload_user_id)
        , _size(-1)
    {

    }
    suiFileMeta::suiFileMeta(const std::string& file_id
        , const std::string& _upload_user_id, const std::string& path
        , size_t size, const std::string& mime_type)
        : _file_id(file_id)
        , _upload_user_id(_upload_user_id)
        , _path(path)
        , _size(size)
        , _mime_type(mime_type)
    {
        
    }
    suiFileMeta::suiFileMeta(const std::string& file_id)
        : _file_id(file_id)
        , _size(-1)
    {
        
    }
    suiFileMeta::suiFileMeta()
        : _size(-1)
    {

    }
    
    suiFileMeta::~suiFileMeta()
    {
        
    }

    std::string suiFileMeta::getFileId() const
    {
        return _file_id;
    }
    std::string suiFileMeta::getUploadUserId() const
    {
        return _upload_user_id;
    }
    odb::nullable<std::string> suiFileMeta::getPath() const
    {
        return _path;
    }
    odb::nullable<size_t> suiFileMeta::getSize() const
    {
        return _size;
    }
    odb::nullable<std::string> suiFileMeta::getMimeType() const
    {
        return _mime_type;
    }

    std::string suiFileMeta::getUploadTimeString() const
    {
        if (_upload_time == 0) return "";

        time_t time_val = static_cast<time_t>(_upload_time);
        struct tm *tm_info = localtime(&time_val);
        if (!tm_info)
        {
            ERROR("时间解析失败");
            return "";
        }

        //开始构造
        std::stringstream ss;

        ss << tm_info->tm_year + 1900 << "-" << tm_info->tm_mon + 1 << "-" << tm_info->tm_mday << ' ';
        ss << tm_info->tm_hour << ':' << tm_info->tm_min << ':' << tm_info->tm_sec;
        return ss.str();
    }
    std::uint64_t suiFileMeta::getUploadTime() const
    {
        return _upload_time;
    }
    
    void suiFileMeta::setFileId(const std::string& file_id)
    {
        _file_id = file_id;
    }
    void suiFileMeta::setPath(const std::string& path)
    {
        _path = path;
    }
    void suiFileMeta::setSize(size_t size)
    {
        _size = size;
    }
    void suiFileMeta::setMimeType(const std::string& mime_type)
    {
        _mime_type = mime_type;
    }

    void suiFileMeta::setUploadUserId(const std::string& upload_user_id)
    {
        _upload_user_id = upload_user_id;
    }

    void suiFileMeta::setUploadTime()
    {
        _upload_time = static_cast<std::uint64_t>(std::time(nullptr));
    }
    /*file end ===============================================================================================*/
}

