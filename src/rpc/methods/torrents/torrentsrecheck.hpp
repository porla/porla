#pragma once

#include "../../typedmethod.hpp"
#include "torrentsrecheck_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsRecheck : public TypedMethod<TorrentsRecheckReq, TorrentsRecheckRes>
    {
    public:
        explicit TorrentsRecheck(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsRecheckReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
