#pragma once
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
}
#include <memory>
#include <utility>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include "suiFile.hpp"


namespace suiPeg
{
    class M3U8Info
    {
    public:
        M3U8Info(std::string name);
        ~M3U8Info();

        bool parse();
        bool write();
        std::vector<std::string>& headers();
        std::vector<std::pair<std::string, std::string>>& pieces();

        void print();
    private:
        std::string _filename; //存储文件名
        std::vector<std::string> _headers; //存储头部信息
        std::vector<std::pair<std::string, std::string>> _pieces; //分片内容

        static inline std::string trim(const std::string& s);
        bool parseM3U8Content(const std::string& content);
    };
    struct hlsSettings
    {
        size_t hls_time;  //分片时间
        std::string playlistType;
        std::string baseUrl;
    };
    class HLSTranscoder
    {
    public:
        using ptr = std::shared_ptr<HLSTranscoder>;
        HLSTranscoder(hlsSettings& settings);
        ~HLSTranscoder();

        bool transcode(const std::string& input, const std::string& output);
    private:
        hlsSettings _settings;
    };

}

