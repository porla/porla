#pragma once

#include <functional>

#include "job.hpp"
#include "../../data/models/addtorrentparams.hpp"

namespace porla::Jobs
{
    class LoadTorrents : public Job
    {
    public:
        LoadTorrents(int count, std::function<void()> callback);

        std::string_view Name() const override { return "load-torrents"; }

        Result Run(Session& session) override;
        void Stopped(Session& session) override;

    private:
        void Complete(const Session& session);

        Data::Models::AddTorrentParams::Cursor m_cursor;

        int m_count;
        int m_loaded  = 0;
        int m_chunks  = 0;
        int m_errors  = 0;
        bool m_failed = false;

        std::function<void()> m_callback;
    };
}
