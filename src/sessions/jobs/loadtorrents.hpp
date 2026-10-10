#pragma once

#include <functional>

#include <sqlite3.h>

#include "job.hpp"
#include "../../data/models/addtorrentparams.hpp"

namespace porla
{
    class Events;
    class Scheduler;
}

namespace porla::Jobs
{
    class LoadTorrents : public Job
    {
    public:
        LoadTorrents(sqlite3* db, Events& events, Scheduler& scheduler, std::function<void()> callback);

        std::string_view Name() const override { return "load-torrents"; }

        Result Run(Session& session) override;
        void Stopped(Session& session) override;

    private:
        void Complete(const Session& session);
        void Finish(Session& session);

        sqlite3*                               m_db;
        Events&                                m_events;
        Scheduler&                             m_scheduler;
        Data::Models::AddTorrentParams::Cursor m_cursor;

        int m_count   = -1;
        int m_loaded  = 0;
        int m_chunks  = 0;
        int m_errors  = 0;
        bool m_failed = false;

        std::function<void()> m_callback;
    };
}
