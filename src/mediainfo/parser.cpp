#include "parser.hpp"

#include <boost/log/trivial.hpp>
#include <MediaInfo/MediaInfo.h>
#include <ZenLib/Ztring.h>

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
