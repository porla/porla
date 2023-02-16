#pragma once

#include <boost/asio.hpp>
#include <libtorrent/torrent_status.hpp>

#include <filesystem>
#include <memory>
#include <vector>

namespace porla
{
    class ISession;
}

namespace porla::Lua::Workflows
{
    struct WorkflowEngineOptions
    {
        boost::asio::io_context& io;
        porla::ISession&         session;
        std::filesystem::path    workflow_dir;
    };

    class WorkflowEngine
    {
    public:
        explicit WorkflowEngine(const WorkflowEngineOptions& options);
        ~WorkflowEngine();

    private:
        void OnTorrentAdded(const libtorrent::torrent_status& ts);

        struct State;

        WorkflowEngineOptions m_opts;
        std::unique_ptr<State> m_state;
    };
}
