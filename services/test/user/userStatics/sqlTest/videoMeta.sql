INSERT INTO `tbl_video_meta` 
(`video_id`, `video_file_id`, `video_cover_file_id`, `upload_user_id`, `review_user_id`, `video_name`, `video_description`, `video_play_count`, `video_size`, `video_duration`, `video_upload_time`, `video_status`) 
VALUES
-- 【大V Eve 的视频】：播放量极高，爆款视频
('video_001', 'file_v1', 'cover_v1', 'user_E', 'admin_01', '粉丝突破百万感谢！', '感谢大家一路以来的支持~', 850000, 150485760, 300, 1700005000, 1),
('video_002', 'file_v2', 'cover_v2', 'user_E', 'admin_01', '大V的日常Vlog', '今天也是元气满满的一天', 620000, 204857600, 600, 1700006000, 1),

-- 【圈子 A/B/C 的视频】：播放量中等，主要在小圈子内传播
('video_003', 'file_v3', 'cover_v3', 'user_A', 'admin_01', '周末和Bob/Carol去探店', '这家咖啡厅绝了！', 1500, 50485760, 120, 1700007000, 1),
('video_004', 'file_v4', 'cover_v4', 'user_B', 'admin_01', '新买的机械键盘开箱', '红轴永远滴神', 800, 30485760, 180, 1700008000, 1),
('video_005', 'file_v5', 'cover_v5', 'user_C', 'admin_01', '翻唱一首最近很火的歌', '轻喷~', 2100, 40485760, 240, 1700009000, 1),

-- 【普通人 Dave 的视频】：正常播放量
('video_006', 'file_v6', 'cover_v6', 'user_D', 'admin_01', '记录写代码到头秃的一天', '又是一个没有解决的Bug', 350, 25485760, 90, 1700010000, 1),

-- 【海王 Frank 的视频】：疯狂发视频，质量参差不齐，部分还在审核中
('video_007', 'file_v7', 'cover_v7', 'user_F', 'admin_01', '教你如何做好时间管理', '懂的都懂', 5000, 60485760, 360, 1700011000, 1),
-- 修复处：将 NULL 改为空字符串 ''
('video_008', 'file_v8', 'cover_v8', 'user_F', '', '深夜emo，谁来陪我', '随便录的', 0, 10485760, 30, 1700012000, 0), 

-- 【透明人 Grace 的视频】：真实的“零播放”惨案
('video_009', 'file_v9', 'cover_v9', 'user_G', 'admin_01', '今天天气真好', '没人看我也要发', 0, 5485760, 15, 1700013000, 1);