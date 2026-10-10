#pragma once

#include "../../typedmethod.hpp"
#include "torrentsfilesprioritize_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsFilesPrioritize : public TypedMethod<TorrentsFilesPrioritizeReq, TorrentsFilesPrioritizeRes>
    {
    public:
        explicit TorrentsFilesPrioritize(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsFilesPrioritizeReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
