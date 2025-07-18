#pragma once

#include "method.hpp"
#include "torrentspropertiesget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsPropertiesGet : public Method<TorrentsPropertiesGetReq, TorrentsPropertiesGetRes, SSL>
    {
    public:
        explicit TorrentsPropertiesGet(Sessions& sessions);

    protected:
        void Invoke(const TorrentsPropertiesGetReq& req, WriteCb<TorrentsPropertiesGetRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
