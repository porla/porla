#pragma once

#include "../../typedmethod.hpp"
#include "torrentscount_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsCount : public TypedMethod<TorrentsCountReq, TorrentsCountRes>
    {
    public:
        explicit TorrentsCount(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsCountReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
