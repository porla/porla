#pragma once

#include <boost/asio/io_context.hpp>

#include "../../typedmethod.hpp"
#include "torrentsmigrate_reqres.hpp"

namespace porla
{
    class Events;
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsMigrate : public TypedMethod<TorrentsMigrateReq, TorrentsMigrateRes>
    {
    public:
        explicit TorrentsMigrate(boost::asio::io_context& io, porla::Events& events, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsMigrateReq& req, ResponseWriterHandle cb) override;

    private:
        boost::asio::io_context& m_io;
        porla::Events& m_events;
        porla::Sessions& m_sessions;
    };
}
