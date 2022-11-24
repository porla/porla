#include "versionjson.hpp"

#include "../buildinfo.hpp"

int porla::Tools::VersionJson(int argc, char **argv, std::unique_ptr<porla::Config> cfg)
{
    printf("{\"branch\": \"%s\",\"commitish\": \"%s\", \"version\": \"%s\"}\n",
           porla::BuildInfo::Branch(),
           porla::BuildInfo::Commitish(),
           porla::BuildInfo::Version());

    return 0;
}
