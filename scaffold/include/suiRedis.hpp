#pragma once
#include <hiredis/hiredis.h>
#include "log.h"
#include <string>
#include <optional>
#include <memory>
#include <sstream>
#include <iostream>

namespace sui
{
    struct redisSetting
    {
        std::string host;
        int port;
        std::string password;
        int db = 0;
    };

    class suiRedis
    {
    protected:
        void setError(const std::string& msg);

        struct ReplyDeleter 
        {
            void operator()(redisReply* r);
        };

    public:
        suiRedis(redisSetting& setting);
        virtual ~suiRedis();

        bool setString(const std::string& key, const std::string& value);
        std::optional<std::string> getString(const std::string& key);

        bool isError();
        std::string getError();

        template<typename... t>
        std::optional<std::string> exec(const std::string& cmd, t... args)
        {
            if(isError())
                return std::nullopt;

            auto reply = (redisReply *)redisCommand(ctx, cmd.c_str(), args...);
            if(reply == nullptr)
            {
                std::string errType;
                switch (ctx->err) 
                {
                    case REDIS_ERR_IO:
                        errType = "I/O 错误 (网络异常)";
                        break;
                    case REDIS_ERR_EOF:
                        errType = "服务端关闭了连接 (EOF)";
                        break;
                    case REDIS_ERR_PROTOCOL:
                        errType = "协议解析错误";
                        break;
                    case REDIS_ERR_OOM:
                        errType = "内存不足 (OOM)";
                        break;
                    case REDIS_ERR_OTHER:
                        errType = "连接或读取超时";
                        break;
                    default:
                        errType = "未知系统错误";
                        break;
                }
                setError(errType);
                return std::nullopt;
            }
            std::string out = reply->str;
            freeReplyObject(reply);
            return out;
        }

        std::optional<std::string> exec(const std::string& cmd);
    protected:
        redisContext *ctx;
        redisSetting _setting;

        bool _isFree; //是否要释放 false 表示不释放（已释放或者错误）
        std::string _errorMsg;
    };
}