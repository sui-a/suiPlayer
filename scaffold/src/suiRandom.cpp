#include "suiRandom.hpp"

namespace suiRandom
{
    const size_t uuid_size = 32;

    std::string RandomUtil::uuid(RandomUtil::UuidType type, size_t length)
    {
        static const std::string digitArray = "0123456789";
        static const std::string charArray = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        static const std::string allArray = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        static std::atomic<unsigned int> counter(0);
        std::string result;
        const std::string* arrayPtr = nullptr;
        if(type == UuidType::DIGIT)
            arrayPtr = &digitArray;
        else if(type == UuidType::ALPHA)
            arrayPtr = &charArray;
        else
            arrayPtr = &allArray;

        //获取机器级随机数
        std::random_device rd;
        size_t seed = rd();
        //创建随机数引擎
        std::mt19937 gen(seed);
        for(size_t i = 0; i < length; i++)
        {
            //生成伪随机uid
            int index = (gen()) % (arrayPtr->size());
            result += ((*arrayPtr)[index]);
        }

        unsigned int val = counter.load(); // 原子读取
        // 将计数器转换为字符串并追加到结果中
        std::ostringstream oss;
        oss << val;
        result += oss.str();
        counter++;
        return result;
    }

}
