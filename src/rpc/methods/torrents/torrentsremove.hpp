#pragma once

#include "../../typedmethod.hpp"
#include "torrentsremovereq.hpp"
#include "torrentsremoveres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsRemove : public TypedMethod<TorrentsRemoveReq, TorrentsRemoveRes>
    {
    public:
        explicit TorrentsRemove(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsRemoveReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
