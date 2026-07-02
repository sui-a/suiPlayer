#include <iostream>
#include <string>
#include "suiSerSearch.hpp"

int main()
{
    sui::serProvider provider("sss", "http://121.5.206.226:2379");
    provider.redister("190.168.1.100:8080");
    std::cout << "开始添加第二个服务" << std::endl;
    sui::serProvider provider2("sss", "http://121.5.206.226:2379");
    provider2.redister("190.168.1.101:8080");
    std::cout << "回车退出..." << std::endl;
    getchar();
    return 0;
}