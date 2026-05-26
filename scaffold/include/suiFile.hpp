#include <fstream>
#include <iostream>
#include <string>


namespace sui
{
    class suiFile
    {
    public:
        static bool read(const std::string& filename, std::string& body);
        static bool write(const std::string& filename, const std::string& body);
    };



}