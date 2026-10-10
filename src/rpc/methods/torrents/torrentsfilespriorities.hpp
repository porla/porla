#pragma once

#include "../../typedmethod.hpp"
#include "torrentsfilespriorities_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsFilesPriorities : public TypedMethod<TorrentsFilesPrioritiesReq, TorrentsFilesPrioritiesRes>
    {
    public:
        explicit TorrentsFilesPriorities(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsFilesPrioritiesReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
