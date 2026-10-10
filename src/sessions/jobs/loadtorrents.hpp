#pragma once

#include <functional>

#include <sqlite3.h>

#include "job.hpp"
#include "../../data/models/addtorrentparams.hpp"

namespace porla::Jobs
{
    class LoadTorrents : public Job
    {
    public:
        struct Output
        {
            int  loaded;
            bool failed;
            bool stopped;
        };

        LoadTorrents(sqlite3* db, int count, std::function<void(const Output&)> callback);

        std::string_view Name() const override { return "load-torrents"; }

        Result Run(Session& session) override;
        void Stopped(Session& session) override;

    private:
        void Complete(const Session& session, bool stopped);

        sqlite3*                               m_db;
        Data::Models::AddTorrentParams::Cursor m_cursor;

        int m_count;
        int m_loaded  = 0;
        int m_chunks  = 0;
        int m_errors  = 0;
        bool m_failed = false;

        std::function<void(const Output&)> m_callback;
    };
}
