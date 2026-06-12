#pragma once
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
#include <string>

namespace suiUtil
{
    //定义日志器
    extern std::shared_ptr<spdlog::logger> logger;

    //定义日志配置结构体
    struct logSetting
    {
        bool async; //是否异步
        int level;  //日志输出等级 1debug 2info 3warn 4error 6off
        std::string format;  //日志输出格式
        std::string path;   //日志输出目标 "stdout"用作标准输出, 其他情况为输出路径
    };

    //初始化接口
    void suiLogInit(const logSetting& settings);
    //默认初始化接口
    void suiLogInitDefault();


    //定义日志输出宏
    #define DEBUG(fmt, ...) suiUtil::logger->debug(std::string("[{}][{}]: ") + fmt, __FILE__, __LINE__, ##__VA_ARGS__)
    #define INFO(fmt, ...) suiUtil::logger->info(std::string("[{}][{}]: ") + fmt, __FILE__, __LINE__, ##__VA_ARGS__)
    #define WARN(fmt, ...) suiUtil::logger->warn(std::string("[{}][{}]: ") + fmt, __FILE__, __LINE__, ##__VA_ARGS__)
    #define ERROR(fmt, ...) suiUtil::logger->error(std::string("[{}][{}]: ") + fmt, __FILE__, __LINE__, ##__VA_ARGS__)

}