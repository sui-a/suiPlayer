#include "suiQueue.hpp"

namespace suiQueue
{
    std::string queueSetting::dlxExchange() const 
    {
        return "dlx_" + exchange;
    }

    std::string queueSetting::dlxQueue() const 
    {
        return "dlx_" + queue;
    }

    std::string queueSetting::dlxBindKey() const 
    {
        return "dlx_" + bindKey;
    }

    inline AMQP::ExchangeType getExchangeType(const std::string& type)
    {
        if(type == "direct")
        {
            //返回广播类型
            return AMQP::direct;
        }
        else if(type == "fanout" || type == "delayed")
        {
            //返回直连类型
            return AMQP::fanout;
        }
        else if(type == "headers")
        {
            return AMQP::headers;
        }
        else if(type == "topic")
        {
            return AMQP::topic;
        }
        //错误返回广播类型
        return AMQP::direct;
    }

    MQClient::MQClient(const std::string& url)
        : _loop(EV_DEFAULT)
        , _handler(_loop)
        , _connect(&_handler, AMQP::Address(url))
        , _channel(&_connect)
        , _state(suiMQState::running)
        , _thread([this](){
            ev_run(_loop);
        })
    {
        
    }

    MQClient::~MQClient()
    {
        ev_async_init(&_ev_async, MQClient::callback);
        ev_async_start(_loop, &_ev_async);
        ev_async_send(_loop, &_ev_async);
        _thread.join();
    }

    void MQClient::callback(struct ev_loop* loop, ev_async* watcher, int32_t revents)
    {
        ev_break(loop, EVBREAK_ALL);
    }

    void MQClient::setError(const std::string& msg)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _errorMsg = msg;
        ERROR("消息队列错误: {}", msg);
    }

    void MQClient::setState(suiMQState state)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _state = state;
    }

    bool MQClient::isRunning()
    {
        std::unique_lock<std::mutex> lock(_mtx);
        return _state == suiMQState::running;
    }

    suiQueue::suiMQState MQClient::getState()
    {
        return _state;
    }
    std::string MQClient::getError()
    {
        return _errorMsg;
    }

    void MQClient::declare_routine(const queueSetting& s, AMQP::Table& args, bool is_dlx)
    {
        std::unique_lock<std::mutex> lock2(_mtx_all);
        std::unique_lock<std::mutex> lock(_mtx);
        std::string exchange, queue, bindKey;
        if(is_dlx)
        {
            exchange = s.dlxExchange();
            queue = s.dlxQueue();
            bindKey = s.dlxBindKey();
        }
        else
        {
            exchange = s.exchange;
            queue = s.queue;
            bindKey = s.bindKey;
        }

        //声明交换机
        AMQP::ExchangeType type = getExchangeType(s.exchangeType);
        _channel.declareExchange(exchange, type).onSuccess([this, queue, bindKey, exchange, args]()->void{
            //声明成功，开始声明交换队列
            this->_channel.declareQueue(queue, AMQP::durable, args).onSuccess([this, queue, bindKey, exchange](){
                //开始绑定交换机与队列
                _channel.bindQueue(exchange, queue, bindKey).onSuccess([this](){
                    //成功
                    std::unique_lock<std::mutex> lock(this->_mtx);
                    this->_cv.notify_all();

                }).onError([this](const char* message)
                {
                    this->setError(message);
                    this->setState(suiMQState::error);
                    std::unique_lock<std::mutex> lock(this->_mtx);
                    this->_cv.notify_all();
                });

            }).onError([this](const char* message)
            {
                this->setError(message);
                this->setState(suiMQState::error);
                std::unique_lock<std::mutex> lock(this->_mtx);
                this->_cv.notify_all();
            });
        }).onError([this](const char* message) 
        {
            this->setError(message);
            this->setState(suiMQState::error);
            std::unique_lock<std::mutex> lock(this->_mtx);
            this->_cv.notify_all();
        });

        _cv.wait(lock);
    }


    void MQClient::declare(const queueSetting& s)
    {
        AMQP::Table args;
        declare_routine(s, args, false);
        if(s.exchangeType == "delayed")
        {
            //开启延迟队列
            args["x-dead-letter-exchange"] = s.exchange;
            args["x-dead-letter-routing-key"] = s.bindKey;
            args["x-message-ttl"] = s.ttl;
            declare_routine(s, args, true);
        }
    }

    bool MQClient::publish(const std::string& exchange, const std::string& bindKey, const std::string& message)
    {
        if(_state != suiMQState::running)
        {
            //上锁进行二次判断
            std::unique_lock<std::mutex> lock(_mtx);
            if(_state != suiMQState::running)
            {
                return false;
            }
        }
        _channel.publish(exchange, bindKey, AMQP::Envelope(message));
        return true;
    }

    void MQClient::consume(const queueSetting& s, MessageCallback& callback)
    {
        std::unique_lock<std::mutex> lock2(_mtx_all);
        std::unique_lock<std::mutex> lock(_mtx);
        _channel.consume(s.queue)
                .onMessage([callback, this](const AMQP::Message &message
                                            , uint64_t deliveryTag
                                            , bool redelivered) {
                if(callback != nullptr)
                {
                    std::string msg(message.body(), message.bodySize());
                    bool ret = callback(msg);
                    if(ret)
                        this->_channel.ack(deliveryTag);
                    else
                        this->_channel.reject(deliveryTag);
                }
            }).onSuccess([this](){
                //成功
                std::unique_lock<std::mutex> lock(this->_mtx);
                this->_cv.notify_all();
            }).onError([this](const char* message){
                this->setError(message);
                this->setState(suiMQState::error);
                std::unique_lock<std::mutex> lock(this->_mtx);
                this->_cv.notify_all();
            });

        _cv.wait(lock);
    }


    void MQClient::start()
    {

    }

    void MQClient::wait()
    {
        _thread.join();
    }

    suiPublisher::suiPublisher(MQClient::ptr clientPtr, const queueSetting& s)
        : _clientPtr(clientPtr)
        , _s(s)
    {
        clientPtr->declare(s);
    }

    suiPublisher::~suiPublisher()
    {
        
    }

    bool suiPublisher::publish(const std::string& message)
    {
        if(_s.exchangeType == "delayed")
        {
            //延迟队列，向私信队列发送数据
            return _clientPtr->publish(_s.dlxExchange(), _s.dlxBindKey(), message);
        }
        return _clientPtr->publish(_s.exchange, _s.bindKey, message);
    }

    bool suiPublisher::isError()
    {
        return _clientPtr->getState() == suiMQState::error;
    }

    std::string suiPublisher::getError()
    {
        return _clientPtr->getError();
    }

    suiSubscriber::suiSubscriber(MQClient::ptr clientPtr, const queueSetting& s)
        : _clientPtr(clientPtr)
        , _s(s)
        , _callback(nullptr)
    {
        clientPtr->declare(s);
    }

    suiSubscriber::~suiSubscriber()
    {}

    void suiSubscriber::consume(MessageCallback callback)
    {
        _callback = callback;
        _clientPtr->consume(_s, _callback);
    }

    bool suiSubscriber::isError()
    {
        return _clientPtr->getState() == suiMQState::error;
    }

    std::string suiSubscriber::getError()
    {
        return _clientPtr->getError();
    }
    
}