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

namespace porla::Lua
{
    struct EngineOptions
    {
        boost::asio::io_context& io;
        porla::ISession&         session;
        std::filesystem::path    workflow_dir;
    };

    class Engine
    {
    public:
        explicit Engine(const EngineOptions& options);
        ~Engine();

    private:
        void OnTorrentAdded(const libtorrent::torrent_status& ts);

        struct State;
        struct WorkflowInstance;

        EngineOptions m_opts;
        std::unique_ptr<State> m_state;
        std::vector<WorkflowInstance> m_workflow_instances;
    };
}
