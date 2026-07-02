#pragma once
#include <string>
#include <random>
#include <iomanip>
#include <atomic>


namespace suiRandom
{
    extern const size_t uuid_size;
    class RandomUtil
    {
    public:
        enum class UuidType
        {
            DIGIT, //数字
            ALPHA, //字母
            ALL    //数字和字母
        };

        //产生固定长度的随机字符串
        static std::string uuid(UuidType type = UuidType::DIGIT, size_t len = uuid_size);
    };



}