#include <sstream>
#include <mutex>
#include "suiFastdfs.hpp"

namespace suifd
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
        
        if(ret != 0)
            return STRERROR(ret);
        
        file_id = id;
        tracker_close_connection(tracker_connection);
        return std::nullopt;
        
    }

    std::optional<std::string> suiFastdfs::download_to_buff(const std::string& file_id, std::string& buff)
    {
        auto tracker_connection = tracker_get_connection();
        if(tracker_connection == nullptr)
            return std::string("获取tracker服务器连接失败");
        
        
        char* buff_ptr = nullptr;
        int64_t buff_size = 0;
        auto ret = storage_download_file_to_buff1(tracker_connection, nullptr, file_id.c_str(), &buff_ptr, &buff_size);
        if(ret != 0)
            return STRERROR(ret);
        if(buff_size != 0)
        {
            buff = std::string(buff_ptr, buff_size);
            free(buff_ptr);
        }
        tracker_close_connection(tracker_connection);
        return std::nullopt;
    }

    std::optional<std::string> suiFastdfs::upload_appender_frist_from_buff(std::string& buff, std::string& file_id) 
    {
        auto tracker_connection = tracker_get_connection();
        if (tracker_connection == nullptr) {
            return "获取tracker服务器连接失败";
        }

        char file_id_buff[256];
        memset(file_id_buff, 0, 256);

        // 调用底层 _by_filebuff1 接口
        auto ret = storage_upload_appender_by_filebuff1(
            tracker_connection, 
            nullptr,            // storage server, 传 nullptr 由内部自动获取
            0,                  // store_path_index
            buff.c_str(),       // file_buff: 内存首地址
            buff.size(),        // file_size: 内存块大小
            nullptr,                // file_ext_name: 扩展名
            nullptr,            // meta_list
            0,                  // meta_count
            nullptr,            // group_name
            file_id_buff        // [out] 返回的 file_id
        );

        if (ret != 0) {
            tracker_close_connection(tracker_connection); // 注意：错误返回前必须释放连接
            return STRERROR(ret);
        }

        file_id = file_id_buff; // 赋值传出
        tracker_close_connection(tracker_connection);
        return std::nullopt;
    }

    std::optional<std::string> suiFastdfs::upload_appender_from_buff(const std::string& file_id, std::string& buff) 
    {
        auto tracker_connection = tracker_get_connection();
        if (tracker_connection == nullptr) {
            return "获取tracker服务器连接失败";
        }

        // 调用 append 接口将内存直接追加到 FastDFS
        auto ret = storage_append_by_filebuff1(
            tracker_connection, 
            nullptr,            // storage server
            buff.c_str(),               // file_buff: 内存首地址
            buff.size(),          // file_size: 内存块大小
            file_id.c_str()    // 目标 appender file_id
        );

        if (ret != 0) {
            tracker_close_connection(tracker_connection); // 错误返回前释放连接
            return STRERROR(ret);
        }

        tracker_close_connection(tracker_connection);
        return std::nullopt;
    }
    std::optional<std::string> suiFastdfs::download_chunk_to_buff(
            const std::string& file_id, 
            int64_t file_offset, 
            int64_t request_size, 
            std::string& out_buff) 
    {
        auto tracker_connection = tracker_get_connection();
        if (tracker_connection == nullptr)
            return "获取tracker服务器连接失败";

        // 1. 获取文件信息，用于计算剩余大小和防越界
        FDFSFileInfo file_info;
        int ret = fdfs_get_file_info1(file_id.c_str(), &file_info);
        if (ret != 0) 
        {
            tracker_close_connection(tracker_connection);
            return STRERROR(ret);
        }

        int64_t total_size = file_info.file_size;
        int64_t remain_size = total_size - file_offset;

        // 2. 检查偏移量是否已经超出文件范围
        if (remain_size <= 0) {
            out_buff.clear(); // 偏移量超出，返回空数据
            tracker_close_connection(tracker_connection);
            return std::nullopt; // 这不算API执行错误，直接返回成功状态即可
        }

        // 3. 核心截断逻辑：如果剩余量小于请求量，只下载剩余量
        int64_t final_download_bytes = (remain_size < request_size) ? remain_size : request_size;

        // 4. 准备接收数据的缓冲区
        out_buff.clear();
        out_buff.reserve(final_download_bytes); // 预分配内存，避免下载过程中多次扩容影响性能
        
        DownloadContext ctx;
        ctx.buffer = &out_buff;

        int64_t out_file_size = 0;

        // 5. 调用 ex1 接口执行精准下载
        ret = storage_download_file_ex1(
            tracker_connection, 
            nullptr,                // storage server, 传 nullptr 让 API 内部自动查询
            file_id.c_str(), 
            file_offset, 
            final_download_bytes,   // 传入修正后的精准大小
            chunk_download_callback, 
            &ctx, 
            &out_file_size
        );

        if (ret != 0) {
            tracker_close_connection(tracker_connection); // 错误返回前释放连接
            return STRERROR(ret);
        }

        tracker_close_connection(tracker_connection);
        return std::nullopt;
    }

    extern "C" int chunk_download_callback(void* arg, int64_t file_size, const char* block_buff, int block_bytes) 
    {
        auto* ctx = static_cast<DownloadContext*>(arg);
        // 将下载到的数据块追加到 std::string 容器中
        ctx->buffer->append(block_buff, block_bytes);
        return 0;
    }
}