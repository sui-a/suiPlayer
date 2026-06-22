#include "FileTarcker.hpp"

namespace suifileTarcker
{
    FileTracker::FileTracker(const FileTracker& tracker)
    {
        _fileid = tracker._fileid;
        _operation_type = tracker._operation_type;
        _file_size = tracker._file_size;
        _cur_operation_size = tracker._cur_operation_size;
    }

    void FileTracker::setFileId(const std::string& fileid)
    {
        _fileid = fileid;
    }

    void FileTracker::getFileId(std::string& fileid)
    {
        fileid = _fileid;
    }

    void FileTracker::setOperationType(FileTrackerOperationType operation_type)
    {
        _operation_type = operation_type;
    }

    FileTrackerOperationType FileTracker::getOperationType() const 
    {
        return _operation_type;
    }

    void FileTracker::setFileSize(int32_t file_size)
    {
        _file_size = file_size;
    }
    int32_t FileTracker::getFileSize() const
    {
        return _file_size;
    }

    void FileTracker::setCurOperationSize(int32_t cur_operation_size)
    {
        _cur_operation_size = cur_operation_size;
    }
    int32_t FileTracker::getCurOperationSize() const
    {
        return _cur_operation_size;
    }

    FileTracker& FileTracker::operator+=(const int32_t file_offset)
    {
        _cur_operation_size += file_offset;
        return *this;
    }

    void FileTracker::setChunkSize(int32_t chunk_size)
    {
        _chunk_size = chunk_size;
    }
    int32_t FileTracker::getChunkSize() const
    {
        return _chunk_size;
    }

    void FileTracker::setChunkIndex(int32_t chunk_index)
    {
        _chunk_index = chunk_index;
    }
    int32_t FileTracker::getChunkIndex() const
    {
        return _chunk_index;
    }
}