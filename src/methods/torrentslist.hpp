#pragma once

#include "method.hpp"
#include "torrentslistres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsList : public Method<TorrentsListRes>
    {
    public:
        explicit TorrentsList(std::string const& path, ISession& session);

        void Invoke(WriteCb<TorrentsListRes> cb) override;

    private:
        ISession& m_session;
    };
}
