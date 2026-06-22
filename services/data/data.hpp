#pragma once
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#include <string>
#include <cstddef>
#ifndef ODB_COMPILER
#include <suiScaffold/suiodb.hpp>
#endif
#ifdef ODB_COMPILER
#include <odb/core.hxx>
#include <odb/nullable.hxx>
#endif
#include <memory>
#include <optional>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <suiScaffold/log.h>
#include <sstream>

namespace suiDataSql{
    //定义数据结构
    #pragma db object table("tbl_session_meta")
    class suiSessionMeta
    {
    public:
        using ptr = std::shared_ptr<suiSessionMeta>;
        suiSessionMeta() = default;
        suiSessionMeta(const std::string& session_id, const std::string& user_id);
        suiSessionMeta(const std::string& session_id);
        ~suiSessionMeta();

        unsigned long long getPrimaryKey() const;
        std::string getSessionId() const;
        odb::nullable<std::string> getUserId() const;
        std::string getUploadTimeString() const;
        std::uint64_t getUploadTime() const;

        void setSessionId(const std::string& session_id);
        void setUserId(odb::nullable<std::string> user_id);
        void setUploadTime();

    private:
        friend class odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey;
        #pragma db not_null index unique type("VARCHAR(128)")
        std::string _session_id;
        #pragma db index type("VARCHAR(128)")
        odb::nullable<std::string> _user_id;
        #pragma db type("BIGINT UNSIGNED NOT NULL DEFAULT 0")
        std::uint64_t _upload_time;
    };

    //构造视图
    #pragma db view object(suiSessionMeta) query((?))
    struct suiSessionPtr
    {
    public:
        using ptr = std::shared_ptr<suiSessionPtr>;
        std::shared_ptr<suiSessionMeta> session;
    };

    enum class fileStatus
    {
        //未知状态
        fileStatusUnknown = 0,
        //在上传状态
        fileStatusUploading = 1,
        //上传成功
        fileStatusSuccess = 2,
        //被下架
        fileStatusDown = 3,
    };

    #pragma db object table("tbl_file_meta")
    class suiFileMeta
    {
    public:
        using ptr = std::shared_ptr<suiFileMeta>;

        suiFileMeta(const std::string& file_id, const std::string& _upload_user_id);
        suiFileMeta(const std::string& file_id, const std::string& _upload_user_id, const std::string& path, size_t size, const std::string& mime_type);
        suiFileMeta(const std::string& file_id);
        suiFileMeta();
        ~suiFileMeta();

        std::string getFileId() const;
        std::string getUploadUserId() const;
        odb::nullable<std::string> getPath() const;
        size_t getSize() const;
        odb::nullable<std::string> getMimeType() const;
        std::string getUploadTimeString() const;
        std::uint64_t getUploadTime() const;
        fileStatus getFileStatus() const;
        
        void setFileId(const std::string& file_id);
        void setPath(const std::string& path);
        void setSize(size_t size);
        void setMimeType(const std::string& mime_type);
        void setUploadUserId(const std::string& upload_user_id);
        void setUploadTime();
        void setFileStatus(fileStatus status);

    private:
        friend class odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey;
        #pragma db not_null index unique type("VARCHAR(128)")
        std::string _file_id;  //非空索引
        #pragma db not_null index type("VARCHAR(128)")
        std::string _upload_user_id; //外键效率低 通过手动维护
        #pragma db type("VARCHAR(512)")
        odb::nullable<std::string> _path; //文件存储路径
        size_t _size = 0;  //文件大小
        #pragma db type("VARCHAR(64)")
        odb::nullable<std::string> _mime_type; //文件mime类型
        #pragma db type("BIGINT UNSIGNED NOT NULL DEFAULT 0")
        std::uint64_t _upload_time;
        //文件状态
        int _file_status;
    };
}