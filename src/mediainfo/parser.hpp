#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace porla::MediaInfo
{
    struct StreamGeneral
    {
        std::optional<std::int64_t> bitrate_overall;
        std::optional<std::string>  codec_id;
        std::optional<std::int64_t> duration;
        std::optional<std::string>  format;
        std::optional<std::string>  format_profile;
    };

    struct StreamAudio
    {
        std::optional<std::string>  codec_id;
        std::optional<std::int64_t> duration;
        std::optional<std::string>  format;
    };

    struct StreamVideo
    {
        std::optional<float>        aspect_ratio_display;
        std::optional<float>        aspect_ratio_pixel;
        std::optional<std::int64_t> bitrate;
        std::optional<std::string>  chroma_subsampling;
        std::optional<std::string>  codec_id;
        std::optional<std::string>  color_space;
        std::optional<std::int64_t> duration;
        std::optional<std::string>  format;
        std::optional<std::string>  format_level;
        std::optional<std::string>  format_profile;
        std::optional<std::string>  format_tier;
        std::optional<float>        frame_rate;
        std::optional<std::string>  hdr_format;
        std::optional<std::string>  hdr_format_level;
        std::optional<std::string>  hdr_format_profile;
        std::optional<std::int64_t> height;
        std::optional<std::int64_t> width;
    };

    struct Container
    {
        StreamGeneral            general;
        std::vector<StreamAudio> audio_streams;
        std::vector<StreamVideo> video_streams;
    };

    class Parser
    {
    public:
        static std::optional<Container> Parse(const std::filesystem::path& path);
        static std::optional<Container> ParseExternal(const std::filesystem::path& path);
    };
}
