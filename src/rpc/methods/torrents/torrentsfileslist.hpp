#pragma once

#include "../../typedmethod.hpp"
#include "torrentsfileslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsFilesList : public TypedMethod<TorrentsFilesListReq, TorrentsFilesListRes>
    {
    public:
        explicit TorrentsFilesList(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsFilesListReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
