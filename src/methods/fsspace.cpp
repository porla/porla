#include "fsspace.hpp"

#include <filesystem>

namespace fs = std::filesystem;

using porla::Methods::FsSpace;
using porla::Methods::FsSpaceReq;
using porla::Methods::FsSpaceRes;

FsSpace::FsSpace() = default;

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
        .free      = space_info.free
    });
}
