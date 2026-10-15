#pragma once

#include "../../typedmethod.hpp"

#include "torrentref.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsTrackersList : public TypedMethod<TorrentRef>
    {
    public:
        explicit TorrentsTrackersList(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentRef& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
