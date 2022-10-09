#pragma once

#include "method.hpp"
#include "torrentsremovereq.hpp"
#include "torrentsremoveres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsRemove : public MethodT<TorrentsRemoveReq, TorrentsRemoveRes>
    {
    public:
        explicit TorrentsRemove(std::string const& path, ISession& session);

    protected:
        void Invoke(TorrentsRemoveReq const& req, WriteCb<TorrentsRemoveRes> cb) override;

    private:
        ISession& m_session;
    };
}
