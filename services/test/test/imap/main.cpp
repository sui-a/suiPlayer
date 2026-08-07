#include "suiMail.hpp"
#include <iostream>
#include <string>
#include <sstream>

std::string codeBody(const std::string& from, const std::string& _title, const std::string& to, const std::string body)
{
    std::stringstream curbody;
    curbody << "From: " << from << std::endl;
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

int main()
{
    suiMail::MailSetting set;
    set.from = "2076354958@qq.com";
    set.username = "2076354958@qq.com";
    set.password = "uddbtalnaergciaf";
    set.url = "smtps://smtp.qq.com:465";

    //初始化操作类
    suiMail::suiMailClient mailClient(set);

    //发送邮件
    std::string body = codeBody(set.from, mailClient.getTitle(), "1953114602@qq.com", "猪猪猪");
    std::cout << "============================================" << std::endl;
    std::cout << body << std::endl;
    mailClient.send("1953114602@qq.com", body);
    if(mailClient.isError())
    {
        std::cerr << "发送邮件失败: " << mailClient.getErrorMsg() << std::endl;
    }
    else
    {
        std::cout << "发送邮件成功" << std::endl;
    }
    return 0;
}