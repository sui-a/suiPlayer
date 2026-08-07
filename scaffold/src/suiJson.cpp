#include "suiJson.hpp"

namespace suiUtil
{
    std::optional<std::string> suiJson::serialize(const Json::Value& val)
    {
        //构建序列化类
        Json::StreamWriterBuilder builder;
        //builder构建的时候会对列表每个项换行，这里是用来去除换行的
        builder["commentStyle"] = "None";
        //去除类中每一项开头的换行以及缩进
        builder["indentation"] = "";
        std::unique_ptr<Json::StreamWriter> wterptr(builder.newStreamWriter());
        //构建字符串流
        std::stringstream ss;
        //流式输出到ss字符串中
        int ret = wterptr->write(val, &ss);
        if(ret != 0)
            return std::nullopt;
        return ss.str();
    }

    std::optional<Json::Value> suiJson::unserialize(const std::string& inp)
    {
        //构建read类的构造对象
        Json::CharReaderBuilder builder;
        Json::Value val;
        std::unique_ptr<Json::CharReader> readptr(builder.newCharReader());
        std::string errs;
        bool rel = readptr->parse(inp.c_str(), inp.c_str() + inp.size(), &val, &errs);
        if(rel == false)
            //反序列化失败
            return std::nullopt;

        return val;
    }

}