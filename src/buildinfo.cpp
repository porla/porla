#include "buildinfo.hpp"

using porla::BuildInfo;

extern "C"
{
    extern const char* GIT_BRANCH;
    extern const char* GIT_COMMITISH;
    extern const char* VERSION;
}

const char* BuildInfo::Branch()
{
    return GIT_BRANCH;
}

const char* BuildInfo::Commitish()
{
    return GIT_COMMITISH;
}

const char* BuildInfo::Version()
{
    return VERSION;
}
