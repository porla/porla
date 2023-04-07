#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "action.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Lua::Workflows
{
    struct WorkflowRunnerOptions
    {
        boost::asio::io_context& io;
        sol::state&              lua;
        porla::ISession&         session;
    };

    class WorkflowRunner : public ActionCallback, public std::enable_shared_from_this<WorkflowRunner>
    {
    public:
        typedef boost::signals2::signal<void()> VoidSignal;

        explicit WorkflowRunner(const WorkflowRunnerOptions& opts, sol::table ctx, std::vector<sol::object> actions);
        ~WorkflowRunner();

        boost::signals2::connection OnFinished(const VoidSignal::slot_type& subscriber);

        void Complete(sol::object output) override;
        void Complete() override;
        void Run();

    private:
        void RunOne();

        WorkflowRunnerOptions    m_opts;
        sol::table               m_ctx;
        std::vector<sol::object> m_actions;
        bool                     m_running;
        std::size_t              m_current_action_index;
        VoidSignal               m_on_finished;

    };
}
