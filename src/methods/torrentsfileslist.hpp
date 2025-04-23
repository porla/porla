#pragma once

#include "method.hpp"
#include "torrentsfileslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsFilesList : public Method<TorrentsFilesListReq, TorrentsFilesListRes, SSL>
    {
    public:
        explicit TorrentsFilesList(Sessions& sessions);

    protected:
        void Invoke(const TorrentsFilesListReq& req, WriteCb<TorrentsFilesListRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
