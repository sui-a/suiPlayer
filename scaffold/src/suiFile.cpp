#include "suiFile.hpp"


bool sui::suiFile::read(const std::string& filename, std::string& body)
{
    std::ifstream ifs;
    //以二进制读形式打开
    ifs.open(filename, std::ios::in | std::ios::binary);
    if(!ifs.is_open())
    {
        //打开失败，直接返回
        return false;
    }
    ifs.seekg(0, std::ios::end);
    size_t fileLen = ifs.tellg();
 
    //初始化文件指针
    ifs.seekg(0, std::ios::beg);
    body.resize(fileLen);
    ifs.read(&body[0], fileLen);
    if(!ifs.good())
    {
        //读取出现错误，直接返回
        ifs.close();
        return false;
    }
    ifs.close();
    return true;
}

bool sui::suiFile::write(const std::string& filename, const std::string& body)
{
    std::ofstream ofs;
    //以二进制截断写形式打开
    ofs.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if(!ofs.is_open())
    {
        //打开失败，直接返回
        return false;
    }
    ofs.write(body.c_str(), body.size());
    if(!ofs.good())
    {
        //写入失败，直接返回
        ofs.close();
        return false;
    }
    ofs.close();
    return true;
}