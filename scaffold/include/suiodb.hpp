#pragma once
#include <odb/schema-catalog.hxx>
#include <odb/database.hxx>
#include <odb/mysql/transaction.hxx>
#include <odb/mysql/database.hxx>
#include <string>

namespace suiOdb
{
    struct odbSetting
    {
        std::string _host;
        unsigned int _port = 3306;
        std::string _user; 
        std::string _password;
        std::string _database;
        std::string _charset = "utf8";
        unsigned int _connection_pool_size = 10;
    };

    class dbFactory
    {
    public:
        static std::shared_ptr<odb::database> create(const odbSetting& settings);
    };


}