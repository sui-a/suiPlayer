#pragma once
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <memory>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include "suiTime.hpp"

namespace suiDataSql{
    //定义数据结构
    //会话信息存储
    #pragma db object table("tbl_session_meta")
    class suiSessionMeta
    {
    public:
        using ptr = std::shared_ptr<suiSessionMeta>;
        suiSessionMeta() = default;
        suiSessionMeta(const std::string& session_id, const std::string& user_id);
        suiSessionMeta(const std::string& session_id);
        ~suiSessionMeta();

        unsigned long long getPrimaryKey() const;
        std::string getSessionId() const;
        odb::nullable<std::string> getUserId() const;
        std::uint64_t getUploadTime() const;

        void setSessionId(const std::string& session_id);
        void setUserId(odb::nullable<std::string> user_id);
        void setUploadTime();
        void setUploadTime(std::uint64_t upload_time);

    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey;
        #pragma db not_null index unique type("VARCHAR(128)")
        std::string _session_id;
        #pragma db index type("VARCHAR(128)")
        odb::nullable<std::string> _user_id;
        #pragma db type("BIGINT UNSIGNED NOT NULL DEFAULT 0")
        std::uint64_t _upload_time;

        #pragma db index("session_id_idx") member(_session_id) //用于根据会话id查询
        #pragma db index("user_id_idx") member(_user_id) //用于根据用户id查询
    };

    //会话状态
    enum class SessionStatus
    {
        //未知状态，用于初始化
        Unknown = 0, //说明出现错误
        //临时会话
        Guest = 1,
        //正式会话
        Normal = 2,
    };

    namespace suiSessionStatus
    {
        //切换成string类型
        std::string toString(SessionStatus status);
    }

    //构造视图
    #pragma db view object(suiSessionMeta = session) query((?))
    struct suiSessionPtr
    {
    public:
        using ptr = std::shared_ptr<suiSessionPtr>;
        std::shared_ptr<suiSessionMeta> session;
    };

    enum class fileStatus
    {
        //未知状态
        fileStatusUnknown = 0,
        //在上传状态
        fileStatusUploading = 1,
        //上传成功
        fileStatusSuccess = 2,
        //被下架
        fileStatusDown = 3,
    };

    //文件元数据表
    #pragma db object table("tbl_file_meta")
    class suiFileMeta
    {
    public:
        using ptr = std::shared_ptr<suiFileMeta>;

        suiFileMeta(const std::string& file_id, const std::string& _upload_user_id);
        suiFileMeta(const std::string& file_id, const std::string& _upload_user_id, const std::string& path, size_t size, const std::string& mime_type);
        suiFileMeta(const std::string& file_id);
        suiFileMeta();
        ~suiFileMeta();

        const std::string& getFileId() const;
        const std::string& getUploadUserId() const;
        odb::nullable<std::string> getPath() const;
        size_t getSize() const;
        odb::nullable<std::string> getMimeType() const;
        std::string getUploadTimeString() const;
        std::uint64_t getUploadTime() const;
        fileStatus getFileStatus() const;
        
        void setFileId(const std::string& file_id);
        void setPath(const std::string& path);
        void setSize(size_t size);
        void setMimeType(const std::string& mime_type);
        void setUploadUserId(const std::string& upload_user_id);
        void setUploadTime();
        void setFileStatus(fileStatus status);

    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey;
        #pragma db not_null index unique type("VARCHAR(128)")
        std::string _file_id;  //非空索引
        #pragma db not_null index type("VARCHAR(128)")
        std::string _upload_user_id; //外键效率低 通过手动维护
        #pragma db type("VARCHAR(512)")
        odb::nullable<std::string> _path; //文件存储路径
        size_t _size = 0;  //文件大小
        #pragma db type("VARCHAR(64)")
        odb::nullable<std::string> _mime_type; //文件mime类型
        #pragma db type("BIGINT UNSIGNED NOT NULL DEFAULT 0")
        std::uint64_t _upload_time;
        //文件状态
        int _file_status;

        #pragma db index("file_id_idx") member(_file_id) //用于根据文件id查询
        #pragma db index("upload_user_id_idx") member(_upload_user_id) //用于根据上传用户id查询
    };
    

    //身份类型
    enum class identityType : uint8_t 
    {
        //未知身份
        identityTypeUnknown = 0,
        //普通用户
        identityTypeNormal = 1, //C端用户
        //高权限用户 
        identityTypeAdmin = 2, //B端用户
    };

    namespace suiIdentityType
    {
        //身份字符串切换
        std::string identityTypeToString(suiDataSql::identityType identity_type);
        suiDataSql::identityType stringToIdentityType(const std::string& identityTypeStr);
        //比较身份
        bool comparePermission(suiDataSql::identityType identity_type, suiDataSql::identityType identity_type2);
    }

    //用户身份映射表
    #pragma db object table("tbl_identity_meta")
    class suiIdentityMeta
    {
    public:
        using ptr = std::shared_ptr<suiIdentityMeta>;

        suiIdentityMeta();

        void setIdentityId(const std::string& curid);
        void setIdentityType(identityType identity_type);
        void setIdentityDescription(const std::string& identity_description);

        const std::string& getIdentityId();
        identityType getIdentityType();
        odb::nullable<std::string> getIdentityDescription();
        
    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null type("VARCHAR(128)")
        std::string _identity_id; //身份id
        #pragma db not_null type("TINYINT UNSIGNED")
        identityType _identityType; //身份类型
        #pragma db type("VARCHAR(64)")
        odb::nullable<std::string> _identity_description; //身份备注

        //根据身份类型查询
        #pragma db index("identity_type_idx") member(_identityType) //用于根据身份类型查询
    };

    //角色类型
    enum class roleType : uint8_t 
    {
        //未知角色
        roleTypeUnknown = 0,
        //普通角色
        roleTypeNormal = 1,
        //管理员角色
        roleTypeAdmin = 2,
        //超级管理员角色
        roleTypeSuperAdmin = 3, //最高级权限角色
    };

    namespace suiRoleType
    {
        std::string roleTypeToString(suiDataSql::roleType role_type);
        suiDataSql::roleType stringToRoleType(const std::string& roleTypeStr);
        //获取权限级别
        int getWeight(suiDataSql::roleType roleId);
        //比较
        bool comparePermission(suiDataSql::roleType role1, suiDataSql::roleType role2);
    }

    //角色信息表
    #pragma db object table("tbl_role_meta")
    class suiRoleMeta
    {
    public:
        using ptr = std::shared_ptr<suiRoleMeta>;

        suiRoleMeta();

        void setRoleId(const std::string& curid);
        void setRoleType(roleType role_type);
        void setRoleDescription(const std::string& role_description);

        const std::string& getRoleId();
        roleType getRoleType();
        odb::nullable<std::string> getRoleDescription();
    private:
        friend class odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null type("VARCHAR(128)")
        std::string _role_id; //角色id
        #pragma db not_null type("TINYINT UNSIGNED")
        roleType _role_type; //角色类型
        #pragma db type("VARCHAR(64)")
        odb::nullable<std::string> _role_description; //角色备注

        #pragma db index("role_type_idx") member(_role_type) //用于根据角色类型查询
    };

    //操作元信息表
    #pragma db object table("tbl_operation_meta")
    class suiOperationMeta
    {
    public:
        using ptr = std::shared_ptr<suiOperationMeta>;

        suiOperationMeta();

        void setOperationId(const std::string& curid);
        void setOperationUrl(const std::string& operation_url);
        void setOperationDescription(const std::string& operation_description);

        const std::string& getOperationId();
        const std::string& getOperationUrl();
        odb::nullable<std::string> getOperationDescription();
    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null type("VARCHAR(128)")
        std::string _operation_id; //操作id
        #pragma db not_null type("VARCHAR(255)")
        std::string _operation_url; //操作url
        #pragma db type("VARCHAR(64)")
        odb::nullable<std::string> _operation_description; //操作备注

        #pragma db index("operation_url_idx") member(_operation_url) //用于根据操作url查询
    };

    enum class userStatus : uint8_t 
    {
        //未知用户状态
        userStatusUnknow = 0,
        userStatusEnable = 1, //使能状态
        userStatusDisable = 2, //被禁用状态
    };

    //用户状态转字符
    namespace userStatusUtil
    {
        std::string userStatusToString(userStatus user_status);
        userStatus userStatusFromString(const std::string& userStatusStr);
    }

    //用户表
    #pragma db object table("tbl_user_meta")
    class suiUsrMeta
    {
    public:
        using ptr = std::shared_ptr<suiUsrMeta>;

        suiUsrMeta();
        //其余默认值
        suiUsrMeta(const std::string& uid, const std::string& email);

        void setUserId(const std::string& user_id);
        void setBindEmail(const std::string& bind_email);
        void setUserName(const std::string& user_name);
        void setAdministratorName(const odb::nullable<std::string>& administrator_name);
        void setPassword(const odb::nullable<std::string>& password);
        void setHeadImageFileId(const odb::nullable<std::string>& head_image_file_id);
        void setUserDescription(const odb::nullable<std::string>& user_description);
        void setUserStatus(userStatus user_status);
        void setUploadTime(std::uint64_t upload_time);
        void setUploadTime();

        const std::string& getUserId();
        const std::string& getBindEmail();
        const std::string& getUserName();
        odb::nullable<std::string> getAdministratorName();
        odb::nullable<std::string> getPassword();
        odb::nullable<std::string> getHeadImageFileId();
        odb::nullable<std::string> getUserDescription();
        userStatus getUserStatus();
        std::uint64_t getUploadTime();
    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null type("VARCHAR(128)")
        std::string _user_id; //用户id
        #pragma db not_null type("VARCHAR(64)")
        std::string _bind_email; //绑定邮箱
        #pragma db not_null type("VARCHAR(64)")
        std::string _user_name; //用户名称 C端显示普通用户名
        #pragma db type("VARCHAR(64)")
        odb::nullable<std::string> _administrator_name; //管理员名称 B端显示管理员用户名
        #pragma db type("VARCHAR(64)")
        odb::nullable<std::string> _password; //密码
        #pragma db type("VARCHAR(512)")
        odb::nullable<std::string> _head_image_file_id; //头像文件id
        #pragma db type("VARCHAR(128)")
        odb::nullable<std::string> _user_description; //管理员用户备注描述
        #pragma db not_null type("TINYINT UNSIGNED")
        userStatus _user_status; //用户状态
        #pragma db not_null type("BIGINT UNSIGNED NOT NULL DEFAULT 0")
        std::uint64_t _upload_time;

        #pragma db index("user_id_idx") member(_user_id) //用于根据用户id查询
        #pragma db index("bind_email_idx") member(_bind_email) //用于根据绑定邮箱查询
        #pragma db index("user_name_idx") member(_user_name) //用于根据用户名查询
        #pragma db index("administrator_name_idx") member(_administrator_name) //用于根据管理员用户名查询
        #pragma db index("user_status_idx") member(_user_status) //用于根据用户状态查询
    };

    //盐信息表
    #pragma db object table("tbl_salt_meta")
    class suiSaltMeta
    {
    public:
        using ptr = std::shared_ptr<suiSaltMeta>;

        suiSaltMeta();
        ~suiSaltMeta() = default;

        void setUserId(const std::string& user_id);
        void setSalt(const std::string& salt);
        void setSalt(const std::vector<char>& salt);

        const std::string& getUserId();
        const std::vector<char>& getSalt();
    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null type("VARCHAR(128)")
        std::string _user_id; //用户id
        #pragma db not_null type("VARBINARY(64)")
        std::vector<char> _salt; //盐
    };

    //用户身份角色关系表
    #pragma db object table("tbl_user_identity_role_meta")
    class suiUserIdIdentityRoleMeta
    {
    public:
        using ptr = std::shared_ptr<suiUserIdIdentityRoleMeta>;

        suiUserIdIdentityRoleMeta();

        void setUserId(const std::string& user_id);
        void setRoleType(roleType role_type);
        void setIdentityType(identityType identity_type);

        const std::string& getUserId();
        roleType getRoleType();
        identityType getIdentityType();
    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null type("VARCHAR(128)")
        std::string _user_id; //用户id
        #pragma db not_null type("TINYINT UNSIGNED")
        roleType _role_type; //角色类型
        #pragma db not_null type("TINYINT UNSIGNED")
        identityType _identity_type; //身份类型

        #pragma db index("user_id_idx") member(_user_id) //用于根据用户id查询
    };

    //权限角色操作关系表
    #pragma db object table("tbl_role_operation_meta")
    class suiRoleOperationMeta
    {
    public:
        using ptr = std::shared_ptr<suiRoleOperationMeta>;

        suiRoleOperationMeta();

        void setRoleType(roleType role);
        void setOperationUrl(const std::string& id);

        roleType getRoleType();
        const std::string& getOperationUrl();
    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null type("TINYINT UNSIGNED")
        roleType _role_type; //角色类型
        #pragma db not_null type("VARCHAR(128)")
        std::string _operation_url; //操作url

        //创建索引
        #pragma db index("operation_id_idx") member(_operation_url) //用于根据操作查询
        #pragma db index("role_op_unique_idx") unique member(_role_type) member(_operation_url) //用于根据角色类型查询
    };

    //用户关注关系表
    #pragma db object table("tbl_user_follow_meta")
    class suiUserFollowMeta
    {
    public:
        using ptr = std::shared_ptr<suiUserFollowMeta>;

        suiUserFollowMeta();

        void setUserId(const std::string& user_id);
        void setFollowUserId(const std::string& follow_user_id);

        const std::string& getUserId();
        const std::string& getFollowUserId();
    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null type("VARCHAR(128)")
        std::string _user_id; //粉丝id
        #pragma db not_null type("VARCHAR(128)")
        std::string _follow_user_id; //用户id

        //创建索引
        #pragma db index("follow_user_id_idx") member(_follow_user_id)
        #pragma db index("user_follow_unique_idx") unique member(_user_id) member(_follow_user_id)
    };

    //用户关注数量或被关注数量视图
    #pragma db view object(suiUserFollowMeta)
    struct suiUserFollowCountView
    {
        #pragma db column("count(*)")
        std::size_t count;
    };

    enum class videoStatus : uint8_t
    {
        videoStatusUnknow = 0, //未知
        videoStatusUpload = 1, //处于正在上传状态
        videoStatusTranscoding = 2, //转码中
        videoStatusPendingReview = 3, //待审核
        videoStatusApproved = 4, //被认可
        videoStatusReject = 5, //被驳回
        videoStatusRemove = 6, //下架
    };

    namespace videoStatusUtil
    {
        std::string toStringFromEnum(videoStatus video_status);
        videoStatus getEnumFromString(const std::string& video_status);

        int getIntFromEnum(videoStatus video_status);
        videoStatus getEnumFromInt(int video_status);
    }

    //视频元信息表
    #pragma db object table("tbl_video_meta")
    class suiVideoMeta
    {
    public:
        using ptr = std::shared_ptr<suiVideoMeta>;

        suiVideoMeta();

        void setVideoId(const std::string& video_id);
        void setVideoFileId(const std::string& video_file_id);
        void setVideoCoverFileId(const std::string& video_cover_file_id);
        void setUploadUserId(const std::string& upload_user_id);
        void setReviewUserId(const odb::nullable<std::string>& review_user_id);
        void setVideoName(const std::string& video_name);
        void setVideoDescription(const odb::nullable<std::string>& video_description);
        void setVideoPlayCount(unsigned int video_play_count);
        void setVideoSize(uint64_t video_size);
        void setVideoDuration(uint64_t video_duration);
        void setVideoUploadTime(uint64_t video_upload_time);
        void setVideoStatus(videoStatus video_status);

        const std::string& getVideoId();
        const std::string& getVideoFileId();
        const std::string& getVideoCoverFileId();
        const std::string& getUploadUserId();
        const odb::nullable<std::string>& getReviewUserId();
        const std::string& getVideoName();
        const odb::nullable<std::string>& getVideoDescription();
        unsigned int getVideoPlayCount();
        uint64_t getVideoSize();
        uint64_t getVideoDuration();
        uint64_t getVideoUploadTime();
        videoStatus getVideoStatus();

    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null unique type("VARCHAR(128)")
        std::string _video_id; //视频id
        #pragma db not_null type("VARCHAR(128)")
        std::string _video_file_id; //视频文件id
        #pragma db not_null type("VARCHAR(128)")
        std::string _video_cover_file_id; //视频封面文件id
        #pragma db not_null type("VARCHAR(128)")
        std::string _upload_user_id; //上传用户id
        #pragma db type("VARCHAR(128)")
        odb::nullable<std::string> _review_user_id; //审核用户id
        #pragma db not_null type("VARCHAR(128)")
        std::string _video_name; //视频标题
        #pragma db type("TEXT")
        odb::nullable<std::string> _video_description; //视频描述
        #pragma db not_null type("INT UNSIGNED") options("DEFAULT 0")
        unsigned int _video_play_count; //视频播放量
        #pragma db not_null type("BIGINT UNSIGNED")
        uint64_t _video_size; //视频大小
        #pragma db not_null type("BIGINT UNSIGNED")
        uint64_t _video_duration; //视频时长
        #pragma db not_null type("BIGINT UNSIGNED")
        uint64_t _video_upload_time; //视频上传时间
        #pragma db not_null type("TINYINT UNSIGNED") options("DEFAULT 0")
        videoStatus _video_status; //视频状态

        //创建索引
        #pragma db index("video_id_idx") member(_video_id)
        #pragma db index("upload_user_id_idx") member(_upload_user_id)
        #pragma db index("review_user_id_idx") member(_review_user_id)
        #pragma db index("video_name_idx") member(_video_name)
        #pragma db index("video_status_idx") member(_video_status)
    };

    struct suiVideoMetaList
    {
        using ptr = std::shared_ptr<suiVideoMetaList>;
        size_t total; //总数
        std::vector<suiVideoMeta> list; //视频列表
    };

    //视频Id列表
    #pragma db view object(suiVideoMeta) query((?))
    struct VideoIdList 
    {
        using ptr = std::shared_ptr<VideoIdList>;
        #pragma db column(suiVideoMeta::_video_id)
        std::string video_id;
        #pragma db column(suiVideoMeta::_video_upload_time)
        std::uint64_t order_field;
    };


    //视频数量视图
    #pragma db view object(suiVideoMeta)
    struct suiVideoCountView
    {
        #pragma db column("count(*)")
        std::size_t count;
    };

    //⽤⼾点赞关联表
    #pragma db object table("tbl_user_like_meta")
    class suiUserLikeMeta
    {
    public:
        using ptr = std::shared_ptr<suiUserLikeMeta>;

        suiUserLikeMeta();

        void setUserId(const std::string& user_id);
        void setVideoId(const std::string& video_id);

        const std::string& getUserId();
        const std::string& getVideoId();
    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null type("VARCHAR(128)")
        std::string _user_id; //用户id
        #pragma db not_null type("VARCHAR(128)")
        std::string _video_id; //视频id

        //创建索引
        #pragma db index("video_id_idx") member(_video_id)
        #pragma db index("user_like_unique_idx") unique member(_user_id) member(_video_id)
    };

    //某用户点赞了的视频列表
    struct suiUserLikeMetaList
    {
        using ptr = std::shared_ptr<suiUserLikeMetaList>;
        size_t total; //总数
        std::vector<suiUserLikeMeta> list; //某用户点赞了的视频列表
    };

    //视频点赞总量视图
    #pragma db view object(suiUserLikeMeta) \
                    object(suiVideoMeta : suiUserLikeMeta::_video_id == suiVideoMeta::_video_id)
    struct suiUserLikeCountView
    {
        //点赞总量
        #pragma db column("COUNT(*)")
        size_t count;
    };

    //视频播放总量
    #pragma db view object(suiVideoMeta) query((?))
    struct suiVideoPlayCountView
    {
        //播放总量
        #pragma db column("IFNULL(SUM(video_play_count), 0)")
        size_t count;
    };

    //用户主页基础数据
    struct UserHomepageBasicData 
    {
        //粉丝数
        size_t fansCount;
        //关注数
        size_t followCount;
        //视频点赞总量
        size_t videoLikeCount;
        //视频播放总量
        size_t videoPlayCount;

        using ptr = std::shared_ptr<UserHomepageBasicData>;
    };

    //总数视图
    #pragma db view object(suiUsrMeta) object(suiUserIdIdentityRoleMeta: suiUserIdIdentityRoleMeta::_user_id == suiUsrMeta::_user_id) query((?))
    struct userInfoTotalView
    {
        //总数
        #pragma db column("COUNT(*)")
        size_t total;
    };

    //批量返回用户info列表
    struct userInfoList
    {
        using ptr = std::shared_ptr<userInfoList>;

        //用户信息列表
        std::vector<suiUsrMeta> list;
        //总数
        size_t total = 0;
    };

    //联查用户表与权限身份映射表
    #pragma db view object(suiUsrMeta) \
            object(suiUserIdIdentityRoleMeta: suiUserIdIdentityRoleMeta::_user_id == suiUsrMeta::_user_id)\
            query((?))
    struct suiUserIdIdentityRoleView
    {
        using ptr = std::shared_ptr<suiUserIdIdentityRoleView>;
        //用户基础信息
        suiUsrMeta::ptr usrMeta;
    };

    class SqlPaginationUtil
    {
    public:
        static std::string buildPageClause(int pageSize, int page);
        static std::string buildOrderByClause(const std::string& orderByColumn, bool isDesc = false);
    };


    //验证码结构体
    struct suiVirfyCoder
    {
        using ptr = std::shared_ptr<suiVirfyCoder>;

        //会话id
        std::string _session_id;
        //验证码id
        std::string _code_id;
        //验证码本体
        std::string _code;
    };

    //视频标签映射表
    #pragma db object table("tbl_tag_meta")
    class suiTagMeta
    {
    public:
        using ptr = std::shared_ptr<suiTagMeta>;

        suiTagMeta();
        void setTagDescription(const std::string& description);
        void setTagId(long long tag_id);

        long long getTagId();
        const std::string& getTagDescription();
    private:
        friend class ::odb::access;
        #pragma db id auto type("BIGINT") 
        long long  _tag_id; //自增标签id主键
        #pragma db not_null type("VARCHAR(32)")
        std::string _tag_description; //标签描述

        //创建索引
        #pragma db index("description_idx") member(_tag_description)
        //主键固定存在主索引
    };

    //所有标签列表
    struct videoTagList
    {
        using ptr = std::shared_ptr<videoTagList>;
        std::vector<std::pair<long long, std::string>> list;
    };

    //视频分类映射表
    #pragma db object table("tbl_video_category_meta")
    class suiVideoTagMeta
    {
    public:
        using ptr = std::shared_ptr<suiVideoTagMeta>;

        suiVideoTagMeta();

        void setVideoId(const std::string& video_id);
        void setTagId(long long tag_id);

        const std::string& getVideoId();
        long long getTagId();
    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null type("VARCHAR(128)")
        std::string _video_id; //视频id
        #pragma db not_null type("BIGINT")
        long long _tag_id; //分类id

        //创建索引
        #pragma db index("video_id_idx") member(_video_id)
        //复合索引去重操作,并以标签id为第一索引字段，视频id为第二索引字段
        #pragma db index("tag_id_idx") unique member(_tag_id) member(_video_id)
    };

    //标签表与视频分类映射表视图
    #pragma db view object(suiTagMeta) \
            object(suiVideoTagMeta: suiVideoTagMeta::_tag_id == suiTagMeta::_tag_id)\
            query((?))
    struct videoTagView
    {
        using ptr = std::shared_ptr<videoTagView>;
        //标签基础信息
        suiVideoTagMeta::ptr videoMeta;
    };

    //视频分类与视频元信息映射视图，返回视频id与上传时间
    #pragma db view object(suiVideoMeta) \
            object(suiVideoTagMeta: suiVideoTagMeta::_video_id == suiVideoMeta::_video_id)\
            query((?))
    struct TaggedVideoItem 
    {
        using ptr = std::shared_ptr<TaggedVideoItem>;
        #pragma db column(suiVideoMeta::_video_id)
        std::string video_id;
        #pragma db column(suiVideoMeta::_video_upload_time)
        std::uint64_t order_field;
    };

    //视频数量视图
    #pragma db view object(suiVideoMeta) \
            object(suiVideoTagMeta: suiVideoTagMeta::_video_id == suiVideoMeta::_video_id)\
            query((?))
    struct TaggedVideoCount
    {
        #pragma db column("count(*)")
        std::size_t count;
    };

    #pragma db view object(suiTagMeta) \
            object(suiVideoTagMeta: suiVideoTagMeta::_tag_id == suiTagMeta::_tag_id)\
            query((?))
    struct tagVideoView
    {
        using ptr = std::shared_ptr<tagVideoView>;
        //标签基础信息
        suiTagMeta::ptr tags;
    };

    //获取总数
    #pragma db view object(suiTagMeta) \
            object(suiVideoTagMeta: suiVideoTagMeta::_tag_id == suiTagMeta::_tag_id)\
            query((?))
    struct videoTagTotal
    {
        using ptr = std::shared_ptr<videoTagTotal>;
        #pragma db column("COUNT(*)")
        size_t total;
    };

    struct suiVideoListByTag
    {
        using ptr = std::shared_ptr<suiVideoListByTag>;
        //视频分类列表
        std::vector<suiVideoTagMeta> list;
        //总数
        size_t total = 0;
    };

    struct suiGetTagsByVideoRsp
    {
        using ptr = std::shared_ptr<suiGetTagsByVideoRsp>;
        //标签列表
        std::vector<suiTagMeta> list;
    };

    //视频弹幕信息表
    #pragma db object table("tbl_video_subtitle_target")
    class suiVideoSubtitleTarget
    {
    public:
        using ptr = std::shared_ptr<suiVideoSubtitleTarget>;
        suiVideoSubtitleTarget();

        void setVideoId(const std::string& video_id);
        void setUserId(const std::string& user_id);
        void setBulletchatId(const std::string& bulletchat_id);
        void setBulletchat(const std::string& bulletchat);
        void setSendByVideoTime(std::uint64_t send_by_video_time);
        void setCreateTime(std::uint64_t create_time);
        void setCreateTime();

        const std::string& getVideoId();
        const std::string& getUserId();
        const std::string& getBulletchatId();
        const std::string& getBulletchat();
        std::uint64_t getSendByVideoTime();
        std::uint64_t getCreateTime();
    private:
        friend class ::odb::access;
        #pragma db id auto 
        unsigned long long  _primaryKey; //主键
        #pragma db not_null type("VARCHAR(128)")
        std::string _bulletchat_id; //弹幕id
        #pragma db not_null type("VARCHAR(128)")
        std::string _video_id; //视频id
        #pragma db not_null type("VARCHAR(128)")
        std::string _user_id; //用户id
        #pragma db type("TEXT")
        std::string _bulletchat; //弹幕内容
        #pragma db type("BIGINT UNSIGNED NOT NULL DEFAULT 0")
        std::uint64_t _send_by_video_time; //发送的时间
        #pragma db type("BIGINT UNSIGNED NOT NULL DEFAULT 0")
        std::uint64_t _create_time; //创建时间

        //创建索引
        //根据视频id查询 再就是 偏移时间查询
        #pragma db index("video_id_idx") member(_video_id) member(_send_by_video_time)
        //根据用户id查询
        #pragma db index("user_id_idx") member(_user_id)
    };

    //
    struct videoBulletChatList
    {
        using ptr = std::shared_ptr<videoBulletChatList>;
        std::vector<suiVideoSubtitleTarget> list;
    };


}