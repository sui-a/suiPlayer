#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/log.h>
#include "data.hpp"
#include <chrono>
#include <optional>
#include <ctime>
#include <iomanip>
#include <sstream>


namespace suiDataSql
{

    /*会话信息存储 ===============================================================================================*/
    suiSessionMeta::suiSessionMeta(const std::string& session_id, const std::string& user_id)
        : _session_id(session_id)
        , _user_id(user_id)
    {

    }
    suiSessionMeta::suiSessionMeta(const std::string& session_id)
        : _session_id(session_id)
    {

    }
    suiSessionMeta::~suiSessionMeta()
    {

    }

    unsigned long long suiSessionMeta::getPrimaryKey() const
    {
        return _primaryKey;
    }
    std::string suiSessionMeta::getSessionId() const
    {
        return _session_id;
    }
    odb::nullable<std::string> suiSessionMeta::getUserId() const
    {
        return _user_id;
    }
    std::string suiSessionMeta::getUploadTimeString() const
    {
        if (_upload_time == 0) return "";

        time_t time_val = static_cast<time_t>(_upload_time);
        struct tm *tm_info = localtime(&time_val);
        if (!tm_info)
        {
            ERROR("时间解析失败");
            return "";
        }

        //开始构造
        std::stringstream ss;

        ss << tm_info->tm_year + 1900 << "-" << tm_info->tm_mon + 1 << "-" << tm_info->tm_mday << ' ';
        ss << tm_info->tm_hour << ':' << tm_info->tm_min << ':' << tm_info->tm_sec;
        return ss.str();
    }

    void suiSessionMeta::setUploadTime()
    {
        _upload_time = static_cast<std::uint64_t>(std::time(nullptr));
    }
    
    std::uint64_t suiSessionMeta::getUploadTime() const
    {
        return _upload_time;
    }

    void suiSessionMeta::setSessionId(const std::string& session_id)
    {
        _session_id = session_id;
    }
    void suiSessionMeta::setUserId(odb::nullable<std::string> user_id)
    {
        _user_id = user_id;
    }
    /*会话信息存储 end ===============================================================================================*/

    /*file ===============================================================================================*/
    suiFileMeta::suiFileMeta(const std::string& file_id, const std::string& _upload_user_id)
        : _file_id(file_id)
        , _upload_user_id(_upload_user_id)
        , _size(-1)
    {

    }
    suiFileMeta::suiFileMeta(const std::string& file_id
        , const std::string& _upload_user_id, const std::string& path
        , size_t size, const std::string& mime_type)
        : _file_id(file_id)
        , _upload_user_id(_upload_user_id)
        , _path(path)
        , _size(size)
        , _mime_type(mime_type)
    {
        
    }
    suiFileMeta::suiFileMeta(const std::string& file_id)
        : _file_id(file_id)
        , _size(-1)
    {
        
    }
    suiFileMeta::suiFileMeta()
        : _size(-1)
    {

    }
    
    suiFileMeta::~suiFileMeta()
    {
        
    }

    const std::string& suiFileMeta::getFileId() const
    {
        return _file_id;
    }
    const std::string& suiFileMeta::getUploadUserId() const
    {
        return _upload_user_id;
    }
    odb::nullable<std::string> suiFileMeta::getPath() const
    {
        return _path;
    }
    size_t suiFileMeta::getSize() const
    {
        return _size;
    }
    odb::nullable<std::string> suiFileMeta::getMimeType() const
    {
        return _mime_type;
    }

    fileStatus suiFileMeta::getFileStatus() const
    {
        return static_cast<fileStatus>(_file_status);
    }

    void suiFileMeta::setFileStatus(fileStatus status)
    {
        _file_status = static_cast<int>(status);
        if(_file_status > 3)
        {
            _file_status = 0;
        }
    }

    std::string suiFileMeta::getUploadTimeString() const
    {
        if (_upload_time == 0) return "0";

        time_t time_val = static_cast<time_t>(_upload_time);
        struct tm *tm_info = localtime(&time_val);
        if (!tm_info)
        {
            ERROR("时间解析失败");
            return "";
        }

        //开始构造
        std::stringstream ss;

        ss << tm_info->tm_year + 1900 << "-" << tm_info->tm_mon + 1 << "-" << tm_info->tm_mday << ' ';
        ss << tm_info->tm_hour << ':' << tm_info->tm_min << ':' << tm_info->tm_sec;
        return ss.str();
    }
    std::uint64_t suiFileMeta::getUploadTime() const
    {
        return _upload_time;
    }
    
    void suiFileMeta::setFileId(const std::string& file_id)
    {
        _file_id = file_id;
    }
    void suiFileMeta::setPath(const std::string& path)
    {
        _path = path;
    }
    void suiFileMeta::setSize(size_t size)
    {
        _size = size;
    }
    void suiFileMeta::setMimeType(const std::string& mime_type)
    {
        _mime_type = mime_type;
    }

    void suiFileMeta::setUploadUserId(const std::string& upload_user_id)
    {
        _upload_user_id = upload_user_id;
    }

    void suiFileMeta::setUploadTime()
    {
        _upload_time = static_cast<std::uint64_t>(std::time(nullptr));
    }
    /*file end ===============================================================================================*/

    namespace suiIdentityType
    {
        //身份字符串切换    
        std::string identityTypeToString(suiDataSql::identityType identity_type)
        {
            switch (identity_type) 
            {
                case suiDataSql::identityType::identityTypeNormal:
                    return "identityTypeNormal";
                case suiDataSql::identityType::identityTypeAdmin:
                    return "identityTypeAdmin";
                default:
                    return "identityTypeUnknown"; //未知身份
            }
        }
        suiDataSql::identityType stringToIdentityType(const std::string& identityTypeStr)
        {
            if (identityTypeStr == "identityTypeNormal") {
                return suiDataSql::identityType::identityTypeNormal;
            }
            else if (identityTypeStr == "identityTypeAdmin") {
                return suiDataSql::identityType::identityTypeAdmin;
            }
            else {
                return suiDataSql::identityType::identityTypeUnknown; //未知身份
            }
        }
    }
    
    /*用户身份映射表 ===============================================================================================*/
    suiIdentityMeta::suiIdentityMeta()
    {

    }

    void suiIdentityMeta::setIdentityId(const std::string& curid)
    {
        _identity_id = curid;
    }

    void suiIdentityMeta::setIdentityType(identityType identity_type)
    {
        _identityType = identity_type;
    }

    void suiIdentityMeta::setIdentityDescription(const std::string& identity_description)
    {
        _identity_description = identity_description;
    }

    const std::string& suiIdentityMeta::getIdentityId()
    {
        return _identity_id;
    }

    identityType suiIdentityMeta::getIdentityType()
    {
        return _identityType;
    }

    odb::nullable<std::string> suiIdentityMeta::getIdentityDescription()
    {
        return _identity_description;
    }
    /*用户身份映射表 end ===============================================================================================*/

    namespace suiRoleType
    {
        //角色字符串切换
        std::string roleTypeToString(suiDataSql::roleType role_type)
        {
            switch (role_type) 
            {
                case suiDataSql::roleType::roleTypeNormal: //普通角色
                    return "roleTypeNormal";
                case suiDataSql::roleType::roleTypeAdmin: //管理员角色
                    return "roleTypeAdmin";
                case suiDataSql::roleType::roleTypeSuperAdmin: //超级管理员角色
                    return "roleTypeSuperAdmin";
                default:
                    return "roleTypeUnknown"; //未知角色
            }
        }
        suiDataSql::roleType stringToRoleType(const std::string& roleTypeStr)
        {
            if (roleTypeStr == "roleTypeNormal") {
                return suiDataSql::roleType::roleTypeNormal;
            }
            else if (roleTypeStr == "roleTypeAdmin") {
                return suiDataSql::roleType::roleTypeAdmin;
            }
            else if (roleTypeStr == "roleTypeSuperAdmin") {
                return suiDataSql::roleType::roleTypeSuperAdmin;
            }
            else {
                return suiDataSql::roleType::roleTypeUnknown; //未知角色
            }
        }

        int getWeight(suiDataSql::roleType roleId)
        {
            switch (roleId) 
            {
                case suiDataSql::roleType::roleTypeSuperAdmin: return 100;
                case suiDataSql::roleType::roleTypeAdmin:      return 50;
                case suiDataSql::roleType::roleTypeNormal:     return 10;
                default:                                       return 0; // Unknown 或 Guest
            }
        }
    }
    
        /*角色信息表 ===============================================================================================*/
    suiRoleMeta::suiRoleMeta()
    {

    }

    void suiRoleMeta::setRoleId(const std::string& curid)
    {
        _role_id = curid;
    }

    void suiRoleMeta::setRoleType(roleType role_type)
    {
        _role_type = role_type;
    }

    void suiRoleMeta::setRoleDescription(const std::string& role_description)
    {
        _role_description = role_description;
    }

    const std::string& suiRoleMeta::getRoleId()
    {
        return _role_id;
    }

    roleType suiRoleMeta::getRoleType()
    {
        return _role_type;
    }

    odb::nullable<std::string> suiRoleMeta::getRoleDescription()
    {
        return _role_description;
    }
    /*角色信息表 end ===============================================================================================*/

        /*操作元信息表 ===============================================================================================*/
    suiOperationMeta::suiOperationMeta()
    {

    }

    void suiOperationMeta::setOperationId(const std::string& curid)
    {
        _operation_id = curid;
    }

    void suiOperationMeta::setOperationUrl(const std::string& operation_url)
    {
        _operation_url = operation_url;
    }

    void suiOperationMeta::setOperationDescription(const std::string& operation_description)
    {
        _operation_description = operation_description;
    }

    const std::string& suiOperationMeta::getOperationId()
    {
        return _operation_id;
    }

    const std::string& suiOperationMeta::getOperationUrl()
    {
        return _operation_url;
    }

    odb::nullable<std::string> suiOperationMeta::getOperationDescription()
    {
        return _operation_description;
    }
    /*操作元信息表 end ===============================================================================================*/

        /*用户表 ===============================================================================================*/
    suiUsrMeta::suiUsrMeta()
    {

    }

    void suiUsrMeta::setUserId(const std::string& user_id)
    {
        _user_id = user_id;
    }

    void suiUsrMeta::setBindEmail(const std::string& bind_email)
    {
        _bind_email = bind_email;
    }

    void suiUsrMeta::setUserName(const std::string& user_name)
    {
        _user_name = user_name;
    }

    void suiUsrMeta::setAdministratorName(const std::string& administrator_name)
    {
        _administrator_name = administrator_name;
    }

    void suiUsrMeta::setPassword(const std::string& password)
    {
        _password = password;
    }

    void suiUsrMeta::setHeadImageFileId(const std::string& head_image_file_id)
    {
        _head_image_file_id = head_image_file_id;
    }

    void suiUsrMeta::setUserDescription(const std::string& user_description)
    {
        _user_description = user_description;
    }

    void suiUsrMeta::setUserStatus(userStatus user_status)
    {
        _user_status = user_status;
    }

    void suiUsrMeta::setUploadTime()
    {
        _upload_time = static_cast<std::uint64_t>(std::time(nullptr));
    }

    const std::string& suiUsrMeta::getUserId()
    {
        return _user_id;
    }

    const std::string& suiUsrMeta::getBindEmail()
    {
        return _bind_email;
    }

    const std::string& suiUsrMeta::getUserName()
    {
        return _user_name;
    }

    odb::nullable<std::string> suiUsrMeta::getAdministratorName()
    {
        return _administrator_name;
    }

    odb::nullable<std::string> suiUsrMeta::getPassword()
    {
        return _password;
    }

    odb::nullable<std::string> suiUsrMeta::getHeadImageFileId()
    {
        return _head_image_file_id;
    }

    odb::nullable<std::string> suiUsrMeta::getUserDescription()
    {
        return _user_description;
    }

    userStatus suiUsrMeta::getUserStatus()
    {
        return _user_status;
    }

    std::uint64_t suiUsrMeta::getUploadTime()
    {
        return _upload_time;
    }
    /*用户表 end ===============================================================================================*/

        /*用户身份角色关系表 ===============================================================================================*/
    suiUserIdIdentityRoleMeta::suiUserIdIdentityRoleMeta()
    {

    }

    void suiUserIdIdentityRoleMeta::setUserId(const std::string& user_id)
    {
        _user_id = user_id;
    }

    void suiUserIdIdentityRoleMeta::setRoleType(roleType role_type)
    {
        _role_type = role_type;
    }

    void suiUserIdIdentityRoleMeta::setIdentityType(identityType identity_type)
    {
        _identity_type = identity_type;
    }

    const std::string& suiUserIdIdentityRoleMeta::getUserId()
    {
        return _user_id;
    }

    roleType suiUserIdIdentityRoleMeta::getRoleType()
    {
        return _role_type;
    }

    identityType suiUserIdIdentityRoleMeta::getIdentityType()
    {
        return _identity_type;
    }
    /*用户身份角色关系表 end ===============================================================================================*/

    /*权限角色操作关系表 ===============================================================================================*/
    suiRoleOperationMeta::suiRoleOperationMeta()
    {

    }

    void suiRoleOperationMeta::setRoleType(roleType role)
    {
        _role_type = role;
    }

    void suiRoleOperationMeta::setOperationUrl(const std::string& id)
    {
        _operation_url = id;
    }

    roleType suiRoleOperationMeta::getRoleType()
    {
        return _role_type;
    }

    const std::string& suiRoleOperationMeta::getOperationUrl()
    {
        return _operation_url;
    }
    /*权限角色操作关系表 end ===============================================================================================*/

    /*用户关注关系表 ===============================================================================================*/
    suiUserFollowMeta::suiUserFollowMeta()
    {

    }

    void suiUserFollowMeta::setUserId(const std::string& user_id)
    {
        _user_id = user_id;
    }

    void suiUserFollowMeta::setFollowUserId(const std::string& follow_user_id)
    {
        _follow_user_id = follow_user_id;
    }

    const std::string& suiUserFollowMeta::getUserId()
    {
        return _user_id;
    }

    const std::string& suiUserFollowMeta::getFollowUserId()
    {
        return _follow_user_id;
    }
    /*用户关注关系表 end ===============================================================================================*/

    /*视频元信息表 ===============================================================================================*/
    suiVideoMeta::suiVideoMeta()
    {

    }

    void suiVideoMeta::setVideoId(const std::string& video_id)
    {
        _video_id = video_id;
    }

    void suiVideoMeta::setVideoFileId(const std::string& video_file_id)
    {
        _video_file_id = video_file_id;
    }

    void suiVideoMeta::setVideoCoverFileId(const std::string& video_cover_file_id)
    {
        _video_cover_file_id = video_cover_file_id;
    }

    void suiVideoMeta::setUploadUserId(const std::string& upload_user_id)
    {
        _upload_user_id = upload_user_id;
    }

    void suiVideoMeta::setReviewUserId(const std::string& review_user_id)
    {
        _review_user_id = review_user_id;
    }

    void suiVideoMeta::setVideoName(const std::string& video_name)
    {
        _video_name = video_name;
    }

    void suiVideoMeta::setVideoDescription(const std::string& video_description)
    {
        _video_description = video_description;
    }

    void suiVideoMeta::setVideoPlayCount(unsigned int video_play_count)
    {
        _video_play_count = video_play_count;
    }

    void suiVideoMeta::setVideoSize(uint64_t video_size)
    {
        _video_size = video_size;
    }

    void suiVideoMeta::setVideoDuration(uint64_t video_duration)
    {
        _video_duration = video_duration;
    }

    void suiVideoMeta::setVideoUploadTime(uint64_t video_upload_time)
    {
        _video_upload_time = video_upload_time;
    }

    void suiVideoMeta::setVideoStatus(videoStatus video_status)
    {
        _video_status = video_status;
    }

    const std::string& suiVideoMeta::getVideoId()
    {
        return _video_id;
    }

    const std::string& suiVideoMeta::getVideoFileId()
    {
        return _video_file_id;
    }

    const std::string& suiVideoMeta::getVideoCoverFileId()
    {
        return _video_cover_file_id;
    }

    const std::string& suiVideoMeta::getUploadUserId()
    {
        return _upload_user_id;
    }

    const std::string& suiVideoMeta::getReviewUserId()
    {
        return _review_user_id;
    }

    const std::string& suiVideoMeta::getVideoName()
    {
        return _video_name;
    }

    const odb::nullable<std::string>& suiVideoMeta::getVideoDescription()
    {
        return _video_description;
    }

    unsigned int suiVideoMeta::getVideoPlayCount()
    {
        return _video_play_count;
    }

    uint64_t suiVideoMeta::getVideoSize()
    {
        return _video_size;
    }

    uint64_t suiVideoMeta::getVideoDuration()
    {
        return _video_duration;
    }

    uint64_t suiVideoMeta::getVideoUploadTime()
    {
        return _video_upload_time;
    }

    videoStatus suiVideoMeta::getVideoStatus()
    {
        return _video_status;
    }
    /*视频元信息表 end ===============================================================================================*/

    /*用户点赞关系表 ===============================================================================================*/
    suiUserLikeMeta::suiUserLikeMeta()
    {

    }

    void suiUserLikeMeta::setUserId(const std::string& user_id)
    {
        _user_id = user_id;
    }

    void suiUserLikeMeta::setVideoId(const std::string& video_id)
    {
        _video_id = video_id;
    }

    const std::string& suiUserLikeMeta::getUserId()
    {
        return _user_id;
    }

    const std::string& suiUserLikeMeta::getVideoId()
    {
        return _video_id;
    }
    /*用户点赞关系表 end ===============================================================================================*/





}
