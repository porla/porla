#pragma once

#include "method.hpp"
#include "torrentspropertiesget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsPropertiesGet : public Method<TorrentsPropertiesGetReq, TorrentsPropertiesGetRes>
    {
    public:
        explicit TorrentsPropertiesGet(porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsPropertiesGetReq& req, WriteCb<TorrentsPropertiesGetRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
