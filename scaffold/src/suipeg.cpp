#include "suipeg.hpp"

namespace suiPeg
{
    M3U8Info::M3U8Info(std::string name)
        :_filename(name)
    { }
    M3U8Info::~M3U8Info()
    { }

    bool M3U8Info::parse()
    {
        std::string context;
        bool rel = suiFile::suiFiler::read(_filename, context);
        if(rel == false)
            return false;

        //读取成功，可以直接解析
        rel = parseM3U8Content(context);
        return rel;
    }
    bool M3U8Info::write()
    {
        std::stringstream context;
        context << "#EXTM3U\n";
        for(auto it : _headers)
            context << it << "\n";
        for(auto it : _pieces)
            context << "#EXTINF:" << it.first << ",\n"
                << it.second << "\n";
        context << "#EXT-X-ENDLIST\n";

        bool rel = suiFile::suiFiler::write(_filename, context.str());
        return true;
    }
    std::vector<std::string>& M3U8Info::headers()
    {
        return _headers;
    }
    std::vector<std::pair<std::string, std::string>>& M3U8Info::pieces()
    {
        return _pieces;
    }

    inline std::string M3U8Info::trim(const std::string& s) 
    {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    void M3U8Info::print()
    {
        std::cout << "========= headers ===========" << std::endl;
        for(auto it : _headers)
        {
            std::cout << it << std::endl;
        }

        std::cout << "========= pieces ===========" << std::endl;

        for(auto it : _pieces)
        {
            std::cout << it.first << " : " << it.second << std::endl;
        }
        std::cout << "=============================" << std::endl;
    }

    bool M3U8Info::parseM3U8Content(const std::string& content)
    {
        std::istringstream stream(content);
        std::string line;
        std::string pendingDuration;   // 暂存 #EXTINF 的时长部分

        while (std::getline(stream, line)) 
        {
            std::string trimmed = trim(line);
            if (trimmed.empty()) continue;

            // 不以 '#' 开头 -> 分片 URL 行
            if (trimmed[0] != '#') 
            {
                if (!pendingDuration.empty()) 
                {
                    _pieces.emplace_back(pendingDuration, trimmed);
                    pendingDuration.clear();
                } else {
                    // 孤立 URL 行，可根据需要处理或忽略
                    // 这里选择忽略并给出警告（可注释掉）
                    // std::cerr << "警告: 发现孤立的分片 URL 行: " << trimmed << std::endl;
                }
                continue;
            }

            // 以 '#' 开头，跳过 #EXTM3U 和 #EXT-X-ENDLIST
            if (trimmed == "#EXTM3U" || trimmed == "#EXT-X-ENDLIST") {
                continue;
            }

            // 检查是否是 #EXTINF 行
            if (trimmed.compare(0, 8, "#EXTINF:") == 0) {
                std::string contentPart = trimmed.substr(8);   // 去掉 "#EXTINF:"
                size_t commaPos = contentPart.find(',');
                if (commaPos != std::string::npos) {
                    pendingDuration = trim(contentPart.substr(0, commaPos));
                } else {
                    pendingDuration = trim(contentPart);       // 无逗号时整段作为时长
                }
            } else {
                // 其他 '#' 开头的行视为头部元数据
                _headers.push_back(trimmed);
            }
        }

        // 文件末尾未配对的 EXTINF 忽略，不清除 pendingDuration 也没关系
        return true;
    }

    HLSTranscoder::HLSTranscoder(hlsSettings& settings)
        :_settings(settings)
    { }

    HLSTranscoder::~HLSTranscoder()
    { }

    bool HLSTranscoder::transcode(const std::string& input, const std::string& output)
    {
        AVFormatContext* inputContext = nullptr;
        AVFormatContext* outputContext = nullptr;

        //打开输入文件，创建输入格式上下文对象
        int ret = avformat_open_input(&inputContext, input.c_str(), nullptr, nullptr);
        if(ret < 0)
            return false;
        //通过输入格式化上下文对象解析文件元信息
        ret = avformat_find_stream_info(inputContext, nullptr);
        if(ret < 0)
        {
            avformat_close_input(&inputContext);
            return false;
        }

        //申请创建输出格式化上下文对象，并设定输出格式hls
        ret = avformat_alloc_output_context2(&outputContext, nullptr, "hls", output.c_str());
        if(ret < 0)
        {
            avformat_close_input(&inputContext);
            return false;
        }
        //遍历输入格式化上下文中的媒体流信息，为输出格式化上下文对象创建媒体流，并复制编解码器参数
        for(int i = 0; i < inputContext->nb_streams; i++)
        {
            AVStream* inputStream = inputContext->streams[i];
            AVStream* outputStream = avformat_new_stream(outputContext, nullptr);
            avcodec_parameters_copy(outputStream->codecpar, inputStream->codecpar);
            outputStream->avg_frame_rate = inputStream->avg_frame_rate;
            outputStream->r_frame_rate = inputStream->r_frame_rate;
            outputStream->time_base = inputStream->time_base;
        }
        //设置hls转码的各项细节参数，播放类型-点播vod, 分片时间，路径前缀， http://192.168.0.0:9000/video/xxx0.tx
        AVDictionary *dict = nullptr;
        av_dict_set_int(&dict, "hls_time", _settings.hls_time, 0);
        av_dict_set(&dict, "hls_base_url", _settings.baseUrl.c_str(), 0);
        av_dict_set(&dict, "hls_playlist_type",  _settings.playlistType.c_str(), 0);
        av_dict_set(&dict, "hls_flags",  "independent_segments", 0);
        //通过输出格式化上下文，向输出文件写入头部信息
        ret =  avformat_write_header(outputContext, &dict);
        if(ret < 0)
        {
            av_dict_free(&dict);
            avformat_free_context(outputContext);
            avformat_close_input(&inputContext);
            return false;
        }
        //遍历输入格式化上下文中的数据帧
        AVPacket pkt;
        while(av_read_frame(inputContext, &pkt) == 0)
        {
            AVStream* inputStream = inputContext->streams[pkt.stream_index];
            AVStream* outputStream = outputContext->streams[pkt.stream_index];

            //进行时间基转换，从输入媒体流时间基转换为输出媒体流时间基
            if(pkt.pts == AV_NOPTS_VALUE)
            {
                //若当前数据帧显示时间戳无效，则将时间戳设置为从0开始的偏移量
                pkt.pts = av_rescale_q(0, AV_TIME_BASE_Q, inputStream->time_base);
                pkt.dts = pkt.pts;
            }
            av_packet_rescale_ts(&pkt, inputStream->time_base, outputStream->time_base);
            //将数据帧通过输出格式化上下文对象，写入输出文件中
            ret = av_interleaved_write_frame(outputContext, &pkt);
            if(ret < 0)
            {
                av_dict_free(&dict);
                avformat_free_context(outputContext);
                avformat_close_input(&inputContext);
                return false;
            }
            //释放数据帧
            av_packet_unref(&pkt);
        }
        //向输出文件写入文件尾部信息
        ret = av_write_trailer(outputContext);
        if(ret < 0)
        {
            //出现错误
            av_dict_free(&dict);
            avformat_free_context(outputContext);
            avformat_close_input(&inputContext);
            return false;
        }

        //转码完成，释放资源 /参数字典对象 /输入格式上下文对象 /输出格式上下文对象
        av_dict_free(&dict);
        avformat_free_context(outputContext);
        avformat_close_input(&inputContext);
        return 0;
    }

}