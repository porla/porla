#pragma once

#include "method.hpp"
#include "torrentsfileslist_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsFilesList : public Method<TorrentsFilesListReq, TorrentsFilesListRes>
    {
    public:
        explicit TorrentsFilesList(ISession& session);

    protected:
        void Invoke(const TorrentsFilesListReq& req, WriteCb<TorrentsFilesListRes> cb);

    private:
        ISession& m_session;
    };
}
