#include "buildinfo.hpp"

#include <string>

using porla::BuildInfo;

extern "C"
{
    extern const char* GIT_BRANCH;
    extern const char* GIT_COMMITISH;
    extern const char* VERSION;
    extern const char* VERSION_MAJOR;
    extern const char* VERSION_MINOR;
    extern const char* VERSION_PATCH;
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

const int BuildInfo::VersionMajor()
{
    return atoi(VERSION_MAJOR);
}

const int BuildInfo::VersionMinor()
{
    return atoi(VERSION_MINOR);
}

const int BuildInfo::VersionPatch()
{
    return atoi(VERSION_PATCH);
}
