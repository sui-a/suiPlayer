#include"suiTime.hpp"

namespace suiTime
{
    long long suiTimeOperater::get_timestamp_sec() 
    {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    }

    long long get_timestamp_ms() 
    {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }

    std::vector<int> timestamp_sec_to_vec(std::uint64_t timestamp_sec) 
    {
        // 将 uint64_t 转换为 time_t (本质是 time_t，能安全转换)
        std::time_t time_raw = static_cast<std::time_t>(timestamp_sec);
        
        // 转换为本地时间结构体 tm
        std::tm* time_info = std::localtime(&time_raw);
        
        // 如果转换失败（例如时间戳非法），返回空 vector
        if (!time_info) return {};

        // 结构体中的年份是从 1900 年开始计算的，月份是 0-11
        int year   = time_info->tm_year + 1900;
        int month  = time_info->tm_mon + 1;
        int day    = time_info->tm_mday;
        int hour   = time_info->tm_hour;
        int minute = time_info->tm_min;
        int second = time_info->tm_sec;

        // 按照从大到小排列返回
        return {year, month, day, hour, minute, second};
    }

    std::vector<int> timestamp_ms_to_vec(std::uint64_t timestamp_ms) 
    {
        // 算出秒数和剩余的毫秒数
        std::uint64_t sec = timestamp_ms / 1000;
        int ms = static_cast<int>(timestamp_ms % 1000);

        // 先利用上面的函数获取秒级的时间数组
        std::vector<int> result = timestamp_sec_to_vec(sec);
        
        if (result.empty()) return {};

        // 将毫秒追加到末尾（依然保持从大到小的顺序）
        result.push_back(ms);
        return result;
    }

    

}
