#include "pluginsget.hpp"

#include <boost/log/trivial.hpp>
#include <git2.h>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

using porla::Lua::PluginEngine;
using porla::Lua::PluginInstallOptions;

using porla::Methods::PluginsGet;
using porla::Methods::PluginsGetReq;
using porla::Methods::PluginsGetRes;

template <bool SSL> PluginsGet<SSL>::PluginsGet(PluginEngine& plugin_engine)
    : m_plugin_engine(plugin_engine)
{
}

template <bool SSL> void PluginsGet<SSL>::Invoke(const PluginsGetReq& req, WriteCb<PluginsGetRes, SSL> cb)
{
    const auto plugin_state = m_plugin_engine.Plugins().find(req.name);

    if (plugin_state == m_plugin_engine.Plugins().end())
    {
        return cb.Error(-1, "Plugin not found");
    }

    std::vector<std::string> branches;
    std::vector<std::string> tags;

    const auto is_repository = git_repository_open_ext(
        nullptr,
        plugin_state->second.path.c_str(),
        GIT_REPOSITORY_OPEN_NO_SEARCH,
        nullptr) == GIT_OK;

    if (is_repository)
    {
        git_repository* repo;
        git_repository_open_ext(
            &repo,
            plugin_state->second.path.c_str(),
            GIT_REPOSITORY_OPEN_NO_SEARCH,
            nullptr);

        git_strarray tags_array = {nullptr};
        git_tag_list(&tags_array, repo);

        for (int i = 0; i < tags_array.count; i++)
        {
            tags.emplace_back(tags_array.strings[i]);
        }

        git_strarray_free(&tags_array);

        git_reference* head;
        git_repository_head(&head, repo);
        git_reference_free(head);

        git_strarray refs = {nullptr};
        git_reference_list(&refs, repo);

        for (int i = 0; i < refs.count; i++)
        {
        }

        git_strarray_free(&refs);

        git_repository_free(repo);
    }

    cb.Ok(PluginsGetRes{
        .config = plugin_state->second.config,
        .path   = fs::absolute(plugin_state->second.path),
        .tags   = tags
    });
}


namespace porla::Methods {
    template class PluginsGet<true>;
    template class PluginsGet<false>;
}
