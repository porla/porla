#pragma once

#include "../../typedmethod.hpp"
#include "torrentsfilesrename_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsFilesRename : public TypedMethod<TorrentsFilesRenameReq, TorrentsFilesRenameRes>
    {
    public:
        explicit TorrentsFilesRename(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsFilesRenameReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
