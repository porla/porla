#include "parser.hpp"

#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/process.hpp>
#include <MediaInfo/MediaInfo.h>
#include <nlohmann/json.hpp>
#include <ZenLib/Ztring.h>

#include "../json/mediainfo.hpp"

namespace bp = boost::process;
namespace fs = std::filesystem;
using json = nlohmann::json;
using porla::MediaInfo::Parser;

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <sys/param.h>
static fs::path GetExecutablePath()
{
    char buf[MAXPATHLEN];
    uint32_t copied = MAXPATHLEN;
    _NSGetExecutablePath(buf, &copied);
    return buf;
}
#elif defined(__linux__)
static fs::path GetExecutablePath()
{
    char self[PATH_MAX];
    const ssize_t chars_read = readlink("/proc/self/exe", self, PATH_MAX);

    if (chars_read < 0)
    {
        // TODO: Add error handling
        return {};
    }

    return self;
}
#endif

#define TO_S(input) ZenLib::Ztring(input).To_Local()
#define TO_I(input) std::stoi(input)
#define TO_F(input) std::stof(input)

std::optional<porla::MediaInfo::Container> Parser::Parse(const std::filesystem::path& path)
{
    MediaInfoLib::MediaInfo mi;
    if (mi.Open(ZenLib::Ztring(path.c_str()).To_Unicode()) == 0)
    {
        return std::nullopt;
    }

    Container c{
        .general = {
            .bitrate_overall = TO_I(mi.Get(MediaInfoLib::Stream_General, 0, __T("OverallBitRate"))),
            .codec_id        = TO_S(mi.Get(MediaInfoLib::Stream_General, 0, __T("CodecID"))),
            .duration        = TO_I(mi.Get(MediaInfoLib::Stream_General, 0, __T("Duration"))),
            .format          = TO_S(mi.Get(MediaInfoLib::Stream_General, 0, __T("Format"))),
            .format_profile  = TO_S(mi.Get(MediaInfoLib::Stream_General, 0, __T("Format_Profile")))
        }
    };

    for (int i = 0; i < mi.Count_Get(MediaInfoLib::Stream_Video); i++)
    {
        c.video_streams.emplace_back(StreamVideo{
            .aspect_ratio_display = TO_F(mi.Get(MediaInfoLib::Stream_Video, i, __T("DisplayAspectRatio"))),
            .aspect_ratio_pixel   = TO_F(mi.Get(MediaInfoLib::Stream_Video, i, __T("PixelAspectRatio"))),
            .bitrate              = TO_I(mi.Get(MediaInfoLib::Stream_Video, i, __T("BitRate"))),
            .chroma_subsampling   = TO_S(mi.Get(MediaInfoLib::Stream_Video, i, __T("ChromaSubsampling"))),
            .codec_id             = TO_S(mi.Get(MediaInfoLib::Stream_Video, i, __T("CodecID"))),
            .color_space          = TO_S(mi.Get(MediaInfoLib::Stream_Video, i, __T("ColorSpace"))),
            .duration             = TO_I(mi.Get(MediaInfoLib::Stream_Video, i, __T("Duration"))),
            .format               = TO_S(mi.Get(MediaInfoLib::Stream_Video, i, __T("Format"))),
            .format_level         = TO_S(mi.Get(MediaInfoLib::Stream_Video, i, __T("Format_Level"))),
            .format_profile       = TO_S(mi.Get(MediaInfoLib::Stream_Video, i, __T("Format_Profile"))),
            .format_tier          = TO_S(mi.Get(MediaInfoLib::Stream_Video, i, __T("Format_Tier"))),
            .frame_rate           = TO_F(mi.Get(MediaInfoLib::Stream_Video, i, __T("FrameRate"))),
            .hdr_format           = TO_S(mi.Get(MediaInfoLib::Stream_Video, i, __T("HDR_Format"))),
            .hdr_format_level     = TO_S(mi.Get(MediaInfoLib::Stream_Video, i, __T("HDR_Format_Level"))),
            .hdr_format_profile   = TO_S(mi.Get(MediaInfoLib::Stream_Video, i, __T("HDR_Format_Profile"))),
            .height               = TO_I(mi.Get(MediaInfoLib::Stream_Video, i, __T("Height"))),
            .width                = TO_I(mi.Get(MediaInfoLib::Stream_Video, i, __T("Width")))
        });
    }

    for (int i = 0; i < mi.Count_Get(MediaInfoLib::Stream_Audio); i++)
    {
        c.audio_streams.emplace_back(StreamAudio{
            .codec_id = TO_S(mi.Get(MediaInfoLib::Stream_Audio, i, __T("CodecID"))),
            .duration = TO_I(mi.Get(MediaInfoLib::Stream_Audio, i, __T("Duration"))),
            .format   = TO_S(mi.Get(MediaInfoLib::Stream_Audio, i, __T("Format")))
        });
    }

    return std::move(c);
}

std::optional<porla::MediaInfo::Container> Parser::ParseExternal(const std::filesystem::path& path)
{
    std::stringstream cmd;
    cmd << GetExecutablePath() << " mediainfo:parse " << path;

    bp::ipstream err;
    bp::ipstream out;

    bp::child mediainfo_parser(cmd.str(), bp::std_out > out, bp::std_err > err);
    mediainfo_parser.wait_for(std::chrono::seconds(1));

    if (mediainfo_parser.exit_code() != 0)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to run MediaInfo parser - exit code " << mediainfo_parser.exit_code();
        return std::nullopt;
    }

    std::stringstream out_str;
    out_str << out.rdbuf();

    BOOST_LOG_TRIVIAL(info) << mediainfo_parser.exit_code();

    return json::parse(out_str.str());
}
