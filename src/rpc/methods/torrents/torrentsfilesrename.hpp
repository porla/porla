#pragma once

#include <sqlite3.h>

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
        explicit TorrentsFilesRename(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsFilesRenameReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
