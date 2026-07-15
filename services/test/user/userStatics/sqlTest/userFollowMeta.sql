TRUNCATE TABLE `tbl_user_follow_meta`;


INSERT INTO `tbl_user_follow_meta` (`user_id`, `follow_user_id`) VALUES
-- 【互关小圈子 + 追星】A, B, C 互相绝对关注，并且他们都关注了大V Eve
('user_A', 'user_B'),
('user_A', 'user_C'),
('user_A', 'user_E'),
('user_B', 'user_A'),
('user_B', 'user_C'),
('user_B', 'user_E'),
('user_C', 'user_A'),
('user_C', 'user_B'),
('user_C', 'user_E'),
('user_D', 'user_E'),
('user_E', 'user_A'),
('user_F', 'user_A'),
('user_F', 'user_B'),
('user_F', 'user_C'),
('user_F', 'user_D'),
('user_F', 'user_E');