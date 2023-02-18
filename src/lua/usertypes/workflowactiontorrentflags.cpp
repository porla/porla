#include "workflowactiontorrentflags.hpp"

#include "../workflows/actions/torrentflags.hpp"

using porla::Lua::UserTypes::WorkflowActionTorrentFlags;
using porla::Lua::Workflows::Action;
using porla::Lua::Workflows::ActionBuilderOptions;
using porla::Lua::Workflows::Actions::TorrentFlags;
using porla::Lua::Workflows::Actions::TorrentFlagsOptions;

WorkflowActionTorrentFlags::WorkflowActionTorrentFlags(sol::table args)
{
    if (args["set"] && args["set"].is<sol::as_table_t<std::vector<std::string>>>())
    {
        m_set = args["set"].get<sol::as_table_t<std::vector<std::string>>>();
    }

    if (args["unset"] && args["unset"].is<sol::as_table_t<std::vector<std::string>>>())
    {
        m_unset = args["unset"].get<sol::as_table_t<std::vector<std::string>>>();
    }

    if (m_set.empty() && m_unset.empty())
    {
        throw std::runtime_error("Expected string list in either TorrentFlag:set or TorrentFlag:unset parameter");
    }
}

std::shared_ptr<Action> WorkflowActionTorrentFlags::Build(const ActionBuilderOptions& opts)
{
    const TorrentFlagsOptions move_options{
        .session = opts.session,
        .set     = m_set,
        .unset   = m_unset
    };

    return std::make_shared<TorrentFlags>(move_options);
}
