#include "pluginslist.hpp"

#include <git2.h>

#include "../../lua/plugins/plugin.hpp"
#include "../../lua/plugins/pluginengine.hpp"

using porla::Lua::Plugins::PluginEngine;
using porla::Lua::Plugins::PluginInstallOptions;

using porla::Methods::PluginsList;
using porla::Methods::PluginsListReq;
using porla::Methods::PluginsListRes;

PluginsList::PluginsList(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

void PluginsList::Invoke(const PluginsListReq& req, WriteCb<PluginsListRes> cb)
{
    PluginsListRes res = {};

    for (const auto& [ name, state ] : m_plugin_engine.Plugins())
    {
        std::string abs_path = fs::absolute(state.path);

        PluginsListRes::Plugin plugin{
            .name = name,
            .path = abs_path
        };

        git_repository* repo;
        int repo_result = git_repository_open_ext(&repo, abs_path.c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, nullptr);

        if (repo_result < 0)
        {
            res.plugins.emplace_back(std::move(plugin));
            continue;
        }

        PluginsListRes::VersionInfo vi;

        git_reference* head;
        git_repository_head(&head, repo);
        vi.head_name = git_reference_name(head);
        vi.shorthand = git_reference_shorthand(head);

        plugin.version_info = vi;

        res.plugins.emplace_back(std::move(plugin));
    }

    cb.Ok(res);
}
