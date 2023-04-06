#include "workflowactionexec.hpp"

#include "../workflows/actions/exec.hpp"

using porla::Lua::UserTypes::WorkflowActionExec;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;

WorkflowActionExec::WorkflowActionExec(sol::table args)
    : m_args(std::move(args))
{
    if (!m_args["file"].is<std::string>())
    {
        throw std::runtime_error("Expected string value in Exec:file parameter");
    }

    if (!m_args["args"].is<sol::as_table_t<std::vector<sol::object>>>())
    {
        throw std::runtime_error("Expected an array of strings and functions in Exec:args parameter");
    }
}

std::shared_ptr<Action> WorkflowActionExec::Build(const ActionBuilderOptions& opts)
{
    porla::Lua::Workflows::Actions::ExecOptions exec_opts{
        .io     = opts.io,
        .file   = m_args["file"],
        .args   = m_args["args"].get<sol::as_table_t<std::vector<sol::object>>>(),
        .std_in = m_args["std_in"]
    };

    return std::make_shared<porla::Lua::Workflows::Actions::Exec>(exec_opts);
}
