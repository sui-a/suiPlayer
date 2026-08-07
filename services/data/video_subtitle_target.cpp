#include "video_subtitle_target.hpp"

namespace suiVideoSubtitleTarget
{
    const std::string _cacheKeyPrefix = "video_subtitle_key_";
    const std::string _map_video_id = "video_id_key";
    const std::string _map_user_id = "user_id_key";
    const std::string _map_subtitle_id = "subtitle_id_key";
    const std::string _map_content = "content_key";
    const std::string _map_video_time = "video_time_key";
    const std::string _map_create_time = "create_time_key";
    const int _cache_expire = 60 * 30; //30分钟过期

    const std::string async_delete_cache_by_video_id = "async_delete_cache_by_video_id";
    const std::string async_insert_db_by_subtitle = "async_insert_db_by_subtitle";
    const std::string async_delete_db_by_subtitleid = "async_delete_db_by_subtitleid";
    const std::string async_delete_db_by_video_id = "async_delete_db_by_video_id";

    videoSubtitleTarget::videoSubtitleTarget(odb::transaction& sql_t,  sw::redis::Transaction& cache_tx
            , videoSubtitleAsync::ptr bulletChatSync)
        :_mysqltx(sql_t), _cacheTx(cache_tx), _bulletChatSync(bulletChatSync)
    {
        
    }

    //新增单个弹幕
    void videoSubtitleTarget::insert(suiDataSql::suiVideoSubtitleTarget::ptr subtitle)
    {
        if(subtitle == nullptr)
            return;

        //优先新增缓存数据
        insertToCache(subtitle);
        
        //发布向数据库添加数据命令
        {
            //添加数据
            Json::Value _data;
            {
                Json::Value curdata;
                curdata[_map_video_id] = subtitle->getVideoId();
                curdata[_map_user_id] = subtitle->getUserId();
                curdata[_map_subtitle_id] = subtitle->getBulletchatId();
                curdata[_map_content] = subtitle->getBulletchat();
                curdata[_map_video_time] = subtitle->getSendByVideoTime();
                curdata[_map_create_time] = subtitle->getCreateTime();
                _data[async_insert_db_by_subtitle].append(curdata);
            }
            auto val = suiUtil::suiJson::serialize(_data);
            if(val.has_value())
            {
                if(_bulletChatSync)
                {
                    _bulletChatSync->async(val.value());
                }
                else
                {
                    WARN("videoSubtitleAsync is nullptr, 弹幕数据无法异步写入数据库");
                }
            }
        }
    }

    //发送者id，视频id，弹幕id，内容，视频偏移时间
    void videoSubtitleTarget::insert(const std::string& userid, const std::string& video_id
        , const std::string& subtitleid, const std::string& content, long long time)
    {
        suiDataSql::suiVideoSubtitleTarget::ptr subtitle = std::make_shared<suiDataSql::suiVideoSubtitleTarget>();
        subtitle->setVideoId(video_id);
        subtitle->setUserId(userid);
        subtitle->setBulletchatId(subtitleid);
        subtitle->setBulletchat(content);
        subtitle->setSendByVideoTime(time);
        subtitle->setCreateTime();
        this->insert(subtitle);
    }

    //查
    //查询某个视频中的弹幕
    suiDataSql::videoBulletChatList::ptr videoSubtitleTarget::select(const std::string& video_id)
    {
        //优先从缓存中查询
        auto ret = selectFromCache(video_id);
        if(ret)
            return ret;
        //此时缓存中没有，从数据库中查询
        ret = selectFromDb(video_id);
        insertToCache(ret);
        return ret;
    }

    //删
    //删除某个视频中的弹幕
    void videoSubtitleTarget::removeByVideoid(const std::string& video_id)
    {
        //异步删除数据库以及缓存
        {
            //使用_bulletChatSync来处理，否则无法保证同步性
            Json::Value _data;
            _data[async_delete_db_by_video_id].append(video_id);
            _data[async_delete_cache_by_video_id].append(video_id);
            auto val = suiUtil::suiJson::serialize(_data);
            if(val.has_value())
            {
                if(_bulletChatSync)
                {
                    _bulletChatSync->async(val.value());
                }
                else
                {
                    WARN("videoSubtitleAsync is nullptr, 弹幕数据无法异步写入数据库");
                }
            }
        }
    }

    //删除某个弹幕
    void videoSubtitleTarget::removeBySubtitleid(const std::string& video_id, const std::string& subtitleid)
    {
        //异步删除数据库
        {
            //添加数据
            Json::Value _data;
            _data[async_delete_db_by_subtitleid].append(subtitleid);
            auto val = suiUtil::suiJson::serialize(_data);
            if(val.has_value())
            {
                if(_bulletChatSync)
                {
                    _bulletChatSync->async(val.value());
                }
                else
                {
                    WARN("videoSubtitleAsync is nullptr, 弹幕数据无法异步写入数据库");
                }
            }
        }
        //同步删除缓存
        removeToCache(video_id, subtitleid);
    }

    void videoSubtitleTarget::removeBySubtitle(suiDataSql::suiVideoSubtitleTarget::ptr subtitle)
    {
        if(subtitle == nullptr)
            return;

        //异步删除数据库
        {
            //添加数据
            Json::Value _data;
            _data[async_delete_db_by_subtitleid].append(subtitle->getBulletchatId());
            auto val = suiUtil::suiJson::serialize(_data);
            if(val.has_value())
            {
                if(_bulletChatSync)
                {
                    _bulletChatSync->async(val.value());
                }
                else
                {
                    WARN("videoSubtitleAsync is nullptr, 弹幕数据无法异步写入数据库");
                }
            }
        }
        //同步删除缓存
        removeToCache(subtitle->getVideoId(), subtitle->getBulletchatId());
    }


    //向缓存插入
    void videoSubtitleTarget::insertToCache(suiDataSql::suiVideoSubtitleTarget::ptr subtitle)
    {
        //提取key
        auto key = getKey(subtitle->getVideoId());
        
        //判断redis中是否存在改字段
        auto handle = _cacheTx.redis();
        bool exists = handle.exists(key);
        std::unordered_map<std::string, std::string> _data;
        if(!exists)
        {
            //同步数据库
            auto bulletChatList = selectFromDb(subtitle->getVideoId());
            if(bulletChatList || !bulletChatList->list.empty())
            {
                for(auto& it : bulletChatList->list)
                {
                    //使用json
                    Json::Value curdata;
                    curdata[_map_video_id] = it.getVideoId();
                    curdata[_map_user_id] = it.getUserId();
                    curdata[_map_content] = it.getBulletchat();
                    curdata[_map_video_time] = it.getSendByVideoTime();
                    curdata[_map_create_time] = it.getCreateTime();
                    auto val = suiUtil::suiJson::serialize(curdata);
                    if(!val.has_value())
                    {
                        WARN("序列化失败");
                        continue;
                    }
                    _data[it.getBulletchatId()] = val.value();
                }
            }
            
        }
        //添加新信息
        {
            Json::Value curdata;
            curdata[_map_video_id] = subtitle->getVideoId();
            curdata[_map_user_id] = subtitle->getUserId();
            curdata[_map_content] = subtitle->getBulletchat();
            curdata[_map_video_time] = subtitle->getSendByVideoTime();
            curdata[_map_create_time] = subtitle->getCreateTime();
            auto val = suiUtil::suiJson::serialize(curdata);
            if(!val.has_value())
            {
                WARN("序列化失败");
            }
            else
            {
                _data[subtitle->getBulletchatId()] = val.value();
            }
        }
        //直接添加
        handle.hmset(key, _data.begin(), _data.end());
        handle.expire(key, std::chrono::seconds(_cache_expire));
    }

    //增加整个视频的弹幕缓存
    void videoSubtitleTarget::insertToCache(suiDataSql::videoBulletChatList::ptr bulletChatList)
    {
        if(!bulletChatList || bulletChatList->list.empty())
        {
            return ;
        }
        //有效
        //默认弹幕列表处于同一个视频下
        auto key = getKey(bulletChatList->list[0].getVideoId());
        std::unordered_map<std::string, std::string> _data;
        for(auto& it : bulletChatList->list)
        {
            //使用json
            Json::Value curdata;
            curdata[_map_video_id] = it.getVideoId();
            curdata[_map_user_id] = it.getUserId();
            curdata[_map_content] = it.getBulletchat();
            curdata[_map_video_time] = it.getSendByVideoTime();
            curdata[_map_create_time] = it.getCreateTime();
            auto val = suiUtil::suiJson::serialize(curdata);
            if(!val.has_value())
            {
                WARN("序列化失败");
                continue;
            }
            _data[it.getBulletchatId()] = val.value();
        }
        //数据定义完毕，直接进行添加
        auto handle = _cacheTx.redis();
        handle.hmset(key, _data.begin(), _data.end()); 
        handle.expire(key, std::chrono::seconds(_cache_expire));
    }

    //缓存查询
    suiDataSql::videoBulletChatList::ptr videoSubtitleTarget::selectFromCache(const std::string& video_id)
    {
        auto key = getKey(video_id);
        suiDataSql::videoBulletChatList::ptr out = std::make_shared<suiDataSql::videoBulletChatList>();
        {
            auto handle = _cacheTx.redis();

            //判断是否存在改字段
            bool exists = handle.exists(key);
            if(!exists)
                return nullptr;

            std::unordered_map<std::string, std::string> _data;
            handle.hgetall(key, std::inserter(_data, _data.begin()));
            for(auto& it : _data)
            {
                auto curdata = suiUtil::suiJson::unserialize(it.second);
                if(curdata.has_value())
                {
                    out->list.push_back(suiDataSql::suiVideoSubtitleTarget());
                    auto& dat = curdata.value();
                    auto& bulletchat = out->list.back();
                    bulletchat.setBulletchatId(it.first);
                    bulletchat.setVideoId(dat[_map_video_id].asString());
                    bulletchat.setUserId(dat[_map_user_id].asString());
                    bulletchat.setBulletchat(dat[_map_content].asString());
                    bulletchat.setSendByVideoTime(dat[_map_video_time].asUInt64());
                    bulletchat.setCreateTime(dat[_map_create_time].asUInt64());
                }
            }
        }
        return out;
    }

    //删除视频中某个弹幕缓存 -- 全量删除提供异步接口执行
    void videoSubtitleTarget::removeToCache(const std::string& video_id, const std::string& subtitleid)
    {
        std::string key = getKey(video_id);
        {
            auto handle = _cacheTx.redis();
            handle.hdel(key, subtitleid);
        }
    }

    //数据库查
    suiDataSql::videoBulletChatList::ptr videoSubtitleTarget::selectFromDb(const std::string& video_id)
    {
        suiDataSql::videoBulletChatList::ptr out = std::make_shared<suiDataSql::videoBulletChatList>();
        {
            //创建句柄
            auto& handle = _mysqltx.database();
            auto ret = handle.query<suiDataSql::suiVideoSubtitleTarget>(odb::query<suiDataSql::suiVideoSubtitleTarget>(
                odb::query<suiDataSql::suiVideoSubtitleTarget>::video_id == video_id
            ));

            for(auto& it : ret)
                out->list.push_back(std::move(it));
        }
        return out;
    }
    

    //异步接口
    //缓存全量删除
    void videoSubtitleTarget::removeToCache(const std::string& video_id)
    {
        std::string key = getKey(video_id);
        {
            auto handle = _cacheTx.redis();
            handle.del(key);
        }
    }

    //数据库增
    void videoSubtitleTarget::insertToDb(suiDataSql::suiVideoSubtitleTarget::ptr subtitle)
    {
        if(subtitle == nullptr)
            return;
        {
            auto& handle = _mysqltx.database();
            handle.persist(*subtitle);
        }
    }

    //数据库删
    void videoSubtitleTarget::removeBySubtitleidToDb(const std::string& subtitleid)
    {
        {
            auto& handle = _mysqltx.database();
            handle.erase_query<suiDataSql::suiVideoSubtitleTarget>(odb::query<suiDataSql::suiVideoSubtitleTarget>(
                odb::query<suiDataSql::suiVideoSubtitleTarget>::bulletchat_id == subtitleid
            ));
        }
    }

    void videoSubtitleTarget::removeByVideoidToDb(const std::string& video_id)
    {
        {
            auto& handle = _mysqltx.database();
            handle.erase_query<suiDataSql::suiVideoSubtitleTarget>(odb::query<suiDataSql::suiVideoSubtitleTarget>(
                odb::query<suiDataSql::suiVideoSubtitleTarget>::video_id == video_id
            ));
        }
    }

    std::string videoSubtitleTarget::getKey(const std::string& video_id)
    {
        return _cacheKeyPrefix + video_id;
    }

    //
    videoSubtitleAsync::videoSubtitleAsync(std::shared_ptr<odb::database> mysql, std::shared_ptr<sw::redis::Redis> redis, suiQueue::MQClient::ptr MqClientPtr)
        : _mysql(mysql)
        , _redis(redis)
    {
        //构造异步对象
        suiQueue::queueSetting syncset;
        syncset.exchange = "subtitle_process_exchange",
        syncset.exchangeType = "delayed",
        syncset.queue = "subtitle_process_queue",
        syncset.bindKey = "subtitle_process",
        syncset.ttl = 3000;
        _bulletChatSync = std::make_shared<suiCacheSync::CacheSyncClient>(MqClientPtr, syncset, std::bind(&videoSubtitleAsync::callback, this, std::placeholders::_1));
    }

    void videoSubtitleAsync::async(const std::string& body)
    {
        _bulletChatSync->syncCache(body);
    }

    bool videoSubtitleAsync::callback(std::string body)
    {
        //对body进行反序列化
        auto ret = suiUtil::suiJson::unserialize(body);
        if(ret.has_value())
        {
            auto& js = ret.value();
            //处理顺序: 数据库增, 数据库删， 数据库全量删除， 缓存全量删除

            //数据库新增单行数据
            if(js.isMember(async_insert_db_by_subtitle) && js[async_insert_db_by_subtitle].isArray())
            {
                //新增单个弹幕
                const Json::Value& list = js[async_insert_db_by_subtitle];
                insertBySubtitleidToDb(list);
            }

            //数据库单行删
            if(js.isMember(async_delete_db_by_subtitleid) && js[async_delete_db_by_subtitleid].isArray())
            {
                const Json::Value& list = js[async_delete_db_by_subtitleid];
                removeBySubtitleidToDb(list);
            }

            //数据库全量删
            if(js.isMember(async_delete_db_by_video_id) && js[async_delete_db_by_video_id].isArray())
            {
                const Json::Value& list = js[async_delete_db_by_video_id];
                removeByVideoidToDb(list);
            }

            //缓存全量删除
            if(js.isMember(async_delete_cache_by_video_id) && js[async_delete_cache_by_video_id].isArray())
            {
                const Json::Value& list = js[async_delete_cache_by_video_id];
                removeByVideoidToCache(list);
            }
        }
        return true;
    }

    void videoSubtitleAsync::insertBySubtitleidToDb(const Json::Value& list)
    {
        for(Json::ArrayIndex i = 0; i < list.size(); ++i)
        {
            const Json::Value& item = list[i];
            if(item.isObject())
            {
                suiDataSql::suiVideoSubtitleTarget::ptr subtitle = std::make_shared<suiDataSql::suiVideoSubtitleTarget>();
                subtitle->setVideoId(item[_map_video_id].asString());
                subtitle->setUserId(item[_map_user_id].asString());
                subtitle->setBulletchatId(item[_map_subtitle_id].asString());
                subtitle->setBulletchat(item[_map_content].asString());
                subtitle->setSendByVideoTime(item[_map_video_time].asUInt64());
                subtitle->setCreateTime(item[_map_create_time].asUInt64());
                try
                {
                    //构建事务
                    odb::transaction tx(_mysql->begin());
                    auto rtx = _redis->transaction(false, false);
                    videoSubtitleTarget target(tx, rtx, nullptr);
                    target.insertToDb(subtitle);
                    tx.commit();
                }
                catch (const odb::exception& e)
                {
                    // 捕获 ODB 数据库异常
                    ERROR("odb数据库添加弹幕异常： {}", e.what());
                    continue;
                }
                catch (const sw::redis::Error& e) 
                {
                    // 捕获 Redis 异常
                    ERROR("redis数据库添加弹幕异常： {}", e.what());
                    continue;
                }
                catch (...)
                {
                    ERROR("添加弹幕时未知异常");
                    continue;
                }
            }
            //
        }
        //
    }

    void videoSubtitleAsync::removeBySubtitleidToDb(const Json::Value& list)
    {
        for(Json::ArrayIndex i = 0; i < list.size(); ++i)
        {
            try
            {
                //构建事务
                odb::transaction tx(_mysql->begin());
                auto rtx = _redis->transaction(false, false);
                videoSubtitleTarget target(tx, rtx, nullptr);
                target.removeBySubtitleidToDb(list[i].asString());
                tx.commit();
            }
            catch (const odb::exception& e)
            {
                // 捕获 ODB 数据库异常
                ERROR("odb数据库删除弹幕异常： {}", e.what());
                continue;
            }
            catch (const sw::redis::Error& e) 
            {
                // 捕获 Redis 异常
                ERROR("redis数据库删除弹幕异常： {}", e.what());
                continue;
            }
            catch (...)
            {
                ERROR("删除弹幕时未知异常");
                continue;
            }
        }
    }

    void videoSubtitleAsync::removeByVideoidToDb(const Json::Value& list)
    {
        for(Json::ArrayIndex i = 0; i < list.size(); ++i)
        {
            try
            {
                //构建事务
                odb::transaction tx(_mysql->begin());
                auto rtx = _redis->transaction(false, false);
                videoSubtitleTarget target(tx, rtx, nullptr);
                target.removeByVideoidToDb(list[i].asString());
                tx.commit();
            }
            catch (const odb::exception& e)
            {
                // 捕获 ODB 数据库异常
                ERROR("odb数据库删除视频弹幕异常： {}", e.what());
                continue;
            }
            catch (const sw::redis::Error& e) 
            {
                // 捕获 Redis 异常
                ERROR("redis数据库删除视频弹幕异常： {}", e.what());
                continue;
            }
            catch (...)
            {
                ERROR("删除视频弹幕时未知异常");
                continue;
            }
        }
    }

    void videoSubtitleAsync::removeByVideoidToCache(const Json::Value& list)
    {
        for(Json::ArrayIndex i = 0; i < list.size(); ++i)
        {
            try
            {
                //构建事务
                odb::transaction tx(_mysql->begin());
                auto rtx = _redis->transaction(false, false);
                videoSubtitleTarget target(tx, rtx, nullptr);
                target.removeToCache(list[i].asString());
                tx.commit();
            }
            catch (const odb::exception& e)
            {
                // 捕获 ODB 数据库异常
                ERROR("odb数据库删除视频弹幕异常： {}", e.what());
                continue;
            }
            catch (const sw::redis::Error& e) 
            {
                // 捕获 Redis 异常
                ERROR("redis数据库删除视频弹幕异常： {}", e.what());
                continue;
            }
            catch (...)
            {
                ERROR("删除视频弹幕时未知异常");
                continue;
            }
        }
    }
}