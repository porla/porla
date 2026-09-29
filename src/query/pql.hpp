#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <string>

#include <libtorrent/torrent_status.hpp>
#include <utility>

namespace porla
{
    struct TorrentClientData;
}

namespace porla::Query
{
    struct QueryContext
    {
        const lt::torrent_status&       status;
        const porla::TorrentClientData* client_data;
        std::time_t                     now;
    };

    using Filter = std::function<bool(const QueryContext&)>;

    class QueryError : public std::runtime_error
    {
    public:
        explicit QueryError(std::string desc, size_t start, size_t end)
            : std::runtime_error(desc)
            , m_desc(std::move(desc))
            , m_start(start)
            , m_end(end)
        {
        }

        [[nodiscard]] size_t start() const { return m_start; }
        [[nodiscard]] size_t end()   const { return m_end; }

    private:
        std::string m_desc;
        size_t      m_start;
        size_t      m_end;
    };

    class PQL
    {
    public:
        static Filter Parse(std::string_view input);
    };
}
