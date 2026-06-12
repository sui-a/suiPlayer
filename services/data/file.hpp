#pragma once 
#include <suiScaffold/suiodb.hpp>
#include "data.hpp"
#include "data-odb.hxx"


namespace suiFile
{
    class FileData
    {
    public:
        FileData(odb::database& db);
        ~FileData();

        void insert(suiDataSql::suiFileMeta& fileMeta);
        void update(suiDataSql::suiFileMeta& fileMeta);
        void remove(suiDataSql::suiFileMeta& fileMeta);

        suiDataSql::suiFileMeta::ptr selectFileByFileId(const std::string& id);

    private:
        odb::database& _db;
    };
}