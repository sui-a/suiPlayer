#pragma once
#include <iostream>
#include <string>
#include <optional>

extern "C" {
    #include <fastcommon/logger.h>
    #include <fastdfs/fdfs_client.h>
}


namespace sui
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

    class suiFastdfs
    {
        suiFastdfs();
        static std::optional<std::string> init(const FastdfsSetting& setting);

    public:
        ~suiFastdfs();
        static std::optional<std::string> upload_from_file(const std::string& filepath, std::string& file_id); //从文件上传数据
        static std::optional<std::string> download_to_file(const std::string& filepath, const std::string& file_id, int64_t* file_size_ptr = nullptr); //从文件下载数据
        static std::optional<std::string> delete_file(const std::string& file_id); //删除文件

        //针对流操作
        static std::optional<std::string> upload_from_buff(const std::string& buff, std::string& file_id); //从缓冲区上传数据
        static std::optional<std::string> download_to_buff(const std::string& file_id, std::string& buff); //下载数据到缓冲区

        friend std::optional<std::string> fdfsCreate(const FastdfsSetting& setting);

    private:
        static FastdfsSetting _setting; //此时的配置参数
    };

    inline suiFastdfs* client = nullptr;
    

}