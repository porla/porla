#include "parser.hpp"

#include <boost/log/trivial.hpp>
#include <boost/process.hpp>

#include <MediaInfo/MediaInfo.h>
#include <ZenLib/Ztring.h>

namespace bp = boost::process;
using porla::MediaInfo::Parser;

bool Parser::Parse(const std::filesystem::path& path)
{
    MediaInfoLib::MediaInfo mi;
    mi.Open(ZenLib::Ztring(path.c_str()).To_Unicode());

    // ZenLib::Ztring stream_general = mi.Option(__T("Info_Parameters"));
    // ZenLib::Ztring stream_general = mi.Get(MediaInfoLib::Stream_Video, 0, __T("Width"), MediaInfoLib::Info_Text, MediaInfoLib::Info_Name);
    ZenLib::Ztring stream_general = mi.Inform();

    BOOST_LOG_TRIVIAL(info) << stream_general.c_str();

    return false;
}

bool Parser::ParseExternal(const std::filesystem::path& path)
{
    char self[PATH_MAX];
    const ssize_t chars_read = readlink("/proc/self/exe", self, PATH_MAX);

    if (chars_read < 0)
    {
        return false;
    }

    std::stringstream cmd;
    cmd << std::string(self, chars_read) << " mediainfo:parse " << path;

    bp::ipstream err;
    bp::ipstream out;

    bp::child mediainfo_parser(cmd.str(), bp::std_out > out, bp::std_err > err);
    mediainfo_parser.wait_for(std::chrono::seconds(1));

    std::stringstream out_str;
    out_str << out.rdbuf();

    return true;
}
