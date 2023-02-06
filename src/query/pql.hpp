#pragma once

#include <functional>
#include <memory>
#include <string>

#include <libtorrent/torrent_status.hpp>

namespace porla::Query
{
    class PQL
    {
    public:
        struct Filter
        {
            virtual ~Filter() = default;
            virtual bool Includes(const libtorrent::torrent_status& ts) = 0;
        };

        static std::unique_ptr<Filter> Parse(const std::string_view& input);
    };
}
