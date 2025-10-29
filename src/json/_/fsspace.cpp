#include "../all.hpp"

#include "../../methods/fsspace_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        FsSpaceReq,
        path)

    NLOHMANN_JSONIFY_ALL_THINGS(
        FsSpaceQuota,
        blocks_limit_hard,
        blocks_limit_soft,
        blocks_time,
        current_inodes,
        current_space,
        inodes_limit_hard,
        inodes_limit_soft,
        inodes_time)

    NLOHMANN_JSONIFY_ALL_THINGS(
        FsSpaceRes,
        available,
        capacity,
        free,
        quota)
}
