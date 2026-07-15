#include <vector>
#include <chrono>
#include <ctime>
#include <cstdint>

namespace suiTime
{
    class suiTimeOperater
    {
    public:
        // 返回秒级时间戳 (10位)
        static long long get_timestamp_sec();

        // 返回毫秒级时间戳 (13位)
        static long long get_timestamp_ms();

        // 1. 秒级时间戳转年月日时分秒
        static std::vector<int> timestamp_sec_to_vec(std::uint64_t timestamp_sec);

        // 2. 毫秒级时间戳转年月日时分秒毫秒
        static std::vector<int> timestamp_ms_to_vec(std::uint64_t timestamp_ms);
    };


    
}