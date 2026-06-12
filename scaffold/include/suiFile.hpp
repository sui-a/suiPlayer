#pragma once 
#include <fstream>
#include <iostream>
#include <string>


namespace suiFile
{
    class suiFiler
    {
    public:
        static bool read(const std::string& filename, std::string& body);
        static bool write(const std::string& filename, const std::string& body);
    };



}