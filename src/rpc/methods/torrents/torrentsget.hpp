#pragma once

#include "../../typedmethod.hpp"
#include "torrentref.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsGet : public TypedMethod<TorrentRef>
    {
    public:
        explicit TorrentsGet(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentRef& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
