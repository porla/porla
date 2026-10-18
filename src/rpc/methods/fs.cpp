#include "fs.hpp"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <optional>

#include <boost/asio/awaitable.hpp>
#include <boost/log/trivial.hpp>
#include <fcntl.h>
#include <sys/quota.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../json/utils.hpp"

namespace fs = std::filesystem;

namespace porla::Rpc::Methods::Fs
{
    namespace
    {
        struct SpaceReq
        {
            std::string path;
        };

        struct Quota
        {
            std::uint64_t blocks_limit_hard;
            std::uint64_t blocks_limit_soft;
            std::uint64_t blocks_time;
            std::uint64_t current_inodes;
            std::uint64_t current_space;
            std::uint64_t inodes_limit_hard;
            std::uint64_t inodes_limit_soft;
            std::uint64_t inodes_time;
        };

        struct SpaceRes
        {
            std::uint64_t        available{0};
            std::uint64_t        capacity{0};
            std::uint64_t        free{0};
            std::optional<Quota> quota{std::nullopt};
        };

        PORLA_JSON_REQUEST(SpaceReq, path)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            Quota,
            blocks_limit_hard,
            blocks_limit_soft,
            blocks_time,
            current_inodes,
            current_space,
            inodes_limit_hard,
            inodes_limit_soft,
            inodes_time)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(SpaceRes, available, capacity, free, quota)

        std::optional<Quota> GetQuota(const std::string& path)
        {
        #if defined(__linux__) && defined(SYS_quotactl_fd)
            const int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);

            if (fd < 0)
            {
                BOOST_LOG_TRIVIAL(debug) << "Could not open " << path << " for quota lookup: " << std::strerror(errno);
                return std::nullopt;
            }

            dqblk blk = {};

            const long ret = syscall(SYS_quotactl_fd, fd, QCMD(Q_GETQUOTA, USRQUOTA), static_cast<int>(getuid()), &blk);
            const int  err = errno;

            close(fd);

            if (ret < 0)
            {
                switch (err)
                {
                case ENOSYS:
                case EPERM:
                case ESRCH:
                case ENOTSUP:
        #if EOPNOTSUPP != ENOTSUP
                case EOPNOTSUPP:
        #endif
                case ENOENT:
                    BOOST_LOG_TRIVIAL(debug) << "No quota information for " << path << ": " << std::strerror(err);
                    break;
                default:
                    BOOST_LOG_TRIVIAL(warning) << "quotactl_fd failed for " << path << ": " << std::strerror(err);
                    break;
                }

                return std::nullopt;
            }

            // Only report a quota when the kernel filled in the block limits and usage.
            if ((blk.dqb_valid & (QIF_BLIMITS | QIF_SPACE)) != (QIF_BLIMITS | QIF_SPACE))
            {
                return std::nullopt;
            }

            return Quota{
                .blocks_limit_hard = blk.dqb_bhardlimit,
                .blocks_limit_soft = blk.dqb_bsoftlimit,
                .blocks_time       = blk.dqb_btime,
                .current_inodes    = blk.dqb_curinodes,
                .current_space     = blk.dqb_curspace,
                .inodes_limit_hard = blk.dqb_ihardlimit,
                .inodes_limit_soft = blk.dqb_isoftlimit,
                .inodes_time       = blk.dqb_itime
            };
        #else
            return std::nullopt;
        #endif
        }

        boost::asio::awaitable<SpaceRes> Space(const SpaceReq& req)
        {
            std::error_code ec;
            const auto space_info = fs::space(req.path, ec);

            if (ec)
            {
                throw Error(-1, ec.message());
            }

            co_return SpaceRes{
                .available = space_info.available,
                .capacity  = space_info.capacity,
                .free      = space_info.free,
                .quota     = GetQuota(req.path)
            };
        }
    }

    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc)
    {
        using Kind = Auth::Context::Kind;

        jsonrpc.Register("fs.space", { Kind::Machine, Kind::User }, MakeMethod<SpaceReq>(io.get_executor(), Space));
    }
}
