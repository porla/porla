#pragma once

#include "../../typedmethod.hpp"
#include "torrentsqueueset_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsQueueSet : public TypedMethod<TorrentsQueueSetReq, TorrentsQueueSetRes>
    {
    public:
        explicit TorrentsQueueSet(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsQueueSetReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
