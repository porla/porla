#pragma once

#include "method.hpp"
#include "torrentsaddreq.hpp"
#include "torrentsaddres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsAdd : public MethodT<TorrentsAddReq, TorrentsAddRes>
    {
    public:
        explicit TorrentsAdd(std::string const& path, ISession& session);

    protected:
        void Invoke(TorrentsAddReq const& req, WriteCb<TorrentsAddRes> cb) override;

    private:
        ISession& m_session;
    };
}
