#include "../include/suiRedis.hpp"

namespace sui
{
    void suiRedis::ReplyDeleter::operator()(redisReply* r) 
    { 
        if (r) 
            freeReplyObject(r); 
    }

    suiRedis::suiRedis(redisSetting& setting)
        : _isFree(true)
        , _setting(setting)
    {
        ctx = redisConnect(_setting.host.c_str(), _setting.port);
        if (ctx == NULL || ctx->err) 
        {
            if (ctx) 
            {
                setError("连接错误: " + std::string(ctx->errstr));
            } 
            else 
            {
                _isFree = false;
                setError("无法分配 redisContext");
            }
            return;
        }

        //开始密码认证
        if(!_setting.password.empty())
        {
            std::unique_ptr<redisReply, ReplyDeleter> reply((redisReply*)redisCommand(ctx, "AUTH %s", _setting.password.c_str()));
                
            // 检查 AUTH 命令是否成功返回 "OK"
            if (!reply || reply->type == REDIS_REPLY_ERROR) 
            {
                std::string err_msg = reply ? reply->str : ctx->errstr;
                setError("Redis 密码认证失败: " + err_msg);
                return;
            }
        }

        if(_setting.db > 0)
        {
            std::unique_ptr<redisReply, ReplyDeleter> reply((redisReply*)redisCommand(ctx, "SELECT %d", _setting.db));
            if (!reply || reply->type == REDIS_REPLY_ERROR) 
            {
                std::string err_msg = reply ? reply->str : ctx->errstr;
                setError("数据库选择失败: " + err_msg);
                return;
            }
        }
        INFO("redis初始化成功 \n 连接主机：{}\r\n 目标端口： {}", setting.host, setting.port);
    }

    suiRedis::~suiRedis()
    {
        if(_isFree)
            redisFree(ctx);
        INFO("redis对象销毁");
    }

    void suiRedis::setError(const std::string& msg)
    {
        _errorMsg = msg;
        ERROR("{}", msg);
    }

    std::optional<std::string> suiRedis::exec(const std::string& cmd)
    {
        if(isError())
            return std::nullopt;

        redisReply *reply = (redisReply *)redisCommand(ctx, cmd.c_str());
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
        INFO("redis执行成功： {}", cmd);
        std::string out = reply->str;
        freeReplyObject(reply);
        return out;
    }

    bool suiRedis::setString(const std::string& key, const std::string& value)
    {
        std::stringstream cmd;
        cmd << "SET " << key << " " << value;
        return exec(cmd.str()) == "OK";
    }

    std::optional<std::string> suiRedis::getString(const std::string& key)
    {
        std::stringstream cmd;
        cmd << "GET " << key;
        auto reply = exec(cmd.str());
        if(reply == "(null)")
            return std::nullopt;
        return reply;
    }

    bool suiRedis::isError()
    {
        if(ctx == nullptr || ctx->err || (_isFree == false) || !_errorMsg.empty())
                return true;

        return false;
    }
    
    std::string suiRedis::getError()
    {
        return _errorMsg;
    }

}
