#include <iostream>
#include <string>
#include "suiSerSearch.hpp"

void online(std::string serName, std::string serAddr)
{
    std::cout << "服务上线: " << serName << ", 地址: " << serAddr << std::endl;
}

void offline(std::string serName, std::string serAddr)
{
    std::cout << "服务下线: " << serName << ", 地址: " << serAddr << std::endl;
}

int main()
{
    sui::serSearch search("sss", "http://121.5.206.226:2379", online, offline);
    search.search();
    std::cout << "正在监控服务变化..." << std::endl;
    std::cout << "回车退出..." << std::endl;
    std::fflush(stdout);
    getchar();
    return 0;
}