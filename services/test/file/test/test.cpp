#include <suiScaffold/suiFastdfs.hpp>
#include <suiScaffold/log.h>
#include <iostream>
#include <fstream>

int main()
{
    //初始化日志
    suiUtil::suiLogInitDefault();

    suifd::FastdfsSetting fdset;
    fdset.Addr = "127.0.0.1:22122";
    //开始初始化
    suifd::fdfsCreate(fdset);

    INFO("初始化完成");

    std::string fileImageContent;

    {
        std::string filePath = "./testfile/texas_head.jpg";
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if(!file.is_open()) {
            ERROR("打开文件失败, 文件路径为: {}", filePath);
            return -1;
        }
        
        // 获取文件大小
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);  // 回到文件开头
        
        // 调整string大小
        fileImageContent.resize(size);
        
        // 读取文件内容
        if (!file.read(fileImageContent.data(), size)) {
            ERROR("读取文件失败, 文件路径为: {}", filePath);
            return -1;
        }
        
        file.close();
        INFO("文件读取成功，大小: {} bytes", size);
    }

    std::string file_id;
    {
        //开始上传
        auto ret = suifd::suiFastdfs::upload_from_buff(fileImageContent, file_id);
        if(ret.has_value())
        {
            //
            ERROR("上传失败，错误为： {}", ret.value());
            return -1;
        }
        INFO("上传完成，文件_id为： {}", file_id);
    }

    INFO("开始下载文件");
    //开始下载
    {
        //开始下载
        std::string curbuff;
        std::string buff;
        suifd::suiFastdfs::download_to_buff(file_id, curbuff);
        INFO("下载完成，文件大小为： {}", curbuff.size());
        if(curbuff == fileImageContent)
        {
            INFO("下载成功");
        }
        else
        {
            ERROR("下载失败");
        }
    }

    return 0;
}