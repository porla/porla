#pragma once

#include <libtorrent/torrent_status.hpp>

namespace porla
{
    struct TorrentClientData;

    // maps torrent status properties to fields that can be used in sorting and querying
    struct Fields
    {
        struct Context
        {
            const lt::torrent_status& status;
            const TorrentClientData*  client_data;
            const std::time_t         now;
        };

        using NumberGetter = std::optional<double>           (*)(const Context&);
        using TextGetter   = std::optional<std::string_view> (*)(const Context&);
        using BoolGetter   = std::optional<bool>             (*)(const Context&);

        enum class Kind
        {
            Text,
            Tag,
            Hash,
            Size,
            Rate,
            Duration,
            Number,
            Percent,
            Date,
            State,
            Bool,
            Flags
        };

        struct Field
        {
            std::string_view name;
            Kind             kind;
            NumberGetter     number  = nullptr;
            TextGetter       text    = nullptr;
            BoolGetter       boolean = nullptr;
        };

        static const std::vector<Field>& All();

        static const Field* Find(std::string_view name);
    };
}
