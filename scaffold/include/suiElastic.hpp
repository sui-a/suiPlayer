#pragma once 
#include <string>
#include <map>
#include <memory>
#include <jsoncpp/json/json.h>
#include <vector>
#include <cpr/response.h>
#include <elasticlient/client.h>
#include <optional>
#include <iostream>

namespace suies
{
    class esBase
    {
    public:
        using Ptr = std::shared_ptr<esBase>;

        esBase(const std::string& key);

        template<typename T>
        esBase& add(const std::string& key, const T& val)
        {
            _value[key] = val;
            return *this;
        }

        template<typename T>
        esBase& append(const std::string& key, const T& val)
        {
            _value[key].append(val);
            return *this;
        }

        virtual std::string to_string();
        virtual Json::Value getValue();
        virtual std::string getKey();

        //判空
        virtual bool isEmpty();

        virtual bool exist(const std::string& key);
    protected:
        std::string _key;
        Json::Value _value;
    };

    class esArray;
    class esObject : public esBase
    {
    public:
        using Ptr = std::shared_ptr<esObject>;

        esObject(const std::string& key);

        esObject& addElement(const std::string& key, esBase::Ptr val);
        esObject::Ptr newObject(const std::string& key);
        esObject::Ptr newArray(const std::string& key);

        std::string to_string() override;
        Json::Value getValue() override;

        bool existElement(const std::string& key);
        esBase::Ptr getElement(const std::string& key);

        virtual bool isEmpty() override;
    private:
        std::map<std::string, esBase::Ptr> _map;
    };

    class esArray: public esBase
    {
    public:
        using Ptr = std::shared_ptr<esArray>;

        esArray(const std::string& key);

        //
        std::shared_ptr<esObject> newObject(const std::string& key);
        std::shared_ptr<esArray> newArray(const std::string& key);
        void addElement(esBase::Ptr val);
        virtual std::string to_string() override;
        virtual Json::Value getValue() override;

    private:
        std::vector<esBase::Ptr> _array;

    };

    class suiTokenizer: public esObject
    {
    public:
        suiTokenizer(const std::string& key = "ikmax");

        suiTokenizer& type(const std::string& val = "custom");
        
        suiTokenizer& tokenizer(const std::string& val = "ik_max_word");

    };

    class suiAnalyzer: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiAnalyzer>;

        suiAnalyzer(const std::string& key = "analyzer");

        suiAnalyzer& tokenizer();
    private:

    };

    class suiAnalysis: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiAnalysis>;

        suiAnalysis(const std::string& key = "analysis");

        suiAnalysis& analyzer(const std::string& key = "analyzer");

        suiAnalysis& analyzer(suiAnalyzer::Ptr ikmax);


    private:

    };

    class suiSettings: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiSettings>;

        suiSettings(const std::string& key = "settings");

        suiSettings& analysis(suiAnalysis::Ptr analysis);
        suiSettings& analysis();

    private:
    
    };

    class suiProperties: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiProperties>;

        suiProperties(const std::string& key = "properties");
        
        suiProperties& addField(esBase::Ptr val);

    private:

    };

    class suiMapping: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiMapping>;

        suiMapping(const std::string& key = "mappings");

        suiMapping& setDynamic(const bool val);
        suiProperties::Ptr getProperties();

    private:

    };

    class suiTerm: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiTerm>;

        suiTerm(const std::string& key);

        template<typename T>
        void setValue(const T& val)
        {
            add(_name, val);
        }
    private:
        std::string _name;
    };

    class suiTerms: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiTerms>;

        suiTerms(const std::string& key);

        template<typename T>
        void appendValue(const T& val)
        {
            append(_name, val);    
        }
    
    private:
        std::string _name;
    };

    class suiMatch: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiMatch>;

        suiMatch(const std::string& name);

        template<typename T>
        void setValue(const T& val)
        {
            add(_name, val);
        }

    private:
        std::string _name;
    };

    class suiMultiMatch: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiMultiMatch>;

        suiMultiMatch();

        template<typename T>
        void appendField(const T& val)
        {
            append("fields", val);
        }

        template<typename T>
        void setQuery(const T& val)
        {
            add("query", val);
        }

        template<typename T>
        void setType(const T& val)
        {
            add("type", val);
        }

        template<typename T>
        void setFuzziness(const T& val)
        {
            add("fuzziness", val);
        }

        template<typename T>
        void setOperator(const T& val)
        {
            add("operator", val);
        }

    private:
        std::string _name;
    };

    class suiRange: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiRange>;

        suiRange(const std::string& field);

        void setGt(const std::string& val);
        void setLt(const std::string& val);

        void setRange(const std::string& gt, const std::string& lt);

    private:
        esObject::Ptr _rangeObj;
    };

    class esQArray: public esArray
    {
    public:
        using Ptr = std::shared_ptr<esQArray>;

        esQArray(const std::string& key = "QArray");

        suiTerm::Ptr term(const std::string& key);
        suiTerms::Ptr terms(const std::string& key);
        suiMatch::Ptr match(const std::string& key);
        suiRange::Ptr range(const std::string& key);
        suiMultiMatch::Ptr multi_match();

    private:

    };

    class suiMust: public esQArray
    {
    public:
        using Ptr = std::shared_ptr<suiMust>;
        suiMust();
    };

    class suiMustNot: public esQArray
    {
    public:
        using Ptr = std::shared_ptr<suiMustNot>;
        suiMustNot();

    };

    class suiShould: public esQArray
    {
    public:
        using Ptr = std::shared_ptr<suiShould>;
        suiShould();

    };

    class suiBool;
    class suiQuery: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiQuery>;
        suiQuery();
        void setMatchAll();
        std::shared_ptr<suiBool> QBool();

    private:

    };

    class esQObject: public esObject
    {
    public:
        using Ptr = std::shared_ptr<esQObject>;

        esQObject(const std::string& key = "QObject");

        suiTerm::Ptr term(const std::string& key);
        suiTerms::Ptr terms(const std::string& key);
        suiMatch::Ptr match(const std::string& key);
        suiRange::Ptr range(const std::string& key);
        suiQuery::Ptr query();
        suiMultiMatch::Ptr multi_match();

    private:
        template<typename T, typename T2>
        std::shared_ptr<T> create(const std::string& key, const T2& valKey)
        {
            if(existElement(key))
            {
                return std::dynamic_pointer_cast<T>(getElement(key));
            }
            //进行创建
            auto curJson = std::make_shared<T>(valKey);
            this->addElement(key, curJson);
            return curJson;
        }
    };

    class suiFilter: public esQArray
    {
    public:
        using Ptr = std::shared_ptr<suiFilter>;
        suiFilter();

        suiTerm::Ptr getFilterTerm(const std::string& key);
    private:

    };

    class suiBool: public esQObject
    {
    public:
        using Ptr = std::shared_ptr<suiBool>;
        suiBool();

        suiMust::Ptr must();
        suiMustNot::Ptr mustNot();
        suiShould::Ptr should();
        suiFilter::Ptr filter();

        void minimum_should_match(const size_t count);
    };

    class suiRequest
    {
    public:
        suiRequest(const std::string& indexName, const std::string& type, const std::string& operation, const std::string& id);

        void setIndex(const std::string& val);
        void setType(const std::string& val);
        void setOperation(const std::string& val);
        void setId(const std::string& val);
        elasticlient::Client::HTTPMethod getMethod();
        std::string getPath();

        std::string getIndex();
        std::string getType();
        std::string getOperation();
        std::string getId();

    private:
    std::string _indexName;
    std::string _type;
    std::string _operation;
    std::string _id;

    };

    class suiIndexer: public esObject, public suiRequest
    {
    public:
        using Ptr = std::shared_ptr<suiIndexer>;

        suiIndexer(const std::string& indexName);

        suiIndexer& settings(suiSettings::Ptr settings);
        suiIndexer& settings(const std::string& key = "settings");

        suiMapping::Ptr getMap();

    private:

    };

    class suiInserter: public esObject, public suiRequest
    {
    public:
        suiInserter(const std::string& indexName, const std::string& id = "");

    private:

    };

    class suiDoc: public esObject
    {
    public:
        using Ptr = std::shared_ptr<suiDoc>;
        
        suiDoc(const std::string& key = "doc");
    };

    class suiUpdate: public esObject, public suiRequest
    {
    public:
        suiUpdate(const std::string& indexName, const std::string& id);

        suiDoc::Ptr getDoc();

    };

    class suiDeleter: public esObject, public suiRequest
    {
    public:
        suiDeleter(const std::string& indexName, const std::string& id = "");
        suiBool::Ptr qbool();

    };

    enum class sortType
    {
        ascending = 0, //升序
        descending = 1 //降序
    };

    class suiSort: public esArray
    {
    public:
        using Ptr = std::shared_ptr<suiSort>;
        suiSort();

        void addOrder(const std::string& key, sortType type);
    };

    class suiSearch: public esQObject, public suiRequest
    {
    public:
        suiSearch(const std::string& indexName);
        suiQuery::Ptr query();
        suiSort::Ptr sort();

        //开启_source
        void setSource();

        void setFrom(const size_t count);
        void setSize(const size_t count);
    private:

    };

    class BaseClient
    {
    public:
        BaseClient() = default;
        ~BaseClient() = default;
        virtual bool create(suiIndexer& idxer) = 0; //创建索引
        virtual bool insert(suiInserter& idxer) = 0; //插入文档
        virtual bool update(suiUpdate& idxer) = 0; //更新文档
        virtual bool remove(suiDeleter& idxer) = 0; //删除文档
        virtual bool remove(std::string idxer) = 0; //删除索引
        virtual std::optional<std::string> search(suiSearch& idxer) = 0; //搜索文档
    };

    class esClient: public BaseClient
    {
    public:
        esClient(const std::vector<std::string>& host);
        bool create(suiIndexer& idxer) override;
        bool insert(suiInserter& idxer) override;
        bool update(suiUpdate& idxer) override;
        bool remove(suiDeleter& idxer) override;
        bool remove(std::string idxer) override;
        std::optional<std::string> search(suiSearch& idxer) override;
    
    private:
        std::shared_ptr<elasticlient::Client> _client;
    };

    //
}    