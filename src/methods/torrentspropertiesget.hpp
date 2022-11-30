#pragma once

#include "method.hpp"
#include "torrentspropertiesget_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsPropertiesGet : public Method<TorrentsPropertiesGetReq, TorrentsPropertiesGetRes>
    {
    public:
        explicit TorrentsPropertiesGet(ISession& session);

    protected:
        void Invoke(const TorrentsPropertiesGetReq& req, WriteCb<TorrentsPropertiesGetRes> cb) override;

    private:
        ISession& m_session;
    };
}
