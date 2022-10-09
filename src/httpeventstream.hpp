#pragma once

#include <memory>

#include <boost/signals2.hpp>
#include <libtorrent/torrent_status.hpp>

#include "httpcontext.hpp"

namespace porla
{
    class ISession;

    class HttpEventStream
    {
    public:
        explicit HttpEventStream(ISession& session);
        HttpEventStream(const HttpEventStream&);

        ~HttpEventStream();

        void operator()(std::shared_ptr<HttpContext>);

    private:
        class ContextState;

        void Broadcast(const std::string& name, const std::string& data);
        void OnStateUpdate(const std::vector<libtorrent::torrent_status>& torrents);

        ISession& m_session;
        std::vector<std::shared_ptr<ContextState>> m_ctxs;

        boost::signals2::connection m_stateUpdateConnection;
    };
}
