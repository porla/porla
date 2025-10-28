#pragma once

#include "../method.hpp"
#include "torrentsfilesprogress_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsFilesProgress : public Method<TorrentsFilesProgressReq, TorrentsFilesProgressRes>
    {
    public:
        explicit TorrentsFilesProgress(porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsFilesProgressReq& req, WriteCb<TorrentsFilesProgressRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
