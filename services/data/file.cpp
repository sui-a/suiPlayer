#include "file.hpp"

namespace suiFile
{
    FileData::FileData(odb::database& db)
        :_db(db)
    {

    }
    FileData::~FileData()
    {

    }

    void FileData::insert(suiDataSql::suiFileMeta& fileMeta)
    {
        _db.persist(fileMeta);
    }
    void FileData::update(suiDataSql::suiFileMeta& fileMeta)
    {
        _db.update(fileMeta);
    }
    void FileData::remove(suiDataSql::suiFileMeta& fileMeta)
    {
        _db.erase_query<suiDataSql::suiFileMeta>(odb::query<suiDataSql::suiFileMeta>::file_id == fileMeta.getFileId());
    }

    suiDataSql::suiFileMeta::ptr FileData::selectFileByFileId(const std::string& fileid)
    {
        typedef odb::query<suiDataSql::suiFileMeta> Query;
        suiDataSql::suiFileMeta::ptr res(_db.query_one<suiDataSql::suiFileMeta>(Query::file_id == fileid));
        return res;
    }

    void FileData::removeByFileId(const std::string& fileId)
    {
        auto fileMeta = selectFileByFileId(fileId);
        if(fileMeta)
            remove(*fileMeta);
    }
}