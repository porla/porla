#pragma once

#include "method.hpp"
#include "torrentsqueryres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsQuery : public Method<TorrentsQueryRes>
    {
    public:
        explicit TorrentsQuery(std::string const& path, ISession& session);

        void Invoke(WriteCb<TorrentsQueryRes> cb) override;

    private:
        ISession& m_session;
    };
}
