#include "../include/log.h"

namespace suiUtil
{
    std::shared_ptr<spdlog::logger> logger;
    void suiLogInit(const logSetting& settings)
    {
        //进行初始化
        //判断日志器是否异步
        if(settings.async == true)
        {
            //异步
            //判断输出位置
            if(settings.path == "stdout")
            {
                //标准输出
                logger = spdlog::stdout_color_mt<spdlog::async_factory>("stdout_logger");
            }
            else
            {
                logger = spdlog::rotating_logger_mt<spdlog::async_factory>("file_logger", settings.path, 100 * 1024 * 1024, 5);
            }
        }
        else
        {
            //同步
            //判断输出位置
            if(settings.path == "stdout")
            {
                //标准输出
                logger = spdlog::stdout_color_mt("stdout_logger");
            }
            else
            {
                logger = spdlog::rotating_logger_mt("file_logger", settings.path, 100 * 1024 * 1024, 5);
            }
        }

        //设置日志等级
        logger->set_level(spdlog::level::level_enum(settings.level));
        //设置日志格式
        logger->set_pattern(settings.format);
    }

    void suiLogInitDefault()
    {
        suiUtil::logSetting settings;
        settings.async = true;
        settings.path = "stdout";
        settings.level = 1;
        settings.format = "%Y-%m-%d %H:%M:%S %e [%l] %v";
        suiUtil::suiLogInit(settings);
    }
}