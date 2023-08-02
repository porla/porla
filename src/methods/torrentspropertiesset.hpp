#pragma once

#include "method.hpp"
#include "torrentspropertiesset_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsPropertiesSet : public Method<TorrentsPropertiesSetReq, TorrentsPropertiesSetRes>
    {
    public:
        explicit TorrentsPropertiesSet(Sessions& sessions);

    protected:
        void Invoke(const TorrentsPropertiesSetReq& req, WriteCb<TorrentsPropertiesSetRes> cb) override;

    private:
        Sessions& m_sessions;
    };
}
