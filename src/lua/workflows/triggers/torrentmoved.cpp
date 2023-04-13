#include "torrentmoved.hpp"

#include <utility>

#include <boost/signals2.hpp>

#include "../workflowrunner.hpp"
#include "../../usertypes/torrent.hpp"
#include "../../../session.hpp"

using porla::Lua::UserTypes::Torrent;
using porla::Lua::Workflows::Triggers::TorrentMoved;
using porla::Lua::Workflows::Triggers::TorrentMovedOptions;
using porla::Lua::Workflows::WorkflowRunner;
using porla::Lua::Workflows::WorkflowRunnerOptions;

class TorrentMoved::State
{
public:
    explicit State(TorrentMovedOptions options)
        : m_opts(std::move(options))
    {
        m_on_torrent_moved = m_opts.session.OnStorageMoved([&](const lt::torrent_handle& th) { OnTorrentMoved(th); });
    }

    ~State()
    {
        m_on_torrent_moved.disconnect();
    }

private:
    void OnTorrentMoved(const lt::torrent_handle& th)
    {
        sol::table ctx           = m_opts.lua.create_table();
        ctx["actions"]           = std::vector<sol::object>();
        ctx["lt:torrent_handle"] = th;
        ctx["torrent"]           = Torrent{th};

        const WorkflowRunnerOptions runner_opts{
            .io      = m_opts.io,
            .lua     = m_opts.lua,
            .session = m_opts.session,
        };

        auto workflow = std::make_shared<WorkflowRunner>(runner_opts, ctx, m_opts.actions);
        boost::asio::post(m_opts.io, [workflow]() { workflow->Run(); });
    }

    TorrentMovedOptions m_opts;
    boost::signals2::connection m_on_torrent_moved;
};

TorrentMoved::TorrentMoved(const TorrentMovedOptions& opts)
    : m_state(std::make_unique<State>(opts))
{
}

TorrentMoved::~TorrentMoved() = default;
