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
        std::string curbody = codeBody(to, body);
        return _send(to, curbody);
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

        ret = curl_easy_setopt(curl, CURLOPT_USERNAME, _setting.password.c_str());
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

    std::string suiMailClient::codeBody(const std::string& to, const std::string body)
    {
        std::stringstream curbody;
        curbody << "From: " << _setting.from << std::endl;
        curbody << "To: " << to << std::endl;
        curbody << "Subject: " << _title << std::endl;
        curbody << "Content-Type: text/html; charset=utf-8" << std::endl;
        curbody << std::endl;
        //邮件使用html包装
        curbody << R"html(
        <!DOCTYPE html>
        <html lang="zh-CN">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>)html" << _title << R"html(</title>
            <style>
                body { 
                    font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif; 
                    background-color: #f4f7f6; 
                    margin: 0; 
                    padding: 40px 20px; 
                }
                .container { 
                    max-width: 500px; 
                    margin: 0 auto; 
                    background: #ffffff; 
                    border-radius: 8px; 
                    box-shadow: 0 4px 12px rgba(0,0,0,0.05); 
                    overflow: hidden; 
                }
                .header { 
                    background: #2b579a; 
                    color: #ffffff; 
                    padding: 20px; 
                    text-align: center; 
                    font-size: 20px; 
                    font-weight: bold; 
                }
                .content { 
                    padding: 30px; 
                    text-align: center; 
                    color: #333333; 
                }
                .code-box { 
                    background: #f8f9fa; 
                    border: 1px dashed #2b579a; 
                    color: #2b579a; 
                    font-size: 32px; 
                    font-weight: bold; 
                    letter-spacing: 6px; 
                    padding: 15px 20px; 
                    margin: 20px auto; 
                    border-radius: 6px; 
                    display: inline-block; 
                }
                .footer { 
                    font-size: 13px; 
                    color: #888888; 
                    margin-top: 30px; 
                    line-height: 1.6;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <div class="header">)html" << _title << R"html(</div>
                <div class="content">
                    <p style="font-size: 16px;">您好！您正在进行安全验证，您的验证码是：</p>
                    <div class="code-box">)html" << body << R"html(</div>
                    <p style="font-size: 15px; font-weight: bold; color: #e53935;">请在 5 分钟内输入验证码</p>
                    <div class="footer">
                        如果您没有请求此验证码，请忽略此信息。<br>
                        （系统自动发送，请勿回复）
                    </div>
                </div>
            </div>
        </body>
        </html>
        )html";
        
        return curbody.str();
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


}
