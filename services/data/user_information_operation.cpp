#include "user_information_operation.hpp"

namespace suiUserInformation
{
    const int suiUserInformationOperation::cacheExpire_min = 60*60*1; //1小时过期
    const int suiUserInformationOperation::cacheExpire_max = 60*60*2; //2小时过期
    const std::string suiUserInformationOperation::_cacheKeyPrefix = "user_info";
    const std::string suiUserInformationOperation::_userIdKey = "user_id";
    const std::string suiUserInformationOperation::_userBindEmailKey = "user_bind_email";
    const std::string suiUserInformationOperation::_userNameKey = "user_name";
    const std::string suiUserInformationOperation::_userAdministratorNameKey = "user_administrator_name";
    const std::string suiUserInformationOperation::_userPasswordKey = "user_password";
    const std::string suiUserInformationOperation::_userHeadImageFileIdKey = "user_head_image_file_id";
    const std::string suiUserInformationOperation::_userDescriptionKey = "user_description";
    const std::string suiUserInformationOperation::_userStatusKey = "user_status";
    const std::string suiUserInformationOperation::_userUploadTimeKey = "user_upload_time";
    
    suiUserInformationOperation::suiUserInformationOperation(odb::transaction& sql_tx,  sw::redis::Transaction& cache_tx, suiRemoveCache::RemoveCache::ptr removeCachePtr)
        :_sql_tx(sql_tx), _cache_tx(cache_tx), _removeCachePtr(removeCachePtr)
    {

    }
    
    void suiUserInformationOperation::insert(const std::string& uid, const std::string& email)
    {
        //判断用户是否存在由业务层处理
        //本地直接向数据库插入
        insertToDb(uid, email);
    }
    
    void suiUserInformationOperation::updateUser(suiDataSql::userInfoList::ptr newInfoPtr)
    {
        //直接向数据库更新，删除原有的缓存
        updateUserToDb(newInfoPtr);
        for(auto& it : newInfoPtr->list)
            deleteUserToCache(it.getUserId());
    }

    void suiUserInformationOperation::updateUser(suiDataSql::suiUsrMeta::ptr newInfoPtr)
    {
        suiDataSql::userInfoList::ptr cur = std::make_shared<suiDataSql::userInfoList>();
        cur->list.push_back(*newInfoPtr);
        updateUserToDb(cur);
        deleteUserToCache(newInfoPtr->getUserId());
    }

    bool suiUserInformationOperation::isExistByEmail(const std::string& emailm, suiDataSql::identityType type)
    {
        //判断相同邮箱身份的用户是否存在
        auto ret = getUserInfoByEmailToDb(emailm, type);
        if(ret == nullptr)
            return false;
        return true;
    }

    bool suiUserInformationOperation::isExistByEmail(const std::string& emailm)
    {
        auto ret = getUserInfoByEmailToDb(emailm);
        if(ret == nullptr)
            return false;
        return true;
    }

    bool suiUserInformationOperation::authenticationByPassword(const std::string& user_id, const std::string& password)
    {
        //先从缓存获取
        auto ret = getUserInfoToCache(user_id);
        if(ret)
        {
            //存在，直接判断
            if(ret == nullptr || ret->getPassword().null() || ret->getPassword().get() != password)
                return false;//不存在用户 或者 用户密码为空 再或者 密码错误
            return true;
        }
        //从数据库验证密码密码验证
        ret = getUserByIdToDb(user_id);
        if(ret == nullptr)
            return false;
        //此时数据存在，添加进缓存中
        insertToCache(ret); 
        if(ret->getPassword().null() || ret->getPassword().get() != password)
            return false;//不存在用户 或者 用户密码为空 再或者 密码错误
        return true;
    }

    suiDataSql::suiUsrMeta::ptr suiUserInformationOperation::getUserInfoById(const std::string& user_id)
    {
        //直接从数据库查询
        auto ret = getUserByIdToDb(user_id);
        if(ret)
        {
            //插入缓存
            insertToCache(ret);
        }
        return ret;
    }

    suiDataSql::userInfoList::ptr suiUserInformationOperation::getUserInfoListByTypeAndStatus(suiDataSql::userStatus status, suiDataSql::identityType type, int page, int pageSize)
    {
        return getUserInfoListByTypeAndStatusToDb(status, type, page, pageSize);
    }

    suiDataSql::userInfoList::ptr suiUserInformationOperation::getUserInfoListByTypeAndStatus(suiDataSql::roleType role, suiDataSql::identityType type, int page, int pageSize)
    {
        return getUserInfoListByTypeAndStatusToDb(role, type, page, pageSize);  
    }

    std::string suiUserInformationOperation::setAvatar(const std::string& user_id, const std::string& avatar_id)
    {
        //修改头像前，先把原缓存清理掉
        deleteUserToCache(user_id);
        return updateUserAvatarToDb(user_id, avatar_id);
    }

    void suiUserInformationOperation::setUserStatus(const std::string& user_id, suiDataSql::userStatus user_status)
    {
        //重新设置用户状态
        deleteUserToCache(user_id);
        updateUserStatusToDb(user_id, user_status); //直接设置，无需返回值处理
    }

    void suiUserInformationOperation::setUserName(const std::string& user_id, const std::string& user_name)
    {
        deleteUserToCache(user_id);
        updateUserNameToDb(user_id, user_name);
    }

    void suiUserInformationOperation::setUserPassward(const std::string& user_id, const std::string& password)
    {
        deleteUserToCache(user_id);//删除缓存
        updateUserPasswordToDb(user_id, password);
    }

    bool suiUserInformationOperation::isEnabled(const std::string& user_id)
    {
        //判断用户是否被禁用
        //优先从缓存中获取
        auto ret = getUserInfoToCache(user_id);
        if(ret)
        {
            //存在，直接判断是否为禁用状态
            if(ret->getUserStatus() == suiDataSql::userStatus::userStatusEnable)
            {
                //使能状态
                return true;
            }
            return false;
        }
        //从数据库验证密码密码验证
        ret = getUserByIdToDb(user_id);
        if(ret == nullptr)
            return false;
        //此时数据存在，添加进缓存中
        insertToCache(ret);
        if(ret->getUserStatus() == suiDataSql::userStatus::userStatusEnable)
        {
            //使能状态
            return true;
        }
        return false;
    }

    void suiUserInformationOperation::setAdminInfo(const std::string& user_id, const std::string& admin_name, const std::string& Remark, suiDataSql::userStatus status)
    {
        //删除缓存
        deleteUserToCache(user_id);
        //设置管理员信息
        setAdminInfoToDb(user_id, admin_name, Remark, status);
    }

    void suiUserInformationOperation::deleteUser(const std::string& user_id)
    {
        //删除缓存
        deleteUserToCache(user_id);
        //删除数据库中的用户信息
        deleteUserToDb(user_id);
    }

    void suiUserInformationOperation::insertToDb(const std::string& uid, const std::string& email)
    {
        //直接添加
        //获取事务的操作对象
        auto& handle = _sql_tx.database();
        //进行添加
        suiDataSql::suiUsrMeta newInfoPtr(uid, email);
        handle.persist(newInfoPtr);

        //不需要内部提交事务
    }

    suiDataSql::suiUsrMeta::ptr suiUserInformationOperation::getUserByIdToDb(const std::string& user_id)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //从数据库中获取用户标准信息
        auto ret = suiDataSql::suiUsrMeta::ptr(handle.query_one<suiDataSql::suiUsrMeta>(
            odb::query<suiDataSql::suiUsrMeta>::user_id == user_id
        ));
        return ret;
    }

    suiDataSql::suiUsrMeta::ptr suiUserInformationOperation::getUserInfoByEmailToDb(const std::string& email)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //从数据库中获取用户标准信息
        auto ret = suiDataSql::suiUsrMeta::ptr(handle.query_one<suiDataSql::suiUsrMeta>(
            odb::query<suiDataSql::suiUsrMeta>::bind_email == email
        ));
        return ret;
    }

    suiDataSql::suiUsrMeta::ptr suiUserInformationOperation::getUserInfoByEmailToDb(const std::string& email, suiDataSql::identityType type)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //从数据库中获取用户标准信息
        auto ret = suiDataSql::suiUserIdIdentityRoleView::ptr(handle.query_one<suiDataSql::suiUserIdIdentityRoleView>(
            odb::query<suiDataSql::suiUserIdIdentityRoleView>::suiUsrMeta::bind_email == email 
            && odb::query<suiDataSql::suiUserIdIdentityRoleView>::suiUserIdIdentityRoleMeta::identity_type == type
        ));
        return ret->usrMeta;
    }
    

    suiDataSql::userInfoList::ptr suiUserInformationOperation::getUserInfoListByNameToDb(const std::string& user_name)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //创建返回对象
        suiDataSql::userInfoList::ptr out = std::make_shared<suiDataSql::userInfoList>();
        //以视图方式向数据库中获取信息
       auto ret = handle.query<suiDataSql::suiUsrMeta>(
            odb::query<suiDataSql::suiUsrMeta>::user_name == user_name
        );
        //循环获取
        for(auto& item : ret)
            out->list.push_back(item);
        return out;
    }

    suiDataSql::userInfoList::ptr suiUserInformationOperation::getUserInfoByAdministratorNameToDb(const std::string& user_name)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //创建返回对象
        suiDataSql::userInfoList::ptr out = std::make_shared<suiDataSql::userInfoList>(); //管理员名称可能相同
        //通过管理员名称获取用户信息
        auto ret = handle.query<suiDataSql::suiUsrMeta>(
            odb::query<suiDataSql::suiUsrMeta>::administrator_name == user_name
        );
        //循环获取
        for(auto& item : ret)
            out->list.push_back(item);
        return out;
    }

    suiDataSql::userInfoList::ptr suiUserInformationOperation::getUserInfoListByTypeAndStatusToDb(suiDataSql::userStatus status, suiDataSql::identityType type
                                                                                                , suiDataSql::roleType role, int page, int pageSize)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //创建返回对象
        suiDataSql::userInfoList::ptr out = std::make_shared<suiDataSql::userInfoList>();

        //设计query筛选
        //select * from tbl_user_like_meta order by primaryKey DESC排序，带有desc的是倒序
        //添加 LIMIT 10 OFFSET 10
        //定义规则
        auto q = odb::query<suiDataSql::suiUserIdIdentityRoleView>::suiUserIdIdentityRoleMeta::identity_type == type
        && odb::query<suiDataSql::suiUserIdIdentityRoleView>::suiUserIdIdentityRoleMeta::role_type == role
        && odb::query<suiDataSql::suiUserIdIdentityRoleView>::suiUsrMeta::user_status == status;
        q += suiDataSql::SqlPaginationUtil::buildPageClause(pageSize, page) + suiDataSql::SqlPaginationUtil::buildOrderByClause("primaryKey");  //定义输出顺序以及内容

        //开始查询
        auto ret = handle.query<suiDataSql::suiUserIdIdentityRoleView>(q);
        //循环获取
        for(auto& itemPtr : ret)
            out->list.push_back(*itemPtr.usrMeta);
        return out;
    }

    suiDataSql::userInfoList::ptr suiUserInformationOperation::getUserInfoListByTypeAndStatusToDb(suiDataSql::userStatus status, suiDataSql::identityType type, int page, int pageSize)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //创建返回对象
        suiDataSql::userInfoList::ptr out = std::make_shared<suiDataSql::userInfoList>();

        auto q = odb::query<suiDataSql::suiUserIdIdentityRoleView>::suiUserIdIdentityRoleMeta::identity_type == type
        && odb::query<suiDataSql::suiUserIdIdentityRoleView>::suiUsrMeta::user_status == status;
        q += suiDataSql::SqlPaginationUtil::buildPageClause(pageSize, page) + suiDataSql::SqlPaginationUtil::buildOrderByClause("primaryKey");  //定义输出顺序以及内容

        //开始查询
        auto ret = handle.query<suiDataSql::suiUserIdIdentityRoleView>(q);
        //循环获取
        for(auto& itemPtr : ret)
            out->list.push_back(*itemPtr.usrMeta);
        return out;
    }
    suiDataSql::userInfoList::ptr suiUserInformationOperation::getUserInfoListByTypeAndStatusToDb(suiDataSql::roleType role, suiDataSql::identityType type, int page, int pageSize)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //创建返回对象
        suiDataSql::userInfoList::ptr out = std::make_shared<suiDataSql::userInfoList>();

        auto q = odb::query<suiDataSql::suiUserIdIdentityRoleView>::suiUserIdIdentityRoleMeta::identity_type == type
        && odb::query<suiDataSql::suiUserIdIdentityRoleView>::suiUserIdIdentityRoleMeta::role_type == role;
        q += " " + suiDataSql::SqlPaginationUtil::buildOrderByClause("primaryKey") + " " + suiDataSql::SqlPaginationUtil::buildPageClause(pageSize, page);  //定义输出顺序以及内容
        std::stringstream ss;
        ss << q.clause();
        INFO("查询语句: {}", ss.str());
        //开始查询
        auto ret = handle.query<suiDataSql::suiUserIdIdentityRoleView>(q);
        //循环获取
        for(auto& itemPtr : ret)
            out->list.push_back(*itemPtr.usrMeta);
        return out;
    }

    void suiUserInformationOperation::updateUserToDb(suiDataSql::userInfoList::ptr newInfoPtr)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        
        for(auto& item : newInfoPtr->list)
        {
            _updateUserToDb(item, handle);
        }
    }

    void suiUserInformationOperation::updateUserToDb(suiDataSql::suiUsrMeta newInfo)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        _updateUserToDb(newInfo, handle);
    }
    void suiUserInformationOperation::_updateUserToDb(suiDataSql::suiUsrMeta newInfo,  odb::transaction::database_type& handle)
    {
        //优先查询用户是否存在
        auto ret = handle.query_one<suiDataSql::suiUsrMeta>(
            odb::query<suiDataSql::suiUsrMeta>::user_id == newInfo.getUserId()
        );
        if(ret == nullptr)
            return;
        //开始修改值，进行更新 用户id固定不变
        ret->setUserName(newInfo.getUserName());
        ret->setAdministratorName(newInfo.getAdministratorName());
        ret->setHeadImageFileId(newInfo.getHeadImageFileId());
        ret->setUserDescription(newInfo.getUserDescription());
        handle.update(*ret);
    }

    void suiUserInformationOperation::updateUserPasswordToDb(const std::string& user_id, const std::string& password)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //优先查询用户是否存在
        auto ret = handle.query_one<suiDataSql::suiUsrMeta>(
            odb::query<suiDataSql::suiUsrMeta>::user_id == user_id
        );
        if(ret == nullptr)
            return;
        //开始修改值
        ret->setPassword(password);
        handle.update(*ret);
    }
    std::string suiUserInformationOperation::updateUserAvatarToDb(const std::string& user_id, const std::string& avatar_id)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //输出原头像id
        std::string out;
        //优先查询用户是否存在
        auto ret = handle.query_one<suiDataSql::suiUsrMeta>(
            odb::query<suiDataSql::suiUsrMeta>::user_id == user_id
        );
        if(ret == nullptr)
            return out;
        //获取原头像id
        if(!ret->getHeadImageFileId().null())
        {
            //此时头像不为空
            out = ret->getHeadImageFileId().get();
        }
        //开始修改值
        ret->setHeadImageFileId(avatar_id);
        handle.update(*ret);

        //返回原头像id
        return out;
    }

    void suiUserInformationOperation::updateUserStatusToDb(const std::string& user_id, suiDataSql::userStatus user_status)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //优先查询用户是否存在
        auto ret = handle.query_one<suiDataSql::suiUsrMeta>(
            odb::query<suiDataSql::suiUsrMeta>::user_id == user_id
        );
        if(ret == nullptr)
            return;
        //开始修改值
        ret->setUserStatus(user_status);
        handle.update(*ret);
    }

    void suiUserInformationOperation::updateUserNameToDb(const std::string& user_id, const std::string& user_new_name)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //查询用户
        auto ret = handle.query_one<suiDataSql::suiUsrMeta>(
            odb::query<suiDataSql::suiUsrMeta>::user_id == user_id
        );
        if(ret == nullptr)
            return; //修改不存在用户不进行处理，属于客户错误
            
        //修改用户名
        ret->setUserName(user_new_name);
        handle.update(*ret);
    }

    void suiUserInformationOperation::setAdminInfoToDb(const std::string& user_id, const std::string& admin_name, const std::string& Remark, suiDataSql::userStatus status)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //查询用户
        auto ret = handle.query_one<suiDataSql::suiUsrMeta>(
            odb::query<suiDataSql::suiUsrMeta>::user_id == user_id
        );
        if(ret == nullptr)
            return;
        //开始修改值
        ret->setAdministratorName(admin_name);
        ret->setUserDescription(Remark);
        ret->setUserStatus(status);

        handle.update(*ret);
    }

    void suiUserInformationOperation::deleteUserToDb(const std::string& user_id)
    {
        //通过事务获取连接操作句柄
        auto& handle = _sql_tx.database();
        //进行删除 如果对方不存在 则也视为删除成功 即不进行不存在错误处理
        handle.erase_query<suiDataSql::suiUsrMeta>(
            odb::query<suiDataSql::suiUsrMeta>::user_id == user_id
        );
    }

    void suiUserInformationOperation::insertToCache(suiDataSql::suiUsrMeta::ptr userInfoPtr)
    {
        //通过redis事务创建连接操作句柄
        auto rehandler = _cache_tx.redis();

        std::string key = getCacheKey(userInfoPtr->getUserId());

        //存储信息
        std::unordered_map<std::string, std::string> curData;
        curData[_userIdKey] = userInfoPtr->getUserId(); //用户id
        curData[_userBindEmailKey] = userInfoPtr->getBindEmail(); //绑定邮箱
        curData[_userNameKey] = userInfoPtr->getUserName(); //用户名
        //管理员名称
        if(!userInfoPtr->getAdministratorName().null())
            curData[_userAdministratorNameKey] = userInfoPtr->getAdministratorName().get(); //B端用户管理员名称
        //密码
        if(!userInfoPtr->getPassword().null())
            curData[_userPasswordKey] = userInfoPtr->getPassword().get(); //密码
        //头像文件id
        if(!userInfoPtr->getHeadImageFileId().null())
            curData[_userHeadImageFileIdKey] = userInfoPtr->getHeadImageFileId().get(); //头像文件id
        //用户备注描述
        if(!userInfoPtr->getUserDescription().null())
            curData[_userDescriptionKey] = userInfoPtr->getUserDescription().get(); //用户备注描述
        //用户状态
        curData[_userStatusKey] = suiDataSql::userStatusUtil::userStatusToString(userInfoPtr->getUserStatus()); //用户状态
        //创建时间
        curData[_userUploadTimeKey] = std::to_string(userInfoPtr->getUploadTime()); //上传时间
        
        //进行添加
        rehandler.hmset(key, curData.begin(), curData.end());
        rehandler.expire(key, std::chrono::seconds(getCacheExpire()));
    }

    suiDataSql::suiUsrMeta::ptr suiUserInformationOperation::getUserInfoToCache(const std::string& user_id)
    {
        //通过缓存获取用户信息
        auto rehandler = _cache_tx.redis();
        std::string key = getCacheKey(user_id);
        std::unordered_map<std::string, std::string> curData;
        rehandler.hgetall(key, std::inserter(curData, curData.begin()));
        if(curData.empty())
            return nullptr;
        //开始解析用户信息
        suiDataSql::suiUsrMeta::ptr userInfoPtr = std::make_shared<suiDataSql::suiUsrMeta>();
        //用户id
        userInfoPtr->setUserId(curData[_userIdKey]);
        //绑定邮箱
        userInfoPtr->setBindEmail(curData[_userBindEmailKey]);
        //用户名
        userInfoPtr->setUserName(curData[_userNameKey]);
        //管理员名称
        if(!curData[_userAdministratorNameKey].empty())
            userInfoPtr->setAdministratorName(curData[_userAdministratorNameKey]);
        //密码
        if(!curData[_userPasswordKey].empty())
            userInfoPtr->setPassword(curData[_userPasswordKey]);
        //头像文件id
        if(!curData[_userHeadImageFileIdKey].empty())
            userInfoPtr->setHeadImageFileId(curData[_userHeadImageFileIdKey]);
        //用户备注描述
        if(!curData[_userDescriptionKey].empty())
            userInfoPtr->setUserDescription(curData[_userDescriptionKey]);
        //用户状态
        userInfoPtr->setUserStatus(suiDataSql::userStatusUtil::userStatusFromString(curData[_userStatusKey]));
        //创建时间
        userInfoPtr->setUploadTime(std::stoull(curData[_userUploadTimeKey]));
        return userInfoPtr;
    }

    void suiUserInformationOperation::deleteUserToCache(const std::string& user_id)
    {
        //直接发布删除消息
        _removeCachePtr->syncCache({getCacheKey(user_id)});
    }

    std::string suiUserInformationOperation::getCacheKey(const std::string& user_id)
    {
        //获取key
        return _cacheKeyPrefix + user_id;
    }

    int suiUserInformationOperation::getCacheExpire()
    {
        thread_local std::mt19937 gen(std::random_device{}());
        // 均匀分布
        std::uniform_int_distribution<int> distrib(cacheExpire_min, cacheExpire_max);
        return distrib(gen);
    }
}