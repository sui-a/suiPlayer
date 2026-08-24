#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiJson.hpp>
#include <suiScaffold/suiRpc.hpp>
#include <brpc/server.h>
#include "video.pb.h"
#include "video_subtitle_target.hpp"

DEFINE_string(registry_center, "127.0.0.1:8084", "注册中心地址");
DEFINE_string(service_name, "video_server", "user_server服务名称");


std::string sessionId = "222222";
std::string userId = "111111";

void addVideo(suiRpc::Channels::Ptr _channels)
{
    //随机新增一个测试视频
    suiRpc::ChannelPtr curChannel;
    while(1)
    {
        curChannel = _channels->select();   
        if(curChannel)
            break;
        INFO("等待服务上线");
        sleep(1);
    }
    //发起实例化服务对象与控制对象
    suiApi::VideoService_Stub stub(&(*curChannel));
    brpc::Controller* cntl = new brpc::Controller();
    cntl->set_timeout_ms(-1); //无限等待
    //定义调用参数
    suiApi::newVideoReq req;
    suiApi::newVideoRsp rsp;
    req.set_id("1");
    req.set_sessionid(sessionId);
    {
        auto videoInfo = req.mutable_videoinfo();
        videoInfo->set_videoid("111");
        videoInfo->set_userid(userId);
        videoInfo->set_useravatarid("fid_avatar_8h6t5r");
        videoInfo->set_nickname("sui");
        videoInfo->set_videotype(2);                     // 2 = 短视频
        // repeated 字段用 add_videotag
        videoInfo->add_videotag(101);
        videoInfo->add_videotag(204);
        videoInfo->add_videotag(307);
        videoInfo->set_videofileid("111");
        videoInfo->set_photofileid("222");
        videoInfo->set_likecount(42);
        videoInfo->set_playcount(1337);
        videoInfo->set_videosize(18888888);              // ~18MB
        videoInfo->set_videodesc("这是一段随机测试视频描述#测试tag");
        videoInfo->set_videotitle("测试标题_随机字符_qwerty");
        videoInfo->set_videoduration(66);                // 66秒
        videoInfo->set_videouptime(1700000000);          // 固定时间戳
        // 审核相关（刚转码完还没人审核，留空）
        videoInfo->set_checkerid("");
        videoInfo->set_checkername("");
        videoInfo->set_checkeravatar("");
        // 枚举：设置为待审核
        videoInfo->set_status(suiApi::videoStatus::videoStatusUpload);
    }
    //开始调用
    stub.newVideo(cntl, &req, &rsp, nullptr);
    if(cntl->Failed())
    {
        //调用失败
        ERROR("stub请求失败: {}", cntl->ErrorText());
        return;
    }
    if(rsp.errorcode() != 0)
    {
        //出现错误
        ERROR("临时会话申请失败, 错误为: {}", rsp.errormsg());
        return ;
    }
    INFO("添加成功");
}

void del

int main(int argc, char* argv[])
{
    //初始化日志
    suiUtil::suiLogInitDefault();
    //解析gflags
    google::ParseCommandLineFlags(&argc, &argv, true);
    //创建搜索服务
    suiRpc::Channels::Ptr _channels = std::make_shared<suiRpc::Channels>(FLAGS_service_name);
    suiEtcd::serSearch::ptr _search = std::make_shared<suiEtcd::serSearch>(FLAGS_service_name, FLAGS_registry_center
        , [_channels](std::string serName, std::string addr){
        _channels->insert(addr);
        INFO("添加服务： {} 的节点: {}", serName, addr);
    }, [_channels](std::string serName, std::string addr){
        INFO("删除服务： {} 的节点: {}", serName, addr);
        _channels->remove(addr);
    });
    //开始服务发现
    _search->search();

    INFO("开始测试");
    INFO("开始新增视频");
    addVideo(_channels);
    INFO("测试结束，点击回车退出");
    std::cin.get();
    return 0;
}
