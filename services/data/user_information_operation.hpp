#pragma once
#include <sstream>
#include <iostream>
#include <suiScaffold/suiodb.hpp>
#include <suiScaffold/suiRedis.hpp>
#include <suiScaffold/suiQueue.hpp>
#include <suiScaffold/log.h>
#include "removeCache.hpp"
#include "data.hpp"
#include "data-odb.hxx"

namespace suiUserInformation
{
    //用户基础信息操作类
    //本次封装，缓存可多次操作，数据库不允许多次操作
    class suiUserInformationOperation
    {
    public:
        //本次直接传入事务对象与缓存删除对象
        suiUserInformationOperation(odb::transaction& sql_t,  sw::redis::Transaction& cache_tx, suiRemoveCache::RemoveCache::ptr removeCachePtr);
        //新增用户信息
        void insert(const std::string& uid, const std::string& email);  //传入用户id与用户邮箱，用户名称初始随机，用户状态初始为
        //修改用户信息
        void updateUser(suiDataSql::userInfoList::ptr newInfoPtr);
        //修改单用户信息
        void updateUser(suiDataSql::suiUsrMeta::ptr newInfoPtr);
        //判断邮箱是否注册过用户 //身份隔离
        bool isExistByEmail(const std::string& emailm, suiDataSql::identityType type);
        //非身份隔离
        bool isExistByEmail(const std::string& emailm);
        //用户密码验证
        bool authenticationByPassword(const std::string& user_id, const std::string& password);
        //获取用户的盐
        //通过邮箱验证密码
        bool authenticationByPasswordByEmail(const std::string& email, const std::string& password);
        //获取用户基础信息
        suiDataSql::suiUsrMeta::ptr getUserInfoById(const std::string& user_id);
        suiDataSql::suiUsrMeta::ptr getUserInfoByEmail(const std::string& email);
        //分页查询某个特定身份和状态的用户 参数：状态 身份 第几页 一页大小
        suiDataSql::userInfoList::ptr getUserInfoListByTypeAndStatus(suiDataSql::userStatus status, suiDataSql::identityType type, int page, int pageSize);
        //通过用户名查询
        suiDataSql::userInfoList::ptr getUserInfoListByUsername(const std::string& username);
        //分页查询某个特定身份和状态的用户 参数：角色 身份 第几页 一页大小
        suiDataSql::userInfoList::ptr getUserInfoListByTypeAndStatus(suiDataSql::roleType role, suiDataSql::identityType type, int page, int pageSize);
        //全属性查询
        suiDataSql::userInfoList::ptr getUserInfoListByTypeAndStatus(suiDataSql::roleType role, suiDataSql::userStatus status, suiDataSql::identityType type, int page, int pageSize);
        //修改头像接口， 返回原头像id，交给外界处理（如删除） 参数：用户id，新头像id
        std::string setAvatar(const std::string& user_id, const std::string& avatar_id);
        //修改用户状态
        void setUserStatus(const std::string& user_id, suiDataSql::userStatus user_status);
        //修改用户名称
        void setUserName(const std::string& user_id, const std::string& user_name);
        //修改密码  暂时先使用明文，后续考虑使用加密
        void setUserPassward(const std::string& user_id, const std::string& password);
        //判断用户是否是启用状态
        bool isEnabled(const std::string& user_id);
        //修改管理员信息 修改B端用户的管理名称，备注以及状态
        void setAdminInfo(const std::string& user_id, const std::string& admin_name, const std::string& Remark);
        //账号注销
        void deleteUser(const std::string& user_id);

    private:
        //向数据库新增用户信息
        void insertToDb(const std::string& uid, const std::string& email);
        //使用uid向数据库获取单用户信息 此接口可以获取被封号的用户
        suiDataSql::suiUsrMeta::ptr getUserByIdToDb(const std::string& user_id);
        //通过邮箱从数据库获取信息
        suiDataSql::suiUsrMeta::ptr getUserInfoByEmailToDb(const std::string& email);
        //通过邮箱从数据库获取信息，并赛选身份类型
        suiDataSql::suiUsrMeta::ptr getUserInfoByEmailToDb(const std::string& email, suiDataSql::identityType type);
        //通过名称从数据库获取所有用户信息
        suiDataSql::userInfoList::ptr getUserInfoListByNameToDb(const std::string& user_name);
        //通过管理员名称从数据库中获取
        suiDataSql::userInfoList::ptr getUserInfoByAdministratorNameToDb(const std::string& user_name);
        //通过状态身份角色类型从数据库中获取用户信息
        suiDataSql::userInfoList::ptr getUserInfoListByTypeAndStatusToDb(suiDataSql::userStatus status, suiDataSql::identityType type, suiDataSql::roleType role, int page, int pageSize);
        //通过状态身份类型从数据库中获取用户信息
        suiDataSql::userInfoList::ptr getUserInfoListByTypeAndStatusToDb(suiDataSql::userStatus status, suiDataSql::identityType type, int page, int pageSize);
        //通过状态身份角色类型从数据库中获取用户信息
        suiDataSql::userInfoList::ptr getUserInfoListByTypeAndStatusToDb(suiDataSql::roleType role, suiDataSql::userStatus status, suiDataSql::identityType type, int page, int pageSize);
        //通过角色身份类型从数据库中获取用户信息
        suiDataSql::userInfoList::ptr getUserInfoListByTypeAndStatusToDb(suiDataSql::roleType role, suiDataSql::identityType type, int page, int pageSize);
        //修改数据库中用户信息 //本地无权更新用户状态，密码，头像
        void updateUserToDb(suiDataSql::userInfoList::ptr newInfoPtr);
        void updateUserToDb(suiDataSql::suiUsrMeta newInfo);
        void _updateUserToDb(suiDataSql::suiUsrMeta newInfo, odb::transaction::database_type& handle);
        //更新用户密码
        void updateUserPasswordToDb(const std::string& user_id, const std::string& password);
        //更新用户头像
        std::string updateUserAvatarToDb(const std::string& user_id, const std::string& avatar_id);
        //更新数据库用户状态
        void updateUserStatusToDb(const std::string& user_id, suiDataSql::userStatus user_status);
        //单向数据库更新用户名称
        void updateUserNameToDb(const std::string& user_id, const std::string& user_new_name);
        //更新管理员标准信息接口
        void setAdminInfoToDb(const std::string& user_id, const std::string& admin_name, const std::string& Remark);
        //从数据库中删除用户信息
        void deleteUserToDb(const std::string& user_id);

        //缓存操作
        //向缓存中添加用户信息
        void insertToCache(suiDataSql::suiUsrMeta::ptr userInfoPtr);
        //从缓存获取用户信息
        suiDataSql::suiUsrMeta::ptr getUserInfoToCache(const std::string& user_id);
        //删除缓存中用户信息
        void deleteUserToCache(const std::string& user_id);

        //获取缓存key
        std::string getCacheKey(const std::string& user_id);
        //获取随机缓存过期时间
        int getCacheExpire();


    private:
        //数据库事务对象
        odb::transaction& _sql_tx;
        //缓存事务对象
        sw::redis::Transaction& _cache_tx;
        //缓存延时删除对象
        suiRemoveCache::RemoveCache::ptr _removeCachePtr;


        //缓存存储时间
        static const int cacheExpire_min;
        static const int cacheExpire_max;
        //缓存前缀
        static const std::string _cacheKeyPrefix;

        //缓存存储字段
        static const std::string _userIdKey;
        static const std::string _userBindEmailKey;
        static const std::string _userNameKey;
        static const std::string _userAdministratorNameKey;
        static const std::string _userPasswordKey;
        static const std::string _userHeadImageFileIdKey;
        static const std::string _userDescriptionKey;
        static const std::string _userStatusKey;
        static const std::string _userUploadTimeKey;
    };





}