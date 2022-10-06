#pragma once

#include "method.hpp"
#include "torrentsaddreq.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsAdd : public MethodT<TorrentsAddReq>
    {
    public:
        explicit TorrentsAdd(std::string const& path, ISession& session);

    protected:
        void Invoke(TorrentsAddReq const& req) override;

    private:
        ISession& m_session;
    };
}
