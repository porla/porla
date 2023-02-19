#pragma once

#include <filesystem>
#include <memory>

namespace boost::asio
{
    class io_context;
}

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
        struct State;
        std::unique_ptr<State> m_state;
    };
}
