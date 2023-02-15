#pragma once

#include <boost/asio.hpp>

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
    };

    class Engine
    {
    public:
        explicit Engine(const EngineOptions& options);
        ~Engine();

    private:
        struct State;
        struct WorkflowInstance;

        std::unique_ptr<State> m_state;
        std::vector<WorkflowInstance> m_workflow_instances;
    };
}
