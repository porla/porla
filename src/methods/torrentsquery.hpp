#pragma once

#include "method.hpp"
#include "torrentsqueryreq.hpp"
#include "torrentsqueryres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsQuery : public MethodT<TorrentsQueryReq, TorrentsQueryRes>
    {
    public:
        explicit TorrentsQuery(std::string const& path, ISession& session);

        void Invoke(const TorrentsQueryReq& req, WriteCb<TorrentsQueryRes> cb) override;

    private:
        ISession& m_session;
    };
}
