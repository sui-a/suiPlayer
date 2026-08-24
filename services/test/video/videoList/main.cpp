#include <suiScaffold/log.h>
#include <gflags/gflags.h>
#include <suiScaffold/suiSerSearch.hpp>
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiJson.hpp>
#include <suiScaffold/suiElastic.hpp>
#include "video_operation_meta.hpp"
#include "video_catgory_tag.hpp"
#include "videoListOperational.hpp"

//消息队列
DEFINE_string(amqp_addr, "amqp://sui:suisuipingan@localhost:8082//", "amqp地址");

//mysql配置
DEFINE_int32(mysql_server_listen_port, 24411, "mysql服务监听端口");
DEFINE_string(mysql_server_registry_center, "sh-cdb-l9h13in4.sql.tencentcdb.com", "mysql注册中心地址");
DEFINE_string(mysql_server_user, "sui", "mysql用户名");
DEFINE_string(mysql_server_password, "suisuipingan", "mysql密码");
DEFINE_string(mysql_server_database, "bilibili", "mysql数据库");

//redis配置
DEFINE_string(redis_server_host, "127.0.0.1", "redis主机地址");
DEFINE_int32(redis_server_port, 8081, "redis端口");
DEFINE_string(redis_server_password, "suisuipingan", "redis密码");
DEFINE_string(redis_server_user, "default", "redis用户名");

//随机插入测试用例
void seedRandomTestCases(std::shared_ptr<odb::database> _mysql, std::shared_ptr<sw::redis::Redis> _redis
                , std::shared_ptr<suiRemoveCache::RemoveCache> _cache_sync)
{
    try
    {
        //创建事务
        //创建事务
        odb::transaction tx(_mysql->begin());
        auto rtx = _redis->transaction(false, false);

        //创建绑定线程的操作句柄
        auto& dbHandler = tx.database();
        auto reHandler = rtx.redis();
        suiVideoOperation::videoOperation videoOperation(dbHandler, reHandler, _cache_sync);
        suiVideocatgoryTag::videocatgoryTag tagOperation(dbHandler);
        // ---------------- 1. 美食类视频 A ----------------
        {
            tagOperation.addTag("美食");
            tagOperation.addTag("音乐");
            tagOperation.addTag("搞笑");
            tagOperation.addTag("游戏");
            tagOperation.addTag("科技");
            tagOperation.addTag("编程");
            tagOperation.addTag("旅游");
            tagOperation.addTag("宠物");
            tagOperation.addTag("运动");
            tagOperation.addTag("影视");
            tagOperation.addTag("摄影");
            tagOperation.addTag("汽车");
            tagOperation.addTag("舞蹈");
        }

        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_001"); 
                video.setVideoFileId("file_f1a2b3"); 
                video.setVideoCoverFileId("cover_c1a2b3"); 
                video.setUploadUserId("user_101"); 
                video.setReviewUserId("sui"); 
                video.setVideoName("舌尖上的中国 - 烤鸭篇"); 
                video.setVideoDescription(odb::nullable<std::string>("带你探寻最地道的北京烤鸭制作工艺")); 
                video.setVideoSize(256001024); // 约250MB
                video.setVideoDuration(240000); // 4分钟
                video.setVideoUploadTime(1690000000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_001", tagOperation.selectTag("美食")->getTagId());
            }
        }
        // ---------------- 2. 美食类视频 B ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_002"); 
                video.setVideoFileId("file_f2b3c4"); 
                video.setVideoCoverFileId("cover_c2b3c4"); 
                video.setUploadUserId("user_102"); 
                video.setReviewUserId("admin_2"); 
                video.setVideoName("蛋炒饭的终极秘诀"); 
                video.setVideoDescription(odb::nullable<std::string>("如何用剩饭炒出粒粒分明的黄金炒饭")); 
                video.setVideoSize(12800512); 
                video.setVideoDuration(120000); 
                video.setVideoUploadTime(1690001000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_002", tagOperation.selectTag("美食")->getTagId());
            }
        }
        // ---------------- 3. 游戏类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_003"); 
                video.setVideoFileId("file_g1h2i3"); 
                video.setVideoCoverFileId("cover_c3d4e5"); 
                video.setUploadUserId("user_103"); 
                video.setReviewUserId("sui"); 
                video.setVideoName("黑神话：悟空 最终预告"); 
                video.setVideoDescription(odb::nullable<std::string>("直面天命！最新实机演示混剪")); 
                video.setVideoSize(512004048); 
                video.setVideoDuration(315000); 
                video.setVideoUploadTime(1690002000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_003", tagOperation.selectTag("游戏")->getTagId());
            }
        }

        // ---------------- 4. 游戏类视频 B ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_004"); 
                video.setVideoFileId("file_j4k5l6"); 
                video.setVideoCoverFileId("cover_m7n8o9"); 
                video.setUploadUserId("user_101"); 
                video.setReviewUserId("admin_1"); 
                video.setVideoName("塞尔达传说：王国之泪 创意载具"); 
                video.setVideoDescription(odb::nullable<std::string>("教你如何组装高达横扫海拉鲁")); 
                video.setVideoSize(89004048); 
                video.setVideoDuration(180000); 
                video.setVideoUploadTime(1690003000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_004", tagOperation.selectTag("游戏")->getTagId());
            }
        }

        // ---------------- 5. 科技类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_005"); 
                video.setVideoFileId("file_tech01"); 
                video.setVideoCoverFileId("cover_tech01"); 
                video.setUploadUserId("user_geek"); 
                video.setReviewUserId("sui"); 
                video.setVideoName("RTX 4090 首发全面测评"); 
                video.setVideoDescription(odb::nullable<std::string>("性能怪兽来袭，跑分与功耗实测")); 
                video.setVideoSize(1024005000); 
                video.setVideoDuration(900000); 
                video.setVideoUploadTime(1690004000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_005", tagOperation.selectTag("科技")->getTagId());
            }
        }
        // ---------------- 6. 科技类视频 B ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_006"); 
                video.setVideoFileId("file_tech02"); 
                video.setVideoCoverFileId("cover_tech02"); 
                video.setUploadUserId("user_apple"); 
                video.setReviewUserId("admin_2"); 
                video.setVideoName("Apple Vision Pro 深度体验"); 
                video.setVideoDescription(odb::nullable<std::string>("空间计算设备的未来在哪里？")); 
                video.setVideoSize(800050000); 
                video.setVideoDuration(750000); 
                video.setVideoUploadTime(1690005000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_006", tagOperation.selectTag("科技")->getTagId());
            }
        }

        // ---------------- 7. 编程类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_007"); 
                video.setVideoFileId("file_cpp01"); 
                video.setVideoCoverFileId("cover_cpp01"); 
                video.setUploadUserId("user_coder"); 
                video.setReviewUserId("sui"); 
                video.setVideoName("C++ 并发编程快速入门"); 
                video.setVideoDescription(odb::nullable<std::string>("std::thread 和 std::mutex 基础教程")); 
                video.setVideoSize(350020000); 
                video.setVideoDuration(1200000); 
                video.setVideoUploadTime(1690006000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_007", tagOperation.selectTag("编程")->getTagId());
            }
        }
        // ---------------- 8. 旅游类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_008"); 
                video.setVideoFileId("file_trv01"); 
                video.setVideoCoverFileId("cover_trv01"); 
                video.setUploadUserId("user_traveler"); 
                video.setReviewUserId("admin_1"); 
                video.setVideoName("周末杭州游：西湖的另一种美"); 
                video.setVideoDescription(odb::nullable<std::string>("避开人群，寻找小众打卡点")); 
                video.setVideoSize(640050000); 
                video.setVideoDuration(480000); 
                video.setVideoUploadTime(1690007000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_008", tagOperation.selectTag("旅游")->getTagId());
            }
        }

        // ---------------- 9. 旅游类视频 B ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_009"); 
                video.setVideoFileId("file_trv02"); 
                video.setVideoCoverFileId("cover_trv02"); 
                video.setUploadUserId("user_traveler"); 
                video.setReviewUserId("sui"); 
                video.setVideoName("川藏线自驾游纪录片 第一集"); 
                video.setVideoDescription(odb::nullable<std::string>("出发！从成都到理塘")); 
                video.setVideoSize(1500000000); 
                video.setVideoDuration(1500000); 
                video.setVideoUploadTime(1690008000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_009", tagOperation.selectTag("旅游")->getTagId());
            }
        }
        // ---------------- 10. 宠物类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_010"); 
                video.setVideoFileId("file_pet01"); 
                video.setVideoCoverFileId("cover_pet01"); 
                video.setUploadUserId("user_cat"); 
                video.setReviewUserId("sui"); 
                video.setVideoName("小橘猫的拆家日常"); 
                video.setVideoDescription(odb::nullable<std::string>("论一只猫能有多调皮")); 
                video.setVideoSize(120000000); 
                video.setVideoDuration(150000); 
                video.setVideoUploadTime(1690009000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_010", tagOperation.selectTag("宠物")->getTagId());
            }
        }
        // ---------------- 11. 宠物类视频 B ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_011"); 
                video.setVideoFileId("file_pet02"); 
                video.setVideoCoverFileId("cover_pet02"); 
                video.setUploadUserId("user_dog"); 
                video.setReviewUserId("admin_2"); 
                video.setVideoName("哈士奇犯错后的表情合集"); 
                video.setVideoDescription(odb::nullable<std::string>("每天一遍，防止抑郁")); 
                video.setVideoSize(85000000); 
                video.setVideoDuration(90000); 
                video.setVideoUploadTime(1690010000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_011", tagOperation.selectTag("宠物")->getTagId());
            }
        }

        // ---------------- 12. 运动类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_012"); 
                video.setVideoFileId("file_spt01"); 
                video.setVideoCoverFileId("cover_spt01"); 
                video.setUploadUserId("user_sport"); 
                video.setReviewUserId("sui"); 
                video.setVideoName("NBA 近十年百大扣篮"); 
                video.setVideoDescription(odb::nullable<std::string>("热血沸腾！飞天遁地的视觉盛宴")); 
                video.setVideoSize(750000000); 
                video.setVideoDuration(600000); 
                video.setVideoUploadTime(1690011000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_012", tagOperation.selectTag("运动")->getTagId());
            }
        }
        // ---------------- 13. 运动类视频 B ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_013"); 
                video.setVideoFileId("file_spt02"); 
                video.setVideoCoverFileId("cover_spt02"); 
                video.setUploadUserId("user_fitness"); 
                video.setReviewUserId("admin_1"); 
                video.setVideoName("10分钟马甲线速成训练"); 
                video.setVideoDescription(odb::nullable<std::string>("零基础跟练，无器械居家燃脂")); 
                video.setVideoSize(200000000); 
                video.setVideoDuration(620000); 
                video.setVideoUploadTime(1690012000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_013", tagOperation.selectTag("运动")->getTagId());
            }
        }

        // ---------------- 14. 音乐类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_014"); 
                video.setVideoFileId("file_mus01"); 
                video.setVideoCoverFileId("cover_mus01"); 
                video.setUploadUserId("user_music"); 
                video.setReviewUserId("sui"); 
                video.setVideoName("周杰伦经典歌曲串烧Live"); 
                video.setVideoDescription(odb::nullable<std::string>("青春的回忆，万人大合唱现场")); 
                video.setVideoSize(950000000); 
                video.setVideoDuration(1200000); 
                video.setVideoUploadTime(1690013000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_014", tagOperation.selectTag("音乐")->getTagId());
            }
        }

        // ---------------- 15. 音乐类视频 B ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_015"); 
                video.setVideoFileId("file_mus02"); 
                video.setVideoCoverFileId("cover_mus02"); 
                video.setUploadUserId("user_piano"); 
                video.setReviewUserId("admin_2"); 
                video.setVideoName("久石让《Summer》钢琴独奏"); 
                video.setVideoDescription(odb::nullable<std::string>("治愈系钢琴曲，戴上耳机静静聆听")); 
                video.setVideoSize(130000000); 
                video.setVideoDuration(210000); 
                video.setVideoUploadTime(1690014000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_015", tagOperation.selectTag("音乐")->getTagId());
            }
        }

        // ---------------- 16. 影视类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_016"); 
                video.setVideoFileId("file_mov01"); 
                video.setVideoCoverFileId("cover_mov01"); 
                video.setUploadUserId("user_movie"); 
                video.setReviewUserId("sui"); 
                video.setVideoName("星际穿越：五维空间解析"); 
                video.setVideoDescription(odb::nullable<std::string>("十分钟带你看懂硬核科幻神作")); 
                video.setVideoSize(320000000); 
                video.setVideoDuration(600000); 
                video.setVideoUploadTime(1690015000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_016", tagOperation.selectTag("影视")->getTagId());
            }
        }

        // ---------------- 17. 搞笑类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_017"); 
                video.setVideoFileId("file_fun01"); 
                video.setVideoCoverFileId("cover_fun01"); 
                video.setUploadUserId("user_fun"); 
                video.setReviewUserId("admin_1"); 
                video.setVideoName("全网最爆笑的人类迷惑行为大赏"); 
                video.setVideoDescription(odb::nullable<std::string>("前方高能！全程无尿点")); 
                video.setVideoSize(180000000); 
                video.setVideoDuration(300000); 
                video.setVideoUploadTime(1690016000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_017", tagOperation.selectTag("搞笑")->getTagId());
            }
        }
        tx.commit();
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("odb数据库异常： {}", e.what());
    }
    catch (const sw::redis::Error& e) 
    {
        // 捕获 Redis 异常
        ERROR("缓存库异常： {}", e.what());
    }
    catch (...)
    {
        ERROR("未知异常");
    }
}

void clearTestData(std::shared_ptr<odb::database> _mysql, std::shared_ptr<sw::redis::Redis> _redis
                , std::shared_ptr<suiRemoveCache::RemoveCache> _cache_sync)
{
    try
    {
        //创建事务
        //创建事务
        odb::transaction tx(_mysql->begin());
        auto rtx = _redis->transaction(false, false);

        //创建绑定线程的操作句柄
        auto& dbHandler = tx.database();
        auto reHandler = rtx.redis();
        suiVideoOperation::videoOperation videoOperation(dbHandler, reHandler, _cache_sync);
        {
            //添加视频元数据
            videoOperation.remove("vid_001");
            videoOperation.remove("vid_002");
            videoOperation.remove("vid_003");
            videoOperation.remove("vid_004");
            videoOperation.remove("vid_005");
            videoOperation.remove("vid_006");
            videoOperation.remove("vid_007");
            videoOperation.remove("vid_008");
            videoOperation.remove("vid_009");
            videoOperation.remove("vid_010");
            videoOperation.remove("vid_011");
            videoOperation.remove("vid_012");
            videoOperation.remove("vid_013");
            videoOperation.remove("vid_014");
            videoOperation.remove("vid_015");
            videoOperation.remove("vid_016");
            videoOperation.remove("vid_017");
            videoOperation.remove("vid_018");
            videoOperation.remove("vid_019");
            videoOperation.remove("vid_020");
        }
        suiVideocatgoryTag::videocatgoryTag tagOperation(dbHandler);
        // ---------------- 1. 美食类视频 A ----------------
        {
            tagOperation.removeTag("美食");
            tagOperation.removeTag("音乐");
            tagOperation.removeTag("搞笑");
            tagOperation.removeTag("游戏");
            tagOperation.removeTag("科技");
            tagOperation.removeTag("编程");
            tagOperation.removeTag("旅游");
            tagOperation.removeTag("宠物");
            tagOperation.removeTag("运动");
            tagOperation.removeTag("影视");
            tagOperation.removeTag("摄影");
            tagOperation.removeTag("汽车");
            tagOperation.removeTag("舞蹈");
        }
        {
            tagOperation.removeVideoTag("vid_001");
            tagOperation.removeVideoTag("vid_002");
            tagOperation.removeVideoTag("vid_003");
            tagOperation.removeVideoTag("vid_004");
            tagOperation.removeVideoTag("vid_005");
            tagOperation.removeVideoTag("vid_006");
            tagOperation.removeVideoTag("vid_007");
            tagOperation.removeVideoTag("vid_008");
            tagOperation.removeVideoTag("vid_009");
            tagOperation.removeVideoTag("vid_010");
            tagOperation.removeVideoTag("vid_011");
            tagOperation.removeVideoTag("vid_012");
            tagOperation.removeVideoTag("vid_013");
            tagOperation.removeVideoTag("vid_014");
            tagOperation.removeVideoTag("vid_015");
            tagOperation.removeVideoTag("vid_016");
            tagOperation.removeVideoTag("vid_017");
            tagOperation.removeVideoTag("vid_018");
            tagOperation.removeVideoTag("vid_019");
            tagOperation.removeVideoTag("vid_020");
        }

        tx.commit();
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("odb数据库异常： {}", e.what());
    }
    catch (const sw::redis::Error& e) 
    {
        // 捕获 Redis 异常
        ERROR("缓存库异常： {}", e.what());
    }
    catch (...)
    {
        ERROR("未知异常");
    }
}

void seedRandomTestCases2(std::shared_ptr<odb::database> _mysql, std::shared_ptr<sw::redis::Redis> _redis
                , std::shared_ptr<suiRemoveCache::RemoveCache> _cache_sync)
{
    try
    {
        odb::transaction tx(_mysql->begin());
        auto rtx = _redis->transaction(false, false);

        //创建绑定线程的操作句柄
        auto& dbHandler = tx.database();
        auto reHandler = rtx.redis();
        suiVideoOperation::videoOperation videoOperation(dbHandler, reHandler, _cache_sync);
        suiVideocatgoryTag::videocatgoryTag tagOperation(dbHandler);
        // ---------------- 18. 摄影类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_018"); 
                video.setVideoFileId("file_photo01"); 
                video.setVideoCoverFileId("cover_photo01"); 
                video.setUploadUserId("user_photo"); 
                video.setReviewUserId("sui"); 
                video.setVideoName("手机延时摄影入门教学"); 
                video.setVideoDescription(odb::nullable<std::string>("教你用手机拍出城市车流与星轨大片")); 
                video.setVideoSize(280000000); 
                video.setVideoDuration(420000); 
                video.setVideoUploadTime(1690017000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_018", tagOperation.selectTag("摄影")->getTagId());
            }
        }
        // ---------------- 19. 汽车类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_019"); 
                video.setVideoFileId("file_car01"); 
                video.setVideoCoverFileId("cover_car01"); 
                video.setUploadUserId("user_car"); 
                video.setReviewUserId("admin_2"); 
                video.setVideoName("小米 SU7 极限续航测试"); 
                video.setVideoDescription(odb::nullable<std::string>("满电能跑多远？高速实测揭晓答案")); 
                video.setVideoSize(680000000); 
                video.setVideoDuration(850000); 
                video.setVideoUploadTime(1690018000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_019", tagOperation.selectTag("汽车")->getTagId());
            }
        }

        // ---------------- 20. 舞蹈类视频 A ----------------
        {
            {
                suiDataSql::suiVideoMeta video;
                video.setVideoId("vid_020"); 
                video.setVideoFileId("file_dance01"); 
                video.setVideoCoverFileId("cover_dance01"); 
                video.setUploadUserId("user_dance"); 
                video.setReviewUserId("sui"); 
                video.setVideoName("K-pop 随机舞蹈街头挑战"); 
                video.setVideoDescription(odb::nullable<std::string>("神仙打架！路人王炸场集锦")); 
                video.setVideoSize(450000000); 
                video.setVideoDuration(500000); 
                video.setVideoUploadTime(1690019000); 
                video.setVideoStatus(suiDataSql::videoStatus::videoStatusApproved);
                videoOperation.insert(video);
                tagOperation.addVideoTag("vid_020", tagOperation.selectTag("舞蹈")->getTagId());
            }
        }
        tx.commit();
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("odb数据库异常： {}", e.what());
    }
    catch (const sw::redis::Error& e) 
    {
        // 捕获 Redis 异常
        ERROR("缓存库异常： {}", e.what());
    }
    catch (...)
    {
        ERROR("未知异常");
    }
}

//获取主页视频列表
void testGetHomeVideoList(std::shared_ptr<odb::database> _mysql, std::shared_ptr<sw::redis::Redis> _redis
                , std::shared_ptr<suiRemoveCache::RemoveCache> _cache_sync, int page, int size)
{
    try
    {
        //获取主页视频
        odb::transaction tx(_mysql->begin());
        auto rtx = _redis->transaction(false, false);
        suiVideoListOperational::videoListOperational videoListOperational(tx, rtx, _cache_sync);
        auto videoList = videoListOperational.getVideoMainList(page, size);
        for(auto& it : videoList)
            INFO("视频ID：{}", it);
        
        tx.commit();
    }
    catch (const odb::exception& e)
    {
        // 捕获 ODB 数据库异常
        ERROR("odb数据库异常： {}", e.what());
    }
    catch (const sw::redis::Error& e) 
    {
        // 捕获 Redis 异常
        ERROR("缓存库异常： {}", e.what());
    }
    catch (...)
    {
        ERROR("未知异常");
    }

}

int main(int argc, char* argv[])
{
    //初始化日志
    suiUtil::suiLogInitDefault();
    //解析gflags
    google::ParseCommandLineFlags(&argc, &argv, true);
    //配置数据库
    suiOdb::odbSetting odbset;
    odbset._host = FLAGS_mysql_server_registry_center;
    odbset._port = FLAGS_mysql_server_listen_port;
    odbset._user = FLAGS_mysql_server_user;
    odbset._password = FLAGS_mysql_server_password;
    odbset._database = FLAGS_mysql_server_database;
    //配置redis
    suiRedis::redisSettings redisset;
    redisset._host = FLAGS_redis_server_host;
    redisset._port = FLAGS_redis_server_port;
    redisset._password = FLAGS_redis_server_password;
    redisset._user = FLAGS_redis_server_user;
    //构造操作句柄
    suiQueue::MQClient::ptr _mqClienrt = std::make_shared<suiQueue::MQClient>(FLAGS_amqp_addr);
    auto _redis = suiRedis::RedisFactory::create(redisset);
    auto _mysql = suiOdb::dbFactory::create(odbset);
    auto _cache_sync = std::make_shared<suiRemoveCache::RemoveCache>(_redis, _mqClienrt);

    INFO("创建成功，开始测试");
    seedRandomTestCases(_mysql, _redis, _cache_sync);
    INFO("插入数据完成，点击回车删除数据");
    std::cin.get();
    INFO("开始获取主页视频列表, 0页， 5条");
    testGetHomeVideoList(_mysql, _redis, _cache_sync, 0, 5);
    INFO("获取主页视频列表完成, 点击回车继续测试");
    std::cin.get();
    INFO("开始再添加部分数据");
    seedRandomTestCases2(_mysql, _redis, _cache_sync);
    INFO("插入数据完成，点击回车继续测试");
    std::cin.get();
    INFO("开始获取缓存外数据");
    testGetHomeVideoList(_mysql, _redis, _cache_sync, 3, 5);
    INFO("获取缓存外数据完成, 点击回车继续测试");
    std::cin.get();
    INFO("开始获取大于缓存存储量的数据");
    testGetHomeVideoList(_mysql, _redis, _cache_sync, 0, 15);
    INFO("获取大于缓存存储量的数据完成, 点击回车继续测试");
    std::cin.get();
    clearTestData(_mysql, _redis, _cache_sync);
    INFO("测试完成，点击回车退出");
    std::cin.get();
    return 0;
}
