#pragma once
#include <string>
#include <unordered_map>


namespace suifileTarcker
{
    enum class FileTrackerOperationType
    {
        Upload,
        Download,
    };

    class FileTracker
    {
    public:
        FileTracker() = default;
        FileTracker(const FileTracker& tracker);

        void setFileId(const std::string& fileid);
        void getFileId(std::string& fileid);

        void setOperationType(FileTrackerOperationType operation_type);
        FileTrackerOperationType getOperationType() const;

        void setFileSize(int32_t file_size);
        int32_t getFileSize() const;

        void setCurOperationSize(int32_t cur_operation_size);
        int32_t getCurOperationSize() const;

        FileTracker& operator+=(const int32_t file_offset);

        void setChunkSize(int32_t chunk_size);
        int32_t getChunkSize() const;

        void setChunkIndex(int32_t chunk_index);
        int32_t getChunkIndex() const;

        
    private:
        std::string _fileid;
        FileTrackerOperationType _operation_type;
        int32_t _file_size; //总大小
        int32_t _cur_operation_size; //已操作的大小

        //切片大小
        int32_t _chunk_size;
        int32_t _chunk_index; //当前需要的切片索引

    };
}