#include "workflowactionhttprequest.hpp"

#include "../workflows/actions/httprequest.hpp"

using porla::Lua::UserTypes::WorkflowActionHttpRequest;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;

WorkflowActionHttpRequest::WorkflowActionHttpRequest(sol::table args)
    : m_args(std::move(args))
{
    if (!m_args["url"].is<std::string>())
    {
        throw std::runtime_error("Expected string value in HttpRequest:url parameter");
    }
}

std::shared_ptr<Action> WorkflowActionHttpRequest::Build(const ActionBuilderOptions& opts)
{
    std::map<std::string, std::string> headers;

    if (m_args["headers"].is<sol::as_table_t<std::map<std::string, std::string>>>())
    {
        headers = m_args["headers"].get<sol::as_table_t<std::map<std::string, std::string>>>();
    }

    porla::Lua::Workflows::Actions::HttpRequestOptions http_request_opts{
        .body    = m_args["body"],
        .io      = opts.io,
        .headers = headers,
        .method  = m_args["method"],
        .url     = m_args["url"]
    };

    return std::make_shared<porla::Lua::Workflows::Actions::HttpRequest>(http_request_opts);
}
