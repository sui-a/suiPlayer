#include "file.hpp"
#include <cstdlib>
#include <string>
#include <suiScaffold/log.h>
#include <iostream>
#include "data.hpp"
#include "data-odb.hxx"
#include "file.hpp"


int main()
{
    suiUtil::suiLogInitDefault();
    {
        //创建数据库操作句柄
        INFO("创建数据库操作句柄");
        suiOdb::odbSetting dbSetting;
        dbSetting._host = "sh-cdb-l9h13in4.sql.tencentcdb.com";
        dbSetting._port = 24411;
        dbSetting._user = "sui";
        dbSetting._password = "suisuipingan";
        dbSetting._database = "bilibili";
        dbSetting._connection_pool_size = 10;
        INFO("数据库操作设置声明成功");
        auto handler = suiOdb::dbFactory::create(dbSetting);
        if (handler == nullptr)
        {
            ERROR("创建数据库操作句柄失败");
            return -1;
        }
       
        try
        {
            {
                //新增数据库事务
                INFO("新增数据库事务");
                odb::transaction t(handler->begin());
                suiDataSql::suiFileMeta fileMeta("123456", "sui");
                fileMeta.setSize(1024);
                fileMeta.setPath("https://test.com");
                fileMeta.setMimeType("text/plain");
                fileMeta.setUploadTime();
                suiFile::FileData fileData(t.database());
                fileData.insert(fileMeta);
                //提交事务
                t.commit();
                INFO("新增数据库事务提交成功");
            }
            INFO("按任意键继续");
            std::cin.get(); // 等待用户输入
            {
                //查询更新数据库事务
                INFO("查询更新数据库事务");
                odb::transaction t(handler->begin());
                suiFile::FileData fileData(t.database());
                suiDataSql::suiFileMeta::ptr fileMeta = fileData.selectFileByFileId("123456");
                if (fileMeta == nullptr)
                {
                    ERROR("查询文件失败");
                    return -1;
                }
                INFO("查询文件成功");
                INFO("文件ID: {}", fileMeta->getFileId());
                INFO("上传用户ID: {}", fileMeta->getUploadUserId());
                if(!fileMeta->getPath().null())
                {
                    INFO("路径: {}", fileMeta->getPath().get());
                }
                INFO("大小: {}", fileMeta->getSize());
                if(!fileMeta->getMimeType().null())
                    INFO("MIME类型: {}", fileMeta->getMimeType().get());
                INFO("上传时间: {}", fileMeta->getUploadTimeString());
                
                //更新文件信息
                fileMeta->setPath("https://test.com/2");
                fileMeta->setSize(2048);
                fileMeta->setMimeType("text/html");
                INFO("更新文件信息");
                fileData.update(*fileMeta);
                //提交事务
                t.commit();
                INFO("查询更新数据库事务提交成功");
                INFO("按任意键继续");
                std::cin.get(); // 等待用户输入
            }
            {
                INFO("开始更新查询");
                odb::transaction t(handler->begin());
                suiFile::FileData fileData(t.database());
                suiDataSql::suiFileMeta::ptr fileMeta = fileData.selectFileByFileId("123456");
                if (fileMeta == nullptr)
                {
                    ERROR("查询文件失败");
                    return -1;
                }
                INFO("查询文件成功");
                INFO("文件ID: {}", fileMeta->getFileId());
                INFO("上传用户ID: {}", fileMeta->getUploadUserId());
                if(!fileMeta->getPath().null())
                {
                    INFO("路径: {}", fileMeta->getPath().get());
                }
                INFO("大小: {}", fileMeta->getSize());
                if(!fileMeta->getMimeType().null())
                    INFO("MIME类型: {}", fileMeta->getMimeType().get());
                INFO("上传时间: {}", fileMeta->getUploadTimeString());
            }
        }
        catch (const std::exception& e)
        {
            ERROR("出现错误 {}", e.what());
        }
        catch (...)
        {
            ERROR("出现其他错误");
        }
    }
    INFO("测试结束");
    return 0;
}