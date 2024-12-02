#pragma once

namespace porla
{
    class BuildInfo
    {
    public:
        static const char* Branch();
        static const char* Commitish();
        static const char* Version();
        static const int VersionMajor();
        static const int VersionMinor();
        static const int VersionPatch();
    };
}
