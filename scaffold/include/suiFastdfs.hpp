#pragma once
#include <string>
#include <optional>
#include <memory>
extern "C" {
    #include <fastcommon/logger.h>
    #include <fastdfs/fdfs_client.h>
}
#ifdef byte
  #undef byte
#endif

#include "log.h"

namespace suifd
{
    struct FastdfsSetting
    {
    public:
        int connectTimeout = 5; //建立连接的超时时间 时间内没有连接成功，直接报错
        int networkTimeout = 60; //网络操作超时时间 时间内没有完成操作，直接报错
        bool useConnectionPool = false; //是否使用连接池
        int connectionPoolMaxIdleTime = 3600; //连接池最大空闲时间 单位秒 超过时间未使用的连接，会被关闭
        std::string Addr;

        std::string getBuffer();
    };

    //
    std::optional<std::string> fdfsCreate(const FastdfsSetting& setting);

    struct DownloadContext 
    {
        std::string* buffer;
    };

    // FastDFS 的 C 风格回调函数
    extern "C" int chunk_download_callback(void* arg, int64_t file_size, const char* block_buff, int block_bytes);

    class suiFastdfs
    {
        suiFastdfs();
        static std::optional<std::string> init(const FastdfsSetting& setting);

    public:
        using ptr = std::shared_ptr<suiFastdfs>;
        ~suiFastdfs();
        static std::optional<std::string> upload_from_file(const std::string& filepath, std::string& file_id); //从文件上传数据
        static std::optional<std::string> download_to_file(const std::string& filepath, const std::string& file_id, int64_t* file_size_ptr = nullptr); //从文件下载数据
        static std::optional<std::string> delete_file(const std::string& file_id); //删除文件

        //针对流操作
        static std::optional<std::string> upload_from_buff(const std::string& buff, std::string& file_id); //从缓冲区上传数据
        static std::optional<std::string> download_to_buff(const std::string& file_id, std::string& buff); //下载数据到缓冲区

        friend std::optional<std::string> fdfsCreate(const FastdfsSetting& setting);

        //分片流
        static std::optional<std::string> upload_appender_frist_from_buff(std::string& buff, std::string& file_id);
        static std::optional<std::string> upload_appender_from_buff(const std::string& file_id, std::string& buff);
        static std::optional<std::string> download_chunk_to_buff(const std::string& file_id, int64_t offset, int64_t download_bytes, std::string& out_buff);

    private:
        static FastdfsSetting _setting; //此时的配置参数
    };

    inline suiFastdfs* client = nullptr;
    

}