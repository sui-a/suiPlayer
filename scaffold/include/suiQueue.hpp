#pragma once
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <ev.h>
#include <string>
#include <functional>
#include <mutex>
#include <thread>   
#include <condition_variable>
#include "log.h"

namespace suiQueue
{

    struct queueSetting
    {
    public:
        std::string exchange; //交换机名称
        //交换机类型： direct, fanout, headers, topic, delayed
        std::string exchangeType;  
        std::string queue;  //队列名称
        std::string bindKey; //绑定键
        //消息过期时间，单位：毫秒
        size_t ttl;

        std::string dlxExchange() const;
        std::string dlxQueue() const;
        std::string dlxBindKey() const;
    };

    inline AMQP::ExchangeType getExchangeType(const std::string& type);

    //返回true则确认消息，false则拒绝消息
    using MessageCallback = std::function<bool(std::string)>;

    enum class suiMQState
    {
        stop,
        running,
        error
    };

    class MQClient
    {
    public:
        using ptr = std::shared_ptr<MQClient>;
        MQClient(const std::string& url);
        ~MQClient();

        //绑定/申请资源
        void declare(const queueSetting& s);
        bool publish(const std::string& exchange, const std::string& bindKey, const std::string& message);
        void consume(const queueSetting& s, MessageCallback& callback);
        void start();
        void wait();
        bool isRunning();

        suiQueue::suiMQState getState();
        std::string getError();
    private:
        static void callback(struct ev_loop* loop, ev_async* watcher, int32_t revents);
        //声明常规交换队列
        void declare_routine(const queueSetting& s, AMQP::Table& args, bool is_dlx);

        void setError(const std::string& msg);
        void setState(suiMQState state);

    private:
        //
        std::mutex _mtx_all;
        std::mutex _mtx;
        std::condition_variable _cv;
        struct ev_loop* _loop;
        struct ev_async _ev_async;
        AMQP::LibEvHandler _handler;
        AMQP::TcpConnection _connect;
        AMQP::TcpChannel _channel;
        std::thread _thread;

        //
        std::string _errorMsg;
        suiMQState _state;
    };

    class suiPublisher
    {
    public:
        using ptr = std::shared_ptr<suiPublisher>;
        suiPublisher(MQClient::ptr clientPtr, const queueSetting& s);
        ~suiPublisher();

        bool publish(const std::string& message);
        std::string getError();

        bool isError();
    private:
        MQClient::ptr _clientPtr;
        queueSetting _s;
    };

    class suiSubscriber
    {
    public:
        using ptr = std::shared_ptr<suiSubscriber>;

        suiSubscriber(MQClient::ptr clientPtr, const queueSetting& s);
        ~suiSubscriber();

        //绑定回调
        void consume(MessageCallback callback);

        bool isError();
        std::string getError();

    private:
        MQClient::ptr _clientPtr;
        MessageCallback _callback;
        queueSetting _s;
    };

    class suiMqFactory
    {
        public:
            template<typename T, typename... Args>
            static std::shared_ptr<T> create(Args&&... args)
            {
                return std::make_shared<T>(std::forward<Args>(args)...);
            }
    };

}