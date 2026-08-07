#include "suiMail.hpp"

namespace suiMail
{
    suiMailClient::suiMailClient(const MailSetting& setting, const std::string& title)
        : _setting(setting)
        , _isError(false)
        , _isInitError(false)
        , _title(title)
        , _isTrack(false)
    {
        // 初始化全局配置
        auto ret = curl_global_init(CURL_GLOBAL_DEFAULT);
        if(ret != CURLE_OK)
        {
            // 初始化失败
            _errorMsg = "初始化全局配置失败: ";
            _isInitError = true;
            return;
        }
    }
    suiMailClient::~suiMailClient()
    {
        if(!_isInitError)
            curl_global_cleanup();  //如果全局初始化失败，不清理资源
    }

    void suiMailClient::setError(const std::string& msg)
    {
        _errorMsg = msg;
        _isError = true;
    }

    bool suiMailClient::send(const std::string& to, const std::string& body)
    {
        return _send(to, body);
    }

    bool suiMailClient::_send(const std::string& to, const std::string& body)
    {
        // 构造操作句柄
        auto curl = curl_easy_init();
        if(curl == nullptr)
        {
            // 初始化失败
            setError("构造操作句柄失败");
            return false;
        }
        // 设置请求参数 url username password body from to subject
        auto ret = curl_easy_setopt(curl, CURLOPT_URL, _setting.url.c_str());
        if(ret != CURLE_OK)
        {
            // 设置失败
            setError("设置url失败: " + (*curl_easy_strerror(ret)));
            return false;
        }

        ret = curl_easy_setopt(curl, CURLOPT_USERNAME, _setting.username.c_str());
        if(ret != CURLE_OK)
        {
            // 设置失败
            setError("设置用户名失败: " + (*curl_easy_strerror(ret)));
            return false;
        }

        ret = curl_easy_setopt(curl, CURLOPT_PASSWORD, _setting.password.c_str());
        if(ret != CURLE_OK)
        {
            // 设置失败
            setError("设置密码失败: " + (*curl_easy_strerror(ret)));
            return false;
        }

        ret = curl_easy_setopt(curl, CURLOPT_MAIL_FROM, _setting.from.c_str());
        if(ret != CURLE_OK)
        {
            // 设置失败
            setError("设置发件人失败: " + (*curl_easy_strerror(ret)));
            return false;
        }

        struct curl_slist* cs = nullptr;
        cs = curl_slist_append(cs, to.c_str());
        ret = curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, cs);
        if(ret != CURLE_OK)
        {
            // 设置失败
            setError("设置收件人失败: " + (*curl_easy_strerror(ret)));
            return false;
        }

        std::stringstream curbody;
        curbody << body;

        ret = curl_easy_setopt(curl, CURLOPT_READDATA, &curbody);
        if(ret != CURLE_OK)
        {
            // 设置失败
            setError("设置body失败失败: " + (*curl_easy_strerror(ret)));
            return false;
        }

        ret = curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        if(ret != CURLE_OK)
        {
            // 设置失败
            setError("设置上传失败: " + (*curl_easy_strerror(ret)));
            return false;
        }

        ret = curl_easy_setopt(curl, CURLOPT_LOGIN_OPTIONS, "AUTH=LOGIN");
        if(ret != CURLE_OK)
        {
            // 设置失败
            setError("设置登录选项失败: " + (*curl_easy_strerror(ret)));
            return false;
        }

        ret = curl_easy_setopt(curl, CURLOPT_READFUNCTION, suiMailClient::callback);
        if(ret != CURLE_OK)
        {
            // 设置失败
            setError("设置回调函数失败: " + (*curl_easy_strerror(ret)));
            return false;
        }
        if(_isTrack)
        {
            ret = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
                if(ret != CURLE_OK)
            {
                // 设置失败
                setError("开启追踪失败: " + (*curl_easy_strerror(ret)));
                return false;
            }
        }

        //执行请求
        ret = curl_easy_perform(curl);
        if(ret != CURLE_OK)
        {
            // 设置失败
            setError("发送执行失败: " + (*curl_easy_strerror(ret)));
            return false;
        }
        //清理资源
        curl_slist_free_all(cs);
        curl_easy_cleanup(curl);
        return true;
    }

    //请求处理回调  
    size_t suiMailClient::callback(char* buff, size_t size, size_t nitems, void* userdata)
    {
        std::stringstream* ssptr = (std::stringstream*)userdata;
        ssptr->read(buff, size * nitems);
        return ssptr->gcount();
    }

    void suiMailClient::setTrack()
    {
        _isTrack = true;
    }

    void suiMailClient::setNoTrack()
    {
        _isTrack = false;
    }

    bool suiMailClient::isError()
    {
        return _isError || _isInitError;
    }

    std::string& suiMailClient::getErrorMsg()
    {
        return _errorMsg;
    }

    std::string& suiMailClient::getFrom()
    {
        return _setting.from;
    }   

    std::string& suiMailClient::getTitle()
    {
        return _title;
    }

    std::string& suiMailClient::getUserName()
    {
        return _setting.username;
    }

}
