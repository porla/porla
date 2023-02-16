#include "workflowrunner.hpp"

#include <boost/log/trivial.hpp>

#include "actionbuilder.hpp"

using porla::Lua::Workflows::ActionBuilder;
using porla::Lua::Workflows::WorkflowRunner;

WorkflowRunner::WorkflowRunner(const WorkflowRunnerOptions& opts, sol::table ctx, std::vector<sol::object> actions)
    : m_opts(opts)
    , m_ctx(std::move(ctx))
    , m_actions(std::move(actions))
    , m_current_action_index(0)
    , m_running(false)
{
}

WorkflowRunner::~WorkflowRunner() = default;

void WorkflowRunner::Complete()
{
    m_current_action_index++;

    boost::asio::post(
        m_opts.io,
        [_this = shared_from_this()]()
        {
            _this->RunOne();
        });
}

void WorkflowRunner::Run()
{
    if (m_running) return;
    m_running = true;

    boost::asio::post(
        m_opts.io,
        [_this = shared_from_this()]()
        {
            _this->RunOne();
        });
}

void WorkflowRunner::RunOne()
{
    if (m_actions.empty())                          return;
    if (m_current_action_index >= m_actions.size()) return;

    const auto& current_action = m_actions.at(m_current_action_index);

    BOOST_LOG_TRIVIAL(info) << "Running action " << m_current_action_index + 1 << " of " << m_actions.size();

    if (current_action.is<ActionBuilder*>())
    {
        const ActionBuilderOptions opts = {
            .io      = m_opts.io,
            .session = m_opts.session
        };

        BOOST_LOG_TRIVIAL(info) << "Creating action instance";

        try
        {
            auto instance = current_action.as<ActionBuilder*>()->Build(opts);

            porla::Lua::Workflows::ActionParams params{
                .context = m_ctx
            };

            BOOST_LOG_TRIVIAL(info) << "Invoking action";

            instance->Invoke(params, shared_from_this());
        }
        catch (const std::exception& ex)
        {
            BOOST_LOG_TRIVIAL(error) << "Error when invoking action: " << ex.what();
        }
    }
}
