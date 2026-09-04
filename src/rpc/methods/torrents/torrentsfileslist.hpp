#pragma once

#include <sqlite3.h>

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
        explicit TorrentsFilesList(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsFilesListReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
