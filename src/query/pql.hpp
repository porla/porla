#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <string>

#include <libtorrent/torrent_status.hpp>
#include <utility>

namespace porla::Query
{
    class QueryError : public std::runtime_error
    {
    public:
        explicit QueryError(std::string desc, size_t pos = -1)
            : std::runtime_error(desc)
            , m_desc(std::move(desc))
            , m_pos(pos)
        {
        }

        [[nodiscard]] size_t pos() const { return m_pos; }

    private:
        std::string m_desc;
        size_t m_pos;
    };

    class PQL
    {
    public:
        struct Filter
        {
            virtual ~Filter() = default;
            [[nodiscard]] virtual bool Includes(const libtorrent::torrent_status& ts) const = 0;
        };

        static std::unique_ptr<Filter> Parse(const std::string_view& input);
    };
}
