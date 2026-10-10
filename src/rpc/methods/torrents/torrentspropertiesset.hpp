#pragma once

#include "../../typedmethod.hpp"
#include "torrentspropertiesset_reqres.hpp"

namespace porla
{
    class Sessions;
    class Torrents;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPropertiesSet : public TypedMethod<TorrentsPropertiesSetReq, TorrentsPropertiesSetRes>
    {
    public:
        explicit TorrentsPropertiesSet(porla::Sessions& sessions, porla::Torrents& torrents);

    protected:
        void Execute(const TorrentsPropertiesSetReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
        porla::Torrents& m_torrents;
    };
}
