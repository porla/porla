#pragma once

#include "../../typedmethod.hpp"

#include "torrentsaddreq.hpp"
#include "torrentsaddres.hpp"

namespace porla
{
    class Torrents;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsAdd : public TypedMethod<TorrentsAddReq, TorrentsAddRes>
    {
    public:
        explicit TorrentsAdd(porla::Torrents& torrents);

    protected:
        void Execute(const TorrentsAddReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Torrents& m_torrents;
    };
}
