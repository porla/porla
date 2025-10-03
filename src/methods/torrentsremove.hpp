#pragma once

#include "method.hpp"
#include "torrentsremovereq.hpp"
#include "torrentsremoveres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsRemove : public Method<TorrentsRemoveReq, TorrentsRemoveRes>
    {
    public:
        explicit TorrentsRemove(porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsRemoveReq& req, WriteCb<TorrentsRemoveRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
