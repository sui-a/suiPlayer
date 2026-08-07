#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <curl/curl.h>

namespace suiMail
{
    struct MailSetting
    {
        std::string url;
        std::string username;
        std::string password;
        std::string from;
    };

    class suiCurlClient
    {
    public:
        suiCurlClient() = default;
        ~suiCurlClient() = default;

        virtual bool send(const std::string& to, const std::string& code) = 0;
    private:

    };

    class suiMailClient : public suiCurlClient
    {
    public:
        suiMailClient(const MailSetting& setting, const std::string& title = "验证码");
        ~suiMailClient();

        virtual bool send(const std::string& to, const std::string& body) override;

        void setTrack();
        void setNoTrack();

        bool isError();
        std::string& getErrorMsg();

        std::string& getFrom();
        std::string& getTitle();
        std::string& getUserName();

    private:

        //发送邮件
        bool _send(const std::string& to, const std::string& body);

        //请求处理回调
        static size_t callback(char* buff, size_t size, size_t nitems, void* userdata);

        //设置普通错误
        void setError(const std::string& msg);

    private:
        MailSetting _setting;
        std::string _title;

        //错误信息
        bool _isError = false; //普通错误
        bool _isInitError = false; //初始化错误
        std::string _errorMsg;
        bool _isTrack; //是否追踪
    };

}