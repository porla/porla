#pragma once

#include <map>
#include <memory>
#include <vector>

#include <libtorrent/fwd.hpp>

namespace porla
{
    class ISession;
}

namespace porla::Workflows
{
    class ActionFactory;
    class ContextProvider;
    class IWorkflow;

    struct ExecutorOptions
    {
        porla::ISession& session;
        std::vector<std::shared_ptr<IWorkflow>> workflows;
        std::shared_ptr<ActionFactory> action_factory;
    };

    class Executor
    {
    public:
        explicit Executor(const ExecutorOptions& options);
        ~Executor();

    private:
        void OnTorrentAdded(const libtorrent::torrent_status& ts);
        void OnTorrentFinished(const libtorrent::torrent_status& ts);

        void TriggerWorkflows(
            const std::string& event_name,
            const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts);

        struct State;

        std::shared_ptr<ActionFactory> m_action_factory;
        std::unique_ptr<State> m_state;
        porla::ISession& m_session;
        std::vector<std::shared_ptr<IWorkflow>> m_workflows;
    };
}
