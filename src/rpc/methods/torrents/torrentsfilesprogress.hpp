#pragma once

#include "../../typedmethod.hpp"
#include "torrentsfilesprogress_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsFilesProgress : public TypedMethod<TorrentsFilesProgressReq, TorrentsFilesProgressRes>
    {
    public:
        explicit TorrentsFilesProgress(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsFilesProgressReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
