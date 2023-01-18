#include "fsspace.hpp"

#include <filesystem>
#include <optional>
#include <regex>

#include <boost/log/trivial.hpp>
#include <sys/quota.h>

namespace fs = std::filesystem;

using porla::Methods::FsSpace;
using porla::Methods::FsSpaceReq;
using porla::Methods::FsSpaceRes;
using porla::Methods::FsSpaceQuota;

FsSpace::FsSpace() = default;

std::optional<std::string> GetBlockDeviceFromPath(const std::string& path)
{
    static const auto mountpoint_re = std::regex(R"(^([\d]+)\s([\d]+)\s([\d]+:[\d]+)\s([a-zA-Z\d\/:\[\]]+)\s([a-zA-Z\d\/\.-]+)\s([a-zA-Z\d\,]+)\s([a-zA-Z\d\/:\s]+)\s-\s([a-zA-Z0-9\.]+)\s([a-zA-Z\d\/_]+)\s([a-zA-Z\d\/:\s=,_]+)$)");

    struct stat fs{};
    if (stat(path.c_str(), &fs) < 0)
    {
        BOOST_LOG_TRIVIAL(error) << "No such file or directory: " << path;
        return std::nullopt;
    }

    std::stringstream minmaj_stream;
    minmaj_stream << static_cast<int>(fs.st_dev >> 8);
    minmaj_stream << ":";
    minmaj_stream << static_cast<int>(fs.st_dev & 0xff);
    std::string minmaj = minmaj_stream.str();

    std::ifstream mountinfo("/proc/self/mountinfo");
    std::string line;

    while (std::getline(mountinfo, line))
    {
        std::smatch matches;

        if (std::regex_search(line, matches, mountpoint_re))
        {
            if (matches[3] == minmaj)
            {
                return matches[9];
            }
        }
        else
        {
            BOOST_LOG_TRIVIAL(debug) << "Mount point line did not match: " << line;
        }
    }

    return std::nullopt;
}

std::optional<FsSpaceQuota> GetQuota(const std::string& path)
{
    if (auto block_device = GetBlockDeviceFromPath(path))
    {
        BOOST_LOG_TRIVIAL(info) << "Found block device: " << *block_device;

        const int cmd = QCMD(Q_GETQUOTA, USRQUOTA);
        dqblk blk = {};

        if (quotactl(cmd, block_device->c_str(), static_cast<int>(getuid()), reinterpret_cast<char*>(&blk)) < 0)
        {
            BOOST_LOG_TRIVIAL(error) << "quotactl error: " << errno;
            return std::nullopt;
        }

        return FsSpaceQuota{
            .blocks_limit_hard = blk.dqb_bhardlimit,
            .blocks_limit_soft = blk.dqb_bsoftlimit,
            .blocks_time       = blk.dqb_btime,
            .current_inodes    = blk.dqb_curinodes,
            .current_space     = blk.dqb_curspace,
            .inodes_limit_hard = blk.dqb_ihardlimit,
            .inodes_limit_soft = blk.dqb_isoftlimit,
            .inodes_time       = blk.dqb_itime
        };
    }

    return std::nullopt;
}

void FsSpace::Invoke(const FsSpaceReq& req, WriteCb<FsSpaceRes> cb)
{
    std::error_code ec;
    const auto space_info = fs::space(req.path, ec);

    if (ec)
    {
        return cb.Error(-1, ec.message());
    }

    cb.Ok(FsSpaceRes{
        .available = space_info.available,
        .capacity  = space_info.capacity,
        .free      = space_info.free,
        .quota     = GetQuota(req.path)
    });
}
