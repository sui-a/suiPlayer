INSERT INTO `tbl_user_like_meta` (`user_id`, `video_id`) VALUES
-- 【大V的视频】：大家都爱看大V的视频
('user_A', 'video_001'),
('user_B', 'video_001'),
('user_C', 'video_001'),
('user_D', 'video_001'),
('user_F', 'video_001'),
('user_G', 'video_001'), -- 透明人也在默默点赞
('user_A', 'video_002'),
('user_C', 'video_002'),

-- 【圈子内部互相捧场】：A, B, C 互相点赞
('user_B', 'video_003'), -- B给A点赞
('user_C', 'video_003'), -- C给A点赞
('user_A', 'video_004'), -- A给B点赞
('user_C', 'video_004'), -- C给B点赞
('user_A', 'video_005'), -- A给C点赞
('user_B', 'video_005'), -- B给C点赞
('user_E', 'video_005'), -- 大V E 偶尔也给小圈子的 C 点了个赞（互动）

-- 【普通人的视频】
('user_A', 'video_006'), -- A觉得D的视频不错

-- 【海王 Frank 的点赞记录】：海王不仅到处关注，还到处点赞（几乎给所有妹子/人都点了赞）
('user_F', 'video_002'),
('user_F', 'video_003'),
('user_F', 'video_004'),
('user_F', 'video_005'),
('user_F', 'video_006'),
('user_F', 'video_009'), -- 甚至给透明人点了个赞，真·海王

-- 【透明人 Grace】：她的视频 video_009 只有海王 F 点了赞，其他人全没看见
('user_G', 'video_005'); -- 透明人也给C点了赞