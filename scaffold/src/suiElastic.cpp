#include "suiElastic.hpp"

namespace sui
{
    esBase::esBase(const std::string& key) 
    : _key(key)
    {

    }

    std::string esBase::to_string()
    {
        Json::Value root;
        root[_key] = _value;
        return root.toStyledString();
    }

    Json::Value esBase::getValue()
    {
        return _value;
    }

    std::string esBase::getKey()
    {
        return _key;
    }

    bool esBase::isEmpty()
    {
        return _value.isNull();
    }

    bool esBase::exist(const std::string& key)
    {
        return _value.isMember(key);
    }


    esObject::esObject(const std::string& key) 
        : esBase(key)
    {

    }

    esObject& esObject::addElement(const std::string& key, esBase::Ptr val)
    {
        _map[key] = val;
        return *this;
    }

    esObject::Ptr esObject::newObject(const std::string& key)
    {
        esObject::Ptr newObj;
        newObj = std::dynamic_pointer_cast<esObject>(getElement(key));
        if(newObj != nullptr)
        {
            return newObj;
        }

        newObj = std::make_shared<esObject>(key);
        addElement(key, newObj);
        return newObj;
    }

    esObject::Ptr esObject::newArray(const std::string& key)
    {
        esObject::Ptr newObj;
        newObj = std::dynamic_pointer_cast<esObject>(getElement(key));
        if(newObj != nullptr)
        {
            return newObj;
        }

        newObj = std::dynamic_pointer_cast<esObject>(std::make_shared<esArray>(key));
        addElement(key, newObj);
        return newObj;  
    }

    std::string esObject::to_string()
    {
        Json::Value root = _value;
        for(auto it : _map)
        {
            root[it.first] = it.second->getValue();
        }
        return root.toStyledString();
    }

    bool esObject::existElement(const std::string& key)
    {
        return _map.find(key) != _map.end();
    }
    
    esBase::Ptr esObject::getElement(const std::string& key)
    {
        if(!existElement(key))
        {
            return nullptr; 
        }
        return _map[key];
    }

    bool esObject::isEmpty()
    {
        if(_value.empty() && _map.empty())
        {
            return true;
        }
        return false;
    }   

    Json::Value esObject::getValue()
    {
        Json::Value root = _value;
        for(auto it : _map)
        {
            root[it.first] = it.second->getValue();
        }
        return root;
    }

    esArray::esArray(const std::string& key)
        : esBase(key)
    {

    }

    std::shared_ptr<esObject> esArray::newObject(const std::string& key)
    {
        //直接构造
        auto curJson = std::make_shared<esObject>(key);
        addElement(curJson);
        return curJson;
    }

    std::shared_ptr<esArray> esArray::newArray(const std::string& key)
    {
        //直接构造
        auto curJson = std::make_shared<esArray>(key);
        addElement(curJson);
        return curJson;
    }

    void esArray::addElement(esBase::Ptr val)
    {
        _array.push_back(val);
    }
    std::string esArray::to_string()
    {
        Json::Value root = getValue();
        return root.toStyledString();
    }

    Json::Value esArray::getValue()
    {
        Json::Value root = _value;
        for(auto it : _array)
        {
            root.append(it->getValue());
        }
        return root;
    }

    suiTokenizer::suiTokenizer(const std::string& key)
        :esObject(key)
    {

    }

    suiAnalyzer::suiAnalyzer(const std::string& key)
        : esObject(key)
    {

    }

    suiAnalyzer& suiAnalyzer::tokenizer()
    {
        auto curToken = std::make_shared<suiTokenizer>();
        curToken->type();
        curToken->tokenizer();
        addElement(curToken->getKey(), curToken);
        return *this;
    }

    suiTokenizer& suiTokenizer::type(const std::string& val)
    {
        add("type", val);
        return *this;
    }

    suiTokenizer& suiTokenizer::tokenizer(const std::string& val)
    {
        add("tokenizer", val);
        return *this;
    }

    suiAnalysis::suiAnalysis(const std::string& key)
        : esObject(key)
    {

    }
 
    suiAnalysis& suiAnalysis::analyzer(const std::string& key)
    {
        if(getValue().isMember(key))
        {
            return *this;
        }
        auto curJson = std::make_shared<suiAnalyzer>(key);
        curJson->tokenizer();
        this->addElement(key, curJson);
        return *this;
    }

    suiAnalysis& suiAnalysis::analyzer(suiAnalyzer::Ptr ikmax)
    {
        this->addElement(ikmax->getKey(), ikmax);
        return *this;
    }

    suiSettings::suiSettings(const std::string& key)
        : esObject(key)
    {

    }

    suiSettings& suiSettings::analysis(suiAnalysis::Ptr analysis)
    {
        this->addElement(analysis->getKey(), analysis);
        return *this;
    }

    suiSettings& suiSettings::analysis()
    {
        if(existElement("analysis"))
        {
            return *this;
        }
        auto curJson = std::make_shared<suiAnalysis>();
        curJson->analyzer();
        this->addElement(curJson->getKey(), curJson);
        return *this;
    }

    suiMapping::suiMapping(const std::string& key)
        :esObject(key)
    {

    }

    suiMapping& suiMapping::setDynamic(const bool val)
    {
        add("dynamic", val);
        return *this;
    }

    suiProperties::suiProperties(const std::string& key)
        :esObject(key)
    {

    }

    suiProperties& suiProperties::addField(esBase::Ptr val)
    {
        addElement(val->getKey(), val);
        return *this;
    }

    suiProperties::Ptr suiMapping::getProperties()
    {
        suiProperties::Ptr curJson = std::dynamic_pointer_cast<suiProperties>(getElement("properties"));
        if(curJson != nullptr)
            return curJson;

        curJson = std::make_shared<suiProperties>();
        addElement("properties", curJson);
        return curJson;
    }

    suiIndexer::suiIndexer(const std::string& indexName)
        : esObject("")
        , suiRequest(indexName, "", "PUT", "")
    {

    }

    suiIndexer& suiIndexer::settings(suiSettings::Ptr settings)
    {
        this->addElement(settings->getKey(), settings);
        return *this;
    }

    suiIndexer& suiIndexer::settings(const std::string& key)
    {
        if(existElement(key))
        {
            return *this;
        }
        auto curJson = std::make_shared<suiSettings>(key);
        curJson->analysis();
        this->addElement(key, curJson);
        return *this;
    }

    suiMapping::Ptr suiIndexer::getMap()
    {
        auto curJson = std::dynamic_pointer_cast<suiMapping>(getElement("mappings"));
        if(curJson != nullptr)
            return curJson;

        curJson = std::make_shared<suiMapping>();
        addElement(curJson->getKey(), curJson);
        return curJson;
    }

    suiInserter::suiInserter(const std::string& indexName, const std::string& id)
        : esObject("")
        , suiRequest(indexName, "_doc", "POST", id)
    {

    }

    suiDoc::suiDoc(const std::string& key)
        :esObject(key)
    {

    }

    suiDeleter::suiDeleter(const std::string& indexName, const std::string& id)
        : esObject("")
        , suiRequest(indexName, "_doc", "DELETE", id)
    {

    }

    suiBool::Ptr suiDeleter::qbool()
    {
        auto curJson = getElement("bool");
        if(curJson != nullptr)
        {
            return std::dynamic_pointer_cast<suiBool>(curJson);
        }
        //不存在
        curJson = std::make_shared<suiBool>();
        addElement("bool", curJson);
        return std::dynamic_pointer_cast<suiBool>(curJson);
    }

    suiUpdate::suiUpdate(const std::string& indexName, const std::string& id)
        : esObject("")
        , suiRequest(indexName, "_update", "POST", id)
    {

    }

    suiDoc::Ptr suiUpdate::getDoc()
    {
        auto curJson = getElement("doc");
        if(curJson != nullptr)
        {
            return std::dynamic_pointer_cast<suiDoc>(curJson);
        }
        //不存在
        curJson = std::make_shared<suiDoc>();
        addElement("doc", curJson);
        return std::dynamic_pointer_cast<suiDoc>(curJson);
    }

    //
    esQObject::esQObject(const std::string& key)
        : esObject(key)
    {

    }

    suiTerm::suiTerm(const std::string& key)
        : esObject("term")
        , _name(key)
    {

    }

    suiTerm::Ptr esQObject::term(const std::string& key)
    {
        suiTerm::Ptr curJson = create<suiTerm>("term", key);
        return curJson;
    }

    suiTerms::suiTerms(const std::string& key)
        : esObject("terms")
        , _name(key)
    {

    }

    suiMatch::suiMatch(const std::string& name)
        : esObject("match")
        , _name(name)
    {

    }

    suiMultiMatch::suiMultiMatch()
        : esObject("multi_match")
    {

    }

    suiRange::suiRange(const std::string& field)
        : esObject("range")
    {
        //查找并添加
        _rangeObj = this->newObject(field);
    }

    void suiRange::setGt(const std::string& val)
    {
        _rangeObj->add("gt", val);
    }

    void suiRange::setLt(const std::string& val)
    {
        _rangeObj->add("lt", val);
    }

    void suiRange::setRange(const std::string& lt, const std::string& gt)
    {
        setGt(gt);
        setLt(lt);
    }

    suiMust::suiMust()
        : esQArray("must")
    {
        
    }

    suiMustNot::suiMustNot()
        : esQArray("must_not")
    {

    }


    suiShould::suiShould()
        : esQArray("should")
    {

    }

    suiBool::suiBool()
        :esQObject("bool")
    {

    }

    suiMust::Ptr suiBool::must()
    {
        auto curJson = std::dynamic_pointer_cast<suiMust>(getElement("must"));
        if(curJson != nullptr)
        {
            return std::dynamic_pointer_cast<suiMust>(curJson);
        }
        //不存在
        curJson = std::make_shared<suiMust>();
        addElement("must", curJson);
        return curJson;
    }

    suiMustNot::Ptr suiBool::mustNot()
    {
        auto curJson = std::dynamic_pointer_cast<suiMustNot>(getElement("must_not"));
        if(curJson != nullptr)
        {
            return std::dynamic_pointer_cast<suiMustNot>(curJson);
        }
        //不存在
        curJson = std::make_shared<suiMustNot>();
        addElement("must_not", curJson);
        return curJson;
    }

    suiShould::Ptr suiBool::should()
    {
        auto curJson = std::dynamic_pointer_cast<suiShould>(getElement("should"));
        if(curJson != nullptr)
        {
            return std::dynamic_pointer_cast<suiShould>(curJson);
        }
        //不存在
        curJson = std::make_shared<suiShould>();
        addElement("should", curJson);
        return curJson;
    }

    void suiBool::minimum_should_match(const size_t count)
    {
        add("minimum_should_match", count);
    }

    suiRequest::suiRequest(const std::string& indexName, const std::string& type, const std::string& operation, const std::string& id)
        : _indexName(indexName)
        , _type(type)
        , _operation(operation)
        , _id(id)
    {

    }

    void suiRequest::setIndex(const std::string& val)
    {
        _indexName = val;
    }

    void suiRequest::setType(const std::string& val)
    {
        _type = val;
    }

    void suiRequest::setOperation(const std::string& val)
    {
        _operation = val;
    }

    void suiRequest::setId(const std::string& val)
    {
        _id = val;
    }

    elasticlient::Client::HTTPMethod suiRequest::getMethod()
    {
        if(_operation == "GET")
            return elasticlient::Client::HTTPMethod::GET;
        else if(_operation == "POST")
            return elasticlient::Client::HTTPMethod::POST;
        else if(_operation == "DELETE")
            return elasticlient::Client::HTTPMethod::DELETE;
        else if(_operation == "PUT")
            return elasticlient::Client::HTTPMethod::PUT;

        //默认输出POST
        return elasticlient::Client::HTTPMethod::POST;
    }

    std::string suiRequest::getPath()
    {   
        std::string out = _indexName;
        if(_operation == "DELETE" && _id.empty())
            return out;
        if(!_type.empty())
        {
            out += "/" + _type;
            if(!_id.empty())
            {
                out += "/" + _id;
            }
        }
        return out;
    }

    std::string suiRequest::getIndex()
    {
        return _indexName;
    }

    std::string suiRequest::getType()
    {
        return _type;
    }

    std::string suiRequest::getOperation()
    {
        return _operation;
    }

    std::string suiRequest::getId()
    {
        return _id;
    }

    suiQuery::suiQuery()
        : esObject("query")
    {

    }

    std::shared_ptr<suiBool> suiQuery::QBool()
    {
        //
        auto curJson = std::dynamic_pointer_cast<suiBool>(getElement("bool"));
        if(curJson != nullptr)
        {
            return std::dynamic_pointer_cast<suiBool>(curJson);
        }
        //不存在
        curJson = std::make_shared<suiBool>();
        addElement("bool", std::dynamic_pointer_cast<esBase>(curJson));
        return curJson;
    }

    suiSearch::suiSearch(const std::string& indexName)
        : esQObject("")
        , suiRequest(indexName, "_doc", "GET", "")
    {

    }

    suiQuery::Ptr suiSearch::query()
    {
        suiQuery::Ptr curJson = std::dynamic_pointer_cast<suiQuery>(getElement("query"));
        if(curJson != nullptr)
        {
            return curJson;
        }
        //不存在
        curJson = std::make_shared<suiQuery>();
        addElement("query", curJson);
        return curJson;
    }

    void suiQuery::setMatchAll()
    {
        add("match_all", Json::Value(Json::ValueType::objectValue));
    }
    
    suiTerms::Ptr esQObject::terms(const std::string& key) 
    {
        suiTerms::Ptr curJson = create<suiTerms>("terms", key);
        return curJson;
    }

    suiMatch::Ptr esQObject::match(const std::string& key)
    {
        suiMatch::Ptr curJson = create<suiMatch>("match", key);
        return curJson;
    }

    suiRange::Ptr esQObject::range(const std::string& key)
    {
        suiRange::Ptr curJson = create<suiRange>("range", key);
        return curJson;
    }

    suiQuery::Ptr esQObject::query()
    {
        if(existElement("query"))
        {
            return std::dynamic_pointer_cast<suiQuery>(getElement("query"));
        }
        suiQuery::Ptr curJson = std::make_shared<suiQuery>();
        addElement("query", curJson);
        return curJson;
    }

    suiMultiMatch::Ptr esQObject::multi_match()
    {
        if(existElement("multi_match"))
        {
            return std::dynamic_pointer_cast<suiMultiMatch>(getElement("multi_match"));
        }
        suiMultiMatch::Ptr curJson = std::make_shared<suiMultiMatch>();
        addElement("multi_match", curJson);
        return curJson;
    }

    esQArray::esQArray(const std::string& key)
        : esArray(key)
    {

    }

    suiTerm::Ptr esQArray::term(const std::string& key)
    {
        auto curJson = newObject("");
        auto curTerm = std::make_shared<suiTerm>(key);
        curJson->addElement("term", curTerm);
        return curTerm;
    }

    suiTerms::Ptr esQArray::terms(const std::string& key)
    {
        auto curJson = newObject("");
        auto curTerm = std::make_shared<suiTerms>(key);
        curJson->addElement("terms", curTerm);
        return curTerm;
    }

    suiMatch::Ptr esQArray::match(const std::string& key)
    {
        auto curJson = newObject("");
        auto curTerm = std::make_shared<suiMatch>(key);
        curJson->addElement("match", curTerm);
        return curTerm;
    }

    suiRange::Ptr esQArray::range(const std::string& key)
    {
        auto curJson = newObject("");
        auto curTerm = std::make_shared<suiRange>(key);
        curJson->addElement("range", curTerm);
        return curTerm;
    }

    suiMultiMatch::Ptr esQArray::multi_match()
    {
        auto curJson = newObject("");
        auto curTerm = std::make_shared<suiMultiMatch>();
        curJson->addElement("multi_match", curTerm);
        return curTerm;
    }

    esClient::esClient(const std::vector<std::string>& hosts)
        : _client(std::make_shared<elasticlient::Client>(hosts))
    {
        
    }

    bool esClient::create(suiIndexer& idxer)
    {
        //获取信息
        auto body = idxer.to_string();
        auto method = idxer.getMethod();
        auto path = idxer.getPath();
        auto resp = _client->performRequest(
                method,
                path,
                body
        );
        if(resp.status_code >= 200 && resp.status_code < 300)
            return true;

        
        return false;
    }
    
    bool esClient::insert(suiInserter& idxer)
    {
        auto body = idxer.to_string();
        auto method = idxer.getMethod();
        auto path = idxer.getPath();
        if(idxer.isEmpty())
        {
            body = "";
        }
        auto resp = _client->performRequest(
                method,
                path,
                body
        );

        if(resp.status_code >= 200 && resp.status_code < 300)
        {
            return true;
        }
        return false;
    }

    bool esClient::update(suiUpdate& idxer)
    {
        auto body = idxer.to_string();
        auto method = idxer.getMethod();
        auto path = idxer.getPath();
        if(idxer.isEmpty())
        {
            body = "";
        }
        auto resp = _client->performRequest(
                method,
                path,
                body
        );
        if(resp.status_code >= 200 && resp.status_code < 300)
        {
            return true;
        }
        std::cout << "请求失败: " << resp.text << std::endl;
        return false;
    }
    
    bool esClient::remove(suiDeleter& idxer)
    {
        auto body = idxer.to_string();
        auto method = idxer.getMethod();
        auto path = idxer.getPath();
        if(idxer.isEmpty())
        {
            body = "";
        }
        auto resp = _client->performRequest(
                method,
                path,
                body
        );
        if(resp.status_code >= 200 && resp.status_code < 300)
        {
            return true;
        }
        return false;
    }

    bool esClient::remove(std::string idxer)
    {
        auto resp = _client->performRequest(
                elasticlient::Client::HTTPMethod::DELETE,
                idxer,
                ""
        );
        if(resp.status_code >= 200 && resp.status_code < 300)
        {
            return true;
        }
        return false;
    }

    std::optional<std::string> esClient::search(suiSearch& idxer)
    {
        auto body = idxer.to_string();
        auto method = idxer.getMethod();
        auto path = idxer.getPath();
        if(idxer.isEmpty())
        {
            body = "";
        }
        auto resp = _client->performRequest(
                method,
                path,
                body
        );
        if(resp.status_code >= 200 && resp.status_code < 300)
        {
            return resp.text;
        }
        return std::nullopt;
    }

    
}