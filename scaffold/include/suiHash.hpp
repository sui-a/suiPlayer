#pragma once
#include <openssl/evp.h>
#include <iomanip>
#include <string>
#include <sstream>

namespace suiHash
{
    class hashQperation
    {
    public:
        static std::string calculateMD5(const std::string& data);
    };




}