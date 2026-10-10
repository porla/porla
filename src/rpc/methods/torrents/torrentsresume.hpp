#pragma once

#include "../../typedmethod.hpp"
#include "torrentsresume_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsResume : public TypedMethod<TorrentsResumeReq, TorrentsResumeRes>
    {
    public:
        explicit TorrentsResume(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsResumeReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
