TRUNCATE TABLE `tbl_user_meta`;

INSERT INTO `tbl_user_meta` 
(`user_id`, `bind_email`, `user_name`, `user_status`, `upload_time`) 
VALUES
('user_A', 'a@test.com', 'Alice (圈子)', 1, 1700000001),
('user_B', 'b@test.com', 'Bob (圈子)',   1, 1700000002),
('user_C', 'c@test.com', 'Carol (圈子)', 1, 1700000003),
('user_D', 'd@test.com', 'Dave (普通)',  1, 1700000004),
('user_E', 'e@test.com', 'Eve (大V)',    1, 1700000005),
('user_F', 'f@test.com', 'Frank (海王)', 1, 1700000006),
('user_G', 'g@test.com', 'Grace (透明)', 1, 1700000007);