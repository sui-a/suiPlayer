#include <sstream>
#include <mutex>
#include "suiFastdfs.hpp"

namespace sui
{
    FastdfsSetting suiFastdfs::_setting = FastdfsSetting();
    static std::mutex Fastdfs_mutex;

    std::optional<std::string> fdfsCreate(const FastdfsSetting& setting)
    {
        if(client == nullptr)
        {
            std::lock_guard<std::mutex> lock(Fastdfs_mutex);
            if(client == nullptr)
            {
                client = new suiFastdfs();
                auto ret = suiFastdfs::init(setting);
                return ret;
            }
        }
        return std::nullopt;
    }
    suiFastdfs::suiFastdfs()
    {
        //初始化日志模块
        g_log_context.log_level = LOG_ERR;
        log_init();
    }

    std::string FastdfsSetting::getBuffer()
    {
        std::stringstream ss;

        ss << "connect_timeout = " << connectTimeout << "\n";
        ss << "network_timeout = " << networkTimeout << "\n";
        ss << "tracker_server = " << Addr << "\n";
        ss << "use_connection_pool = " << (useConnectionPool ? "true" : "false") << "\n";
        ss << "connection_pool_max_idle_time = " << connectionPoolMaxIdleTime << "\n";

        std::string out = ss.str();
        return out;
    }

    std::optional<std::string> suiFastdfs::init(const FastdfsSetting& setting)
    {
        //初始化全局=
        suiFastdfs::_setting = setting;
        
        std::string config_content = _setting.getBuffer();
        int ret = fdfs_client_init_from_buffer(config_content.c_str());
        
        if(ret != 0)
        {
            return STRERROR(ret);
        }
        return std::nullopt;
    }

    std::optional<std::string> suiFastdfs::upload_from_file(const std::string& filepath, std::string& file_id)
    {
        auto tracker_connection = tracker_get_connection();
        if(tracker_connection == nullptr)
            return "获取tracker服务器连接失败";

        char file_id_buff[256];
        memset(file_id_buff, 0, 256);
        auto ret = storage_upload_by_filename1(tracker_connection, nullptr, 0, filepath.c_str(), nullptr, nullptr, 0, nullptr, file_id_buff);
        
        if(ret != 0)
            return STRERROR(ret);

        file_id += file_id_buff;
        tracker_close_connection(tracker_connection);
        return std::nullopt;
    }

    std::optional<std::string> suiFastdfs::download_to_file(const std::string& filepath, const std::string& file_id, int64_t* file_size_ptr)
    {
        auto tracker_connection = tracker_get_connection();
        if(tracker_connection == nullptr)
            return std::string("获取tracker服务器连接失败");
        std::cout << "1" << std::endl;
        int64_t file_size = 0;
        auto ret = storage_download_file_to_file1(tracker_connection, nullptr, file_id.c_str(), filepath.c_str(), &file_size);
        if (ret != 0)
            return STRERROR(ret);
        
        if(file_size_ptr != nullptr)
            *file_size_ptr = file_size;
        tracker_close_connection(tracker_connection);
        return std::nullopt;
    }


    std::optional<std::string> suiFastdfs::delete_file(const std::string& file_id)
    {
        auto tracker_connection = tracker_get_connection();
        if(tracker_connection == nullptr)
            return "获取tracker服务器连接失败";

        auto ret = storage_delete_file1(tracker_connection, nullptr, file_id.c_str());

        if (ret != 0)
            return STRERROR(ret);
        
        tracker_close_connection(tracker_connection);
        return std::nullopt;
    }


    suiFastdfs::~suiFastdfs()
    {
        fdfs_client_destroy();
    }

    std::optional<std::string> suiFastdfs::upload_from_buff(const std::string& buff, std::string& file_id)
    {
        auto tracker_connection = tracker_get_connection();
        if(tracker_connection == nullptr)
            return std::string("获取tracker服务器连接失败");
        
        char id[256];
        char group[256];
        auto ret = storage_upload_by_filebuff1(tracker_connection, nullptr, 0,  buff.c_str(), buff.size(), nullptr, nullptr, 0, nullptr, id); //返回有问题
        
        std::cout << "id:  " << *id << std::endl;
        if(ret != 0)
            return STRERROR(ret);

        file_id = id;
        std::cout << "id: " << file_id << std::endl;
        std::cout << "组是： " << group << std::endl;
        tracker_close_connection(tracker_connection);
        return std::nullopt;
        
    }

    std::optional<std::string> suiFastdfs::download_to_buff(const std::string& file_id, std::string& buff)
    {
        auto tracker_connection = tracker_get_connection();
        if(tracker_connection == nullptr)
            return std::string("获取tracker服务器连接失败");
        
        char *buff_ptr = nullptr;
        int64_t buff_size = 0;
        auto ret = storage_download_file_to_buff1(tracker_connection, nullptr, file_id.c_str(), &buff_ptr, &buff_size);
        if(ret != 0)
            return STRERROR(ret);

        buff = buff_ptr;
        tracker_close_connection(tracker_connection);
        return std::nullopt;
    }

    
}