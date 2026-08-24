#include "suiElastic.hpp"
#include <iostream>

void create(suiest::esClient& client)
{
    suiest::suiIndexer indexer("students");
    {
        {
            indexer.settings();
        }
        {
            auto map = indexer.getMap(); 
            auto pro = map->setDynamic(false).getProperties();
            {
                auto name = std::make_shared<suiest::esBase>("name");
                name->add("type", "text");
                name->add("analyzer", "ik_max_word");
                pro->addField(name);

                auto age = std::make_shared<suiest::esBase>("age");
                age->add("type", "integer");
                pro->addField(age);

                auto skills = std::make_shared<suiest::esBase>("skills");
                skills->add("type", "text");
                pro->addField(skills);

                auto birth = std::make_shared<suiest::esBase>("birth");
                birth->add("type", "date");
                birth->add("index", true);
                pro->addField(birth);
            }
        }
    }
    
    if(client.create(indexer))
    {
        std::cout << "成功" << std::endl;
        return;
    }
    std::cout << "失败" << std::endl;
}

void search_for_id(suiest::esClient& client)
{
    suiest::suiSearch search("students");
    {
        search.setId("Sy7TNJ4BVsd-lVIDwOzk");

        auto ret = client.search(search);
        if(ret.has_value())
        {
            Json::Value root;
            Json::Reader reader;
            reader.parse(ret.value(), root);
            std::cout << ret.value() << std::endl;
            std::string name = root["_source"]["name"].asString();
            std::cout << name << std::endl;
        }
        else 
        {
            std::cout << "错误" << std::endl;
        }
    }
}

void insert(suiest::esClient& client)
{
    suiest::suiInserter Inserter("students");
    Inserter.setId("Sy7TNJ4BVsd-lVIDwOzisy2");
    Inserter.add("name", "李四");
    Inserter.add("age", 15);
    Inserter.add("skills", "C++, Elasticsearch");
    Inserter.add("birth", "2004-05-16");
    Inserter.add("sex", "man");


    if(client.insert(Inserter))
    {
        std::cout << "成功" << std::endl;
        return;
    }
    std::cout << "失败" << std::endl;
    return ;
}

void update(suiest::esClient& client)
{
    suiest::suiUpdate update("students", "Sy7TNJ4BVsd-lVIDwOzk");
    {
        {
            auto doc = update.getDoc();
            doc->add("parent", "王五");
        }
    }

    if(!client.update(update))
    {
        std::cout << "失败" << std::endl;
        return;
    }
    std::cout << "成功" << std::endl;
    return ;
}

void mydelete(suiest::esClient& client)
{
    suiest::suiDeleter del("students", "Sy7TNJ4BVsd-lVIDwOzk");
    if(client.remove(del))
        std::cout << "成功" << std::endl;
}

void search_by_name(suiest::esClient& client)
{
    suiest::suiSearch search("students");
    // search.setFrom(1);
    // search.setSize(2);
    {
        auto query = search.query();
        auto boolkey = query->QBool();
        auto must = boolkey->must();
        {
            //创建match查询字段
            //auto match = must->match("name");
            //match->setValue("李");
            auto range = must->range("age");
            range->setGt("10");
            range->setLt("30");
        }
        auto sort = search.sort();
        sort->addOrder("age", suiest::sortType::ascending);
    }
    std::cout << search.to_string() << std::endl;
    auto ret = client.search(search);
    if(ret.has_value())
    {
        std::cout << ret.value() << std::endl;
    }
    else
    {
        std::cout << "错误" << std::endl;
    }
}

int main()
{
    suiest::esClient client({"http://elastic:suisuipingan@127.0.0.1:8085/"});
    {
        //create(client);
        //insert(client);
        //search_for_id(client);
        //update(client);
        //mydelete(client);
        search_by_name(client);
    }
    return 0;   
}