#include "torrentadded.hpp"

#include <utility>

#include <boost/signals2.hpp>

#include "../workflowfilter.hpp"
#include "../workflowrunner.hpp"
#include "../../usertypes/torrent.hpp"
#include "../../../session.hpp"

using porla::Lua::UserTypes::Torrent;
using porla::Lua::Workflows::Triggers::TorrentAdded;
using porla::Lua::Workflows::Triggers::TorrentAddedOptions;
using porla::Lua::Workflows::WorkflowFilter;
using porla::Lua::Workflows::WorkflowRunner;
using porla::Lua::Workflows::WorkflowRunnerOptions;

class TorrentAdded::State
{
public:
    explicit State(TorrentAddedOptions options)
        : m_opts(std::move(options))
    {
        m_on_torrent_added = m_opts.session.OnTorrentAdded(
            [&](const lt::torrent_status& ts) { OnTorrentAdded(ts); });
    }

    ~State()
    {
        m_on_torrent_added.disconnect();
    }

private:
    void OnTorrentAdded(const lt::torrent_status& ts)
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
        boost::asio::post(
            m_opts.io,
            [workflow]() { workflow->Run(); });
    }

    TorrentAddedOptions m_opts;
    boost::signals2::connection m_on_torrent_added;
};

TorrentAdded::TorrentAdded(const TorrentAddedOptions& opts)
    : m_state(std::make_unique<State>(opts))
{
}

TorrentAdded::~TorrentAdded() = default;
