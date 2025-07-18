#pragma once

#include "method.hpp"
#include "torrentspropertiesset_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsPropertiesSet : public Method<TorrentsPropertiesSetReq, TorrentsPropertiesSetRes, SSL>
    {
    public:
        explicit TorrentsPropertiesSet(Sessions& sessions);

    protected:
        void Invoke(const TorrentsPropertiesSetReq& req, WriteCb<TorrentsPropertiesSetRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
