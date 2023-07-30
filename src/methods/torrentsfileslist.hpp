#pragma once

#include "method.hpp"
#include "torrentsfileslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsFilesList : public Method<TorrentsFilesListReq, TorrentsFilesListRes>
    {
    public:
        explicit TorrentsFilesList(Sessions& sessions);

    protected:
        void Invoke(const TorrentsFilesListReq& req, WriteCb<TorrentsFilesListRes> cb) override;

    private:
        Sessions& m_sessions;
    };
}
