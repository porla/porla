#include "executor.hpp"

#include <boost/log/trivial.hpp>
#include <boost/signals2.hpp>
#include <libtorrent/torrent_status.hpp>

#include "torrentcontextprovider.hpp"
#include "workflow.hpp"
#include "../session.hpp"

namespace lt = libtorrent;
using porla::Workflows::Executor;

struct Executor::State
{
    boost::signals2::connection torrent_added_connection;
    boost::signals2::connection torrent_finished_connection;
    boost::signals2::connection torrent_mediainfo_connection;
};

Executor::Executor(const ExecutorOptions& options)
    : m_session(options.session)
    , m_state(std::make_unique<State>())
    , m_workflows(options.workflows)
    , m_action_factory(options.action_factory)
{
    m_state->torrent_added_connection = m_session.OnTorrentAdded([this](const auto& ts) { OnTorrentAdded(ts); });
    m_state->torrent_finished_connection = m_session.OnTorrentFinished([this](const auto& ts) { OnTorrentFinished(ts); });
    m_state->torrent_mediainfo_connection = m_session.OnTorrentMediaInfo([this](const auto& th) { OnTorrentMediaInfo(th); });
}

Executor::~Executor()
{
    m_state->torrent_added_connection.disconnect();
    m_state->torrent_finished_connection.disconnect();
    m_state->torrent_mediainfo_connection.disconnect();
}

void Executor::OnTorrentAdded(const lt::torrent_status& ts)
{
    TriggerWorkflows("torrent_added", {
        {"torrent", std::make_shared<porla::Workflows::TorrentContextProvider>(ts)}
    });
}

void Executor::OnTorrentFinished(const lt::torrent_status& ts)
{
    TriggerWorkflows("torrent_finished", {
        {"torrent", std::make_shared<porla::Workflows::TorrentContextProvider>(ts)}
    });
}

void Executor::OnTorrentMediaInfo(const libtorrent::torrent_handle& th)
{
    TriggerWorkflows("torrent_mediainfo", {
        {"torrent", std::make_shared<porla::Workflows::TorrentContextProvider>(th.status())}
    });
}

void Executor::TriggerWorkflows(
    const std::string& event_name,
    const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts)
{
    BOOST_LOG_TRIVIAL(info) << "Running workflows for " << event_name;

    for (const auto& workflow : m_workflows)
    {
        if (!workflow->ShouldExecute(event_name, contexts))
        {
            continue;
        }

        BOOST_LOG_TRIVIAL(info) << "Invoking workflow";

        workflow->Execute(*m_action_factory, contexts);
    }
}
