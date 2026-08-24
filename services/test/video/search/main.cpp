#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiJson.hpp>
#include <suiScaffold/suiElastic.hpp>
#include "suiTime.hpp"
#include "video_search.hpp"

//随机上传视频
void uploadRandomVideo(suiVideoSearch::videoSearch& handle)
{
    int64_t nowTime = suiTime::suiTimeOperater::get_timestamp_sec();
    int64_t oneHour = 3600;
    int64_t oneDay = 86400;
    int64_t oneMonth = 86400 * 30;
    {
        handle.insertVideo("vid_101", suiDataSql::videoStatus::videoStatusApproved, 
                   "C++基础教程：从入门到放弃", "最全的C++基础语法讲解，新手必看保姆级教程", nowTime - oneMonth * 12); // 1年前
        handle.insertVideo("vid_102", suiDataSql::videoStatus::videoStatusApproved, 
                        "C++11/14/17新特性全解析", "现代C++编程指南，智能指针与Lambda表达式实战", nowTime - oneMonth * 6);
        handle.insertVideo("vid_103", suiDataSql::videoStatus::videoStatusApproved, 
                        "C++高并发内存池设计与实现", "硬核底层实战，带你手写tcmalloc核心架构", nowTime - oneDay * 15);
        handle.insertVideo("vid_104", suiDataSql::videoStatus::videoStatusApproved, 
                        "为什么大厂还在用C++？", "深度分析C++在音视频、游戏引擎和系统底层的不可替代性", nowTime - oneDay * 2);
        handle.insertVideo("vid_105", suiDataSql::videoStatus::videoStatusApproved, 
                        "C加加面试八股文速成", "明天就面试？这期视频帮你突击C++常见考点（含错别字测试）", nowTime - oneHour * 10);

        // 【集群 B：Redis 与高并发架构】 (测试交叉搜索)
        handle.insertVideo("vid_201", suiDataSql::videoStatus::videoStatusApproved, 
                        "Redis缓存穿透、击穿、雪崩怎么防？", "后端面试必问的高并发架构缓存三连击解决方案", nowTime - oneMonth * 3);
        handle.insertVideo("vid_202", suiDataSql::videoStatus::videoStatusApproved, 
                        "Redis底层数据结构剖析", "跳表、SDS、压缩列表原理，C++源码级硬核讲解", nowTime - oneDay * 20);
        handle.insertVideo("vid_203", suiDataSql::videoStatus::videoStatusApproved, 
                        "10分钟教你搭建Redis集群", "手把手实操Redis Cluster主从同步与哨兵模式", nowTime - oneDay * 1);
        handle.insertVideo("vid_204", suiDataSql::videoStatus::videoStatusApproved, 
                        "千万级弹幕系统的缓存设计", "基于Redis和C++实现B站同款高并发弹幕服务实战", nowTime - oneHour * 2);

        // 【集群 C：Elasticsearch 与搜索】 (测试相关度与最新发布)
        handle.insertVideo("vid_301", suiDataSql::videoStatus::videoStatusApproved, 
                        "Elasticsearch快速入门实战", "全文搜索引擎ES的核心概念与倒排索引原理解析", nowTime - oneMonth * 8);
        handle.insertVideo("vid_302", suiDataSql::videoStatus::videoStatusApproved, 
                        "ES倒排索引底层原来是这样的", "揭秘Elasticsearch极速检索背后的数据结构，硬核架构", nowTime - oneDay * 10);
        handle.insertVideo("vid_303", suiDataSql::videoStatus::videoStatusApproved, 
                        "给系统加上模糊搜索功能", "基于C++和ES搭建企业级搜索中台，支持复杂排序", nowTime - oneHour * 5);
        handle.insertVideo("vid_304", suiDataSql::videoStatus::videoStatusApproved, 
                        "搜索系统性能调优指南", "如何解决Elasticsearch高频查询带来的CPU毛刺问题", nowTime - 600); // 10分钟前
        handle.insertVideo("vid_305", suiDataSql::videoStatus::videoStatusApproved, 
                        "伊拉斯提克搜索集群运维", "Elasticsearch (ES) 线上环境部署与监控实战（含音译词测试）", nowTime); // 刚刚发布

        // 【集群 D：日常/游戏/其他】 (测试干扰项)
        handle.insertVideo("vid_401", suiDataSql::videoStatus::videoStatusApproved, 
                        "程序员的周末Vlog", "周末不写C++代码，去户外徒步放松一下心情", nowTime - oneDay * 5);
        handle.insertVideo("vid_402", suiDataSql::videoStatus::videoStatusApproved, 
                        "黑神话悟空通关实录", "国产3A大作太惊艳了，底层据说也有深度优化的C++引擎技术", nowTime - oneDay * 3);
        handle.insertVideo("vid_403", suiDataSql::videoStatus::videoStatusApproved, 
                        "今天吃点好的，探店必吃榜", "干饭人的日常，和代码架构无关，就是香", nowTime - oneHour * 24);

        // 【集群 E：非正常状态视频】 (绝对不应该被搜出来)
        handle.insertVideo("vid_901", suiDataSql::videoStatus::videoStatusPendingReview, 
                        "C++黑客免杀木马实战", "正在审核中的敏感视频，测试ES过滤条件", nowTime);
        handle.insertVideo("vid_902", suiDataSql::videoStatus::videoStatusReject, 
                        "Redis免费破解版下载", "已经被驳回的违规搬运视频，不该出现在列表", nowTime - oneDay);
        handle.insertVideo("vid_903", suiDataSql::videoStatus::videoStatusRemove, 
                        "Elasticsearch早期教程(已过时)", "已经被UP主自己下架的老视频", nowTime - oneMonth * 24);
    }
}

//进行视频更新
void updateRandomVideo(suiVideoSearch::videoSearch& handle)
{
    handle.updateVideo("vid_101", 5000000, 250000); // C++入门：500万播放
    handle.updateVideo("vid_301", 1200000, 80000);  // ES入门：120万播放

    // --- 高赞硬核 (播放适中，但点赞率畸高，用来测点赞排序) ---
    handle.updateVideo("vid_103", 80000, 40000);    // C++内存池：点赞率50%
    handle.updateVideo("vid_202", 50000, 30000);    // Redis源码：点赞率60%

    // --- 近期热门 (各项数据都在快速上升) ---
    handle.updateVideo("vid_104", 300000, 45000);   // 为什么用C++
    handle.updateVideo("vid_201", 850000, 60000);   // 缓存三连击
    handle.updateVideo("vid_402", 2000000, 150000); // 黑神话悟空：近期大爆款

    // --- 垂直领域交叉实战 (数据中等) ---
    handle.updateVideo("vid_102", 150000, 12000);   // C++新特性
    handle.updateVideo("vid_203", 60000,  3000);    // Redis集群搭建
    handle.updateVideo("vid_204", 45000,  4000);    // 千万级弹幕架构
    handle.updateVideo("vid_302", 25000,  2000);    // ES倒排索引
    handle.updateVideo("vid_303", 30000,  1500);    // 模糊搜索功能开发

    // --- 刚发布/数据差的底层视频 ---
    handle.updateVideo("vid_105", 5000,   100);     // C加加八股文
    handle.updateVideo("vid_304", 500,    20);      // 搜索调优
    handle.updateVideo("vid_305", 50,     5);       // 音译ES运维
    handle.updateVideo("vid_401", 3000,   150);     // 程序员Vlog
    handle.updateVideo("vid_403", 1000,   30);      // 探店美食
}

//开始模糊查询
void queryRandomVideo(suiVideoSearch::videoSearch& handle)
{
    {
        INFO("开始以C为关键字查询");
        size_t count = 0;
        auto ret = handle.searchVideo("C", 0, 1000, count);
        INFO("查询完成，结果如下: ");
        for(auto it : ret)
        {
            INFO("视频id: {}", it);
        }
    }
}

int main(int argc, char* argv[])
{
    //初始化日志
    suiUtil::suiLogInitDefault();
    //解析gflags
    google::ParseCommandLineFlags(&argc, &argv, true);
    //连接es
    auto clientPtr = std::shared_ptr<suies::esClient>(new suies::esClient({"http://elastic:suisuipingan@127.0.0.1:8085/"}));
    suiVideoSearch::videoSearch videoSearch(clientPtr);
    INFO("连接成功");
    {
        //videoSearch.initIndex();
        //INFO("索引初始化成功， 开始添加随机视频");
        //uploadRandomVideo(videoSearch);
        //INFO("测试数据添加成功, 开始更新播放量和点赞量");
        //updateRandomVideo(videoSearch);
        INFO("测试数据更新成功, 开始模糊查询");
        queryRandomVideo(videoSearch);
    }
    return 0;
}
