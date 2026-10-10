#pragma once

#include "../../typedmethod.hpp"
#include "torrentspropertiesget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPropertiesGet : public TypedMethod<TorrentsPropertiesGetReq, TorrentsPropertiesGetRes>
    {
    public:
        explicit TorrentsPropertiesGet(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsPropertiesGetReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
