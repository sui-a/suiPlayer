#include "suiHash.hpp"

namespace suiHash
{
    std::string hashQperation::calculateMD5(const std::string& data)
    {
        unsigned char digest[EVP_MAX_MD_SIZE];
        unsigned int digest_len = 0;

        // 创建上下文
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (ctx == nullptr) return "";

        // 初始化、输入数据、生成摘要
        if (EVP_DigestInit_ex(ctx, EVP_md5(), nullptr) &&
            EVP_DigestUpdate(ctx, data.c_str(), data.length()) &&
            EVP_DigestFinal_ex(ctx, digest, &digest_len)) {
            // 成功
        }

        // 释放上下文
        EVP_MD_CTX_free(ctx);

        // 将二进制转为 32 位 Hex 字符串
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (unsigned int i = 0; i < digest_len; ++i) {
            oss << std::setw(2) << static_cast<int>(digest[i]);
        }

        return oss.str();
    }




    
}