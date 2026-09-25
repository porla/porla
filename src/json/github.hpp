#pragma once

#include "utils.hpp"

#include <vector>

namespace porla::Json
{
    struct GitHubReleaseAsset
    {
        std::string browser_download_url;
    };

    struct GitHubRelease
    {
        std::string                     tag_name;
        std::vector<GitHubReleaseAsset> assets;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        GitHubReleaseAsset,
        browser_download_url)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        GitHubRelease,
        tag_name,
        assets)

}
