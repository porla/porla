#pragma once

namespace porla
{
    class BuildInfo
    {
    public:
        static const char* Branch();
        static const char* Commitish();
        static const char* Version();
    };
}
