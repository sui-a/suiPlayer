#pragma once
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <memory>
#include <string>
#include <cstddef>
#include <cstdint>

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
        std::string getUploadTimeString() const;
        std::uint64_t getUploadTime() const;

        void setSessionId(const std::string& session_id);
        void setUserId(odb::nullable<std::string> user_id);
        void setUploadTime();

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

        //角色字符串切换
    };

    namespace suiRoleType
    {
        std::string roleTypeToString(suiDataSql::roleType role_type);
        suiDataSql::roleType stringToRoleType(const std::string& roleTypeStr);
        //获取权限级别
        int getWeight(suiDataSql::roleType roleId);
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
        //普通用户
        userStatusEnable = 1, //使能状态
        //高权限用户 
        userStatusDisable = 2, //被禁用状态
    };

    //用户表
    #pragma db object table("tbl_user_meta")
    class suiUsrMeta
    {
    public:
        using ptr = std::shared_ptr<suiUsrMeta>;

        suiUsrMeta();

        void setUserId(const std::string& user_id);
        void setBindEmail(const std::string& bind_email);
        void setUserName(const std::string& user_name);
        void setAdministratorName(const std::string& administrator_name);
        void setPassword(const std::string& password);
        void setHeadImageFileId(const std::string& head_image_file_id);
        void setUserDescription(const std::string& user_description);
        void setUserStatus(userStatus user_status);
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
        odb::nullable<std::string> _user_description; //用户备注
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
        void setReviewUserId(const std::string& review_user_id);
        void setVideoName(const std::string& video_name);
        void setVideoDescription(const std::string& video_description);
        void setVideoPlayCount(unsigned int video_play_count);
        void setVideoSize(uint64_t video_size);
        void setVideoDuration(uint64_t video_duration);
        void setVideoUploadTime(uint64_t video_upload_time);
        void setVideoStatus(videoStatus video_status);

        const std::string& getVideoId();
        const std::string& getVideoFileId();
        const std::string& getVideoCoverFileId();
        const std::string& getUploadUserId();
        const std::string& getReviewUserId();
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
        std::string _review_user_id; //审核用户id
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
}