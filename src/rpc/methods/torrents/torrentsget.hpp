#pragma once

#include "../../typedmethod.hpp"
#include "torrentsget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsGet : public TypedMethod<TorrentsGetReq, TorrentsGetRes>
    {
    public:
        explicit TorrentsGet(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsGetReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
