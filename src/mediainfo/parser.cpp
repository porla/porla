#include "parser.hpp"

#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/process.hpp>

#include <MediaInfo/MediaInfo.h>
#include <nlohmann/json.hpp>
#include <ZenLib/Ztring.h>

namespace bp = boost::process;
namespace fs = std::filesystem;
using json = nlohmann::json;
using porla::MediaInfo::Parser;

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <sys/param.h>
static fs::path GetExecutablePath()
{
    char buf[MAXPATHLEN];
    uint32_t copied = MAXPATHLEN;
    _NSGetExecutablePath(buf, &copied);
    return buf;
}
#endif

bool Parser::Parse(const std::filesystem::path& path)
{
    MediaInfoLib::MediaInfo mi;
    mi.Open(ZenLib::Ztring(path.c_str()).To_Unicode());

    // ZenLib::Ztring stream_general = mi.Option(__T("Info_Parameters"));
    // ZenLib::Ztring stream_general = mi.Get(MediaInfoLib::Stream_Video, 0, __T("Width"), MediaInfoLib::Info_Text, MediaInfoLib::Info_Name);
    ZenLib::Ztring stream_general = mi.Inform();

    json j{
       {"general", {
           {"complete_name", mi.Get(MediaInfoLib::Stream_General, 0, __T("CompleteName")).c_str()},
           {"format", mi.Get(MediaInfoLib::Stream_General, 0, __T("Format")).c_str()},
           {"format_profile", mi.Get(MediaInfoLib::Stream_General, 0, __T("Format_Profile")).c_str()},
           {"codec_id", mi.Get(MediaInfoLib::Stream_General, 0, __T("CodecID")).c_str()},
           {"duration", std::stoi(mi.Get(MediaInfoLib::Stream_General, 0, __T("Duration")).c_str())},
           {"overall_bitrate", std::stoi(mi.Get(MediaInfoLib::Stream_General, 0, __T("OverallBitRate")).c_str())},
       }}
    };

    for (int i = 0; i < mi.Count_Get(MediaInfoLib::Stream_Video); i++)
    {
        json video{
            {"codec_id", mi.Get(MediaInfoLib::Stream_Video, i, __T("CodecID")).c_str()},
            {"format", mi.Get(MediaInfoLib::Stream_Video, i, __T("Format")).c_str()},
            {"format_info", mi.Get(MediaInfoLib::Stream_Video, i, __T("Format/Info")).c_str()},
            {"format_profile", mi.Get(MediaInfoLib::Stream_Video, i, __T("Format_Profile")).c_str()},
            {"height", std::stoi(mi.Get(MediaInfoLib::Stream_Video, i, __T("Height")).c_str())},
            {"width", std::stoi(mi.Get(MediaInfoLib::Stream_Video, i, __T("Width")).c_str())},
        };

        j["video"].push_back(video);
    }

    for (int i = 0; i < mi.Count_Get(MediaInfoLib::Stream_Audio); i++)
    {
        json audio{
            {"codec_id", mi.Get(MediaInfoLib::Stream_Audio, i, __T("CodecID")).c_str()},
            {"format", mi.Get(MediaInfoLib::Stream_Audio, i, __T("Format")).c_str()},
        };

        j["audio"].push_back(audio);
    }

    std::cout << j.dump() << "\n";

    return true;
}

bool Parser::ParseExternal(const std::filesystem::path& path)
{
    /*char self[PATH_MAX];
    const ssize_t chars_read = readlink("/proc/self/exe", self, PATH_MAX);

    if (chars_read < 0)
    {
        return false;
    }*/

    std::stringstream cmd;
    cmd << GetExecutablePath() << " mediainfo:parse " << path;

    bp::ipstream err;
    bp::ipstream out;

    bp::child mediainfo_parser(cmd.str(), bp::std_out > out, bp::std_err > err);
    mediainfo_parser.wait_for(std::chrono::seconds(1));

    std::stringstream out_str;
    out_str << out.rdbuf();

    BOOST_LOG_TRIVIAL(info) << mediainfo_parser.exit_code();

    return true;
}
