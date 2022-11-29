#pragma once

#include "method.hpp"
#include "torrentspropertiesset_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsPropertiesSet : public Method<TorrentsPropertiesSetReq, TorrentsPropertiesSetRes>
    {
    public:
        explicit TorrentsPropertiesSet(ISession& session);

    protected:
        void Invoke(const TorrentsPropertiesSetReq& req, WriteCb<TorrentsPropertiesSetRes> cb) override;

    private:
        ISession& m_session;
    };
}
