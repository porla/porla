#include "torrentfinished.hpp"

#include <utility>

#include <boost/signals2.hpp>

#include "../workflowfilter.hpp"
#include "../workflowrunner.hpp"
#include "../../usertypes/torrent.hpp"
#include "../../../session.hpp"

using porla::Lua::UserTypes::Torrent;
using porla::Lua::Workflows::Triggers::TorrentFinished;
using porla::Lua::Workflows::Triggers::TorrentFinishedOptions;
using porla::Lua::Workflows::WorkflowFilter;
using porla::Lua::Workflows::WorkflowRunner;
using porla::Lua::Workflows::WorkflowRunnerOptions;

class TorrentFinished::State
{
public:
    explicit State(TorrentFinishedOptions options)
            : m_opts(std::move(options))
    {
        m_on_torrent_finished = m_opts.session.OnTorrentFinished(
            [&](const lt::torrent_status& ts) { OnTorrentFinished(ts); });
    }

    ~State()
    {
        m_on_torrent_finished.disconnect();
    }

private:
    void OnTorrentFinished(const lt::torrent_status& ts)
    {
        sol::table ctx           = m_opts.lua.create_table();
        ctx["actions"]           = std::vector<sol::object>();
        ctx["lt:torrent_handle"] = ts.handle;
        ctx["torrent"]           = Torrent{ts.handle};

        if (!WorkflowFilter::Includes(m_opts.filter, ctx)) return;

        const WorkflowRunnerOptions runner_opts{
            .io      = m_opts.io,
            .lua     = m_opts.lua,
            .session = m_opts.session,
        };

        auto workflow = std::make_shared<WorkflowRunner>(runner_opts, ctx, m_opts.actions);
        boost::asio::post(m_opts.io, [workflow]() { workflow->Run(); });
    }

    TorrentFinishedOptions m_opts;
    boost::signals2::connection m_on_torrent_finished;
};

TorrentFinished::TorrentFinished(const TorrentFinishedOptions& opts)
        : m_state(std::make_unique<State>(opts))
{
}

TorrentFinished::~TorrentFinished() = default;
