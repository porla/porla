#pragma once

#include <functional>

#include "torrentref.hpp"

#include "../resolve.hpp"
#include "../../typedmethod.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentAction : public TypedMethod<TorrentRef>
    {
    public:
        using Fn = std::function<void(const ResolvedTorrent&)>;

        TorrentAction(porla::Sessions& sessions, Fn fn);

        void Execute(const TorrentRef&, ResponseWriterHandle) override; // ResolveTorrent → fn → Ok()

    private:
        porla::Sessions& m_sessions;
        Fn               m_fn;
    };
}
