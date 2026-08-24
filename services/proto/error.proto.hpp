#pragma once
#include <cstdint>

// ============================================================================
// 统一错误码定义中心 (Error Code Definition)
// 
// 【错误码定义规范】（5位数字分段法）
// 1. 唯一成功：0 永远代表成功。
// 2. 只加不改：历史遗留的错误码严禁修改或删除，废弃的直接保留，新错误往后延。
// 3. 区间分配：前 2 位代表【业务模块/系统】，后 3 位代表【具体错误】。
// 4. 前端解耦：客户端逻辑判断强依赖错误码数字，errorMsg 仅用于展示或日志。
// ============================================================================


// ---------------------------------------------------------
// [0] 成功码
// ---------------------------------------------------------


// ---------------------------------------------------------
// [10000 - 19999] 全局与基础架构级错误 (Global & Infrastructure)
// 说明：与具体业务无关的底层系统错误，如网络、数据库、通用参数校验等。
// ---------------------------------------------------------
// constexpr int32_t ERR_SYSTEM_BUSY        = 10001; // 系统繁忙
// constexpr int32_t ERR_INVALID_PARAM      = 10002; // 通用参数错误/缺失
// constexpr int32_t ERR_DB_CONNECTION      = 10003; // 数据库连接失败
// constexpr int32_t ERR_NETWORK_TIMEOUT    = 10004; // 网络请求超时


// ---------------------------------------------------------
// [20000 - 29999] 用户与鉴权模块 (User & Auth Module)
// 说明：涉及账号、密码、Token、权限等通用身份相关的错误。
// ---------------------------------------------------------
// constexpr int32_t ERR_AUTH_UNAUTHORIZED  = 20001; // 未登录或 Token 缺失
// constexpr int32_t ERR_AUTH_TOKEN_EXPIRED = 20002; // Token 已过期
// constexpr int32_t ERR_USER_NOT_FOUND     = 20003; // 用户不存在
// constexpr int32_t ERR_PASSWORD_INCORRECT = 20004; // 密码错误


// ---------------------------------------------------------
// [30000 - 39999] 文件与图片上传模块 (File & Image Module)
// 说明：处理多媒体资源上传、解析、存储的核心业务错误。
// ---------------------------------------------------------
// constexpr int32_t ERR_IMAGE_TOO_LARGE    = 30001; // 图片体积超出系统限制
// constexpr int32_t ERR_IMAGE_FORMAT_BAD   = 30002; // 不支持的图片格式
// constexpr int32_t ERR_STORAGE_UPLOAD     = 30003; // 上传至云存储后端失败
// constexpr int32_t ERR_IMAGE_CORRUPTED    = 30004; // 图片数据损坏或无法解析


// ---------------------------------------------------------
// [40000 - 49999] 其他核心业务模块 (如订单、商品等)
// 说明：预留给未来其他独立业务线的错误码区间。
// ---------------------------------------------------------
// constexpr int32_t ERR_BIZ_XXX            = 40001;


// ---------------------------------------------------------
// [99000 - 99999] 致命与未知错误 (Fatal & Unknown)
// 说明：未被捕获的异常或极其严重的系统崩溃。
// ---------------------------------------------------------
// constexpr int32_t ERR_INTERNAL_SERVER    = 99999; // 服务器内部未知错误

namespace suiErrorCodeDef
{
    //成功
    constexpr int32_t SUCCESS = 0;

    //==========================================================================
    //[30000 - 39999] 文件服务错误
    constexpr int32_t ERR_FILE_SERVICE_SESSION_INVALID = 31000; //无效会话
    constexpr int32_t ERR_FILE_SERVICE_UPLOAD_USER_NO_PERMISSION = 31001; //无上传权限
    constexpr int32_t ERR_FILE_SERVICE_UPLOAD_FAILED = 31002; //上传失败
    constexpr int32_t ERR_FILE_SERVICE_UPLOAD_USER_MISMATCH = 31003; //用户id与会话用户不匹配
    constexpr int32_t ERR_FILE_SERVICE_FILE_NOT_FOUND = 31004; //文件不存在
    constexpr int32_t ERR_FILE_SERVICE_DOWNLOAD_FAILED = 31005; //下载失败
    constexpr int32_t ERR_FILE_SERVICE_UPLOAD_DATA_INVALID = 31006; //上传数据失效
    constexpr int32_t ERR_FILE_SERVICE_UPLOAD_INVALID = 31007; //上传操作失效
    constexpr int32_t ERR_FILE_SERVICE_UPLOAD_CHUNK_INDEX_INVALID = 31008; //上传分片的序号不对
    constexpr int32_t ERR_FILE_SERVICE_UPLOAD_ID_INVALID = 31009; //上传id失效
    constexpr int32_t ERR_FILE_SERVICE_FILE_STATUS_ERROR = 31010; //文件状态错误
    constexpr int32_t ERR_FILE_SERVICE_FILE_SIZE_ERROR = 31011; //文件大小错误
    
    
    
    //==========================================================================

    //==========================================================================
    //[40000 - 49999] 用户服务错误
    constexpr int32_t ERR_USER_SERVICE_SESSION_INVALID = 41000; //会话失效
    constexpr int32_t ERR_USER_SERVICE_SESSION_GUEST = 41001; //临时会话验证失败
    constexpr int32_t ERR_USER_SERVICE_VERIFY_CODE_MISMATCH = 41002; //验证码申请失败
    constexpr int32_t ERR_USER_SERVICE_VERIFY_CODE_GET_FAILED = 41003; //验证码获取失败
    constexpr int32_t ERR_USER_SERVICE_EMAIL_INVALID = 41004; //邮箱格式错误
    constexpr int32_t ERR_USER_SERVICE_SESSION_ID_INVALID = 41005; //会话格式错误
    constexpr int32_t ERR_USER_SERVICE_EMAIL_LOGIN_FAILED = 41006; //邮箱登录失败
    constexpr int32_t ERR_USER_SERVICE_SET_AVATAR_FAILED = 41007; //用户身份验证失败
    constexpr int32_t ERR_USER_SERVICE_GET_USER_INFO_FAILED = 41008; //获取用户信息失败
    constexpr int32_t ERR_USER_SERVICE_USER_NOT_FOUND = 41010; //用户不存在
    constexpr int32_t ERR_USER_SERVICE_USER_PERMISSION_INVALID = 41011; //用户权限不足
    constexpr int32_t ERR_USER_SERVICE_SESSION_STATUS_INVALID = 41012; //用户会话状态异常
    constexpr int32_t ERR_USER_SERVICE_VERIFY_CODE_INVALID = 41013; //验证码错误
    constexpr int32_t ERR_USER_SERVICE_USER_ADMIN_FAILED = 41014; //新增管理员失败
    constexpr int32_t ERR_USER_SERVICE_USER_ADMIN_REMOVE_FAILED = 41015; //删除管理员失败
    constexpr int32_t ERR_USER_SERVICE_USER_ADMIN_SET_FAILED = 41016; //修改管理员失败
    constexpr int32_t ERR_USER_SERVICE_USER_ADMIN_PERMISSION_INVALID = 41017; //用户权限异常
    constexpr int32_t ERR_USER_SERVICE_USER_ADMIN_INVALID = 41018; //申请参数错误
    constexpr int32_t ERR_USER_SERVICE_USER_SALT_SET = 41019; //用户信息设置失败
    constexpr int32_t ERR_USER_SERVICE_EMAIL_DISABLE = 41020; //用户已被禁用
    constexpr int32_t ERR_USER_SERVICE_PASSWORD_INVALID = 41021; //密码错误
    constexpr int32_t ERR_USER_IDENTITY_ROLE_INVALID = 41022; //用户属性异常
    //==========================================================================


    //==========================================================================
    //[50000 - 59999] 视频服务错误
    constexpr int32_t ERR_VIDEO_SERVICE_VIDEO_NOT_FOUND = 51000; //视频不存在
    constexpr int32_t ERR_VIDEO_SERVICE_UPLOAD_USER_MISMATCH = 51001; //视频上传用户与会话用户不匹配
    constexpr int32_t ERR_VIDEO_SERVICE_VIDEO_STATUS_INVALID = 51002; //视频状态异常

    //==========================================================================
    //服务器错误
    constexpr int32_t ERR_SERVER = 99998; // 服务器内部未知错误
    //未知错误
    constexpr int32_t ERR_UNKNOWN = 99999; // 未知错误
}