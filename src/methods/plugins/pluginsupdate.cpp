#include "pluginsupdate.hpp"

#include <boost/log/trivial.hpp>
#include <git2.h>

#include "../../lua/plugins/plugin.hpp"
#include "../../lua/plugins/pluginengine.hpp"

using porla::Lua::Plugins::PluginEngine;
using porla::Lua::Plugins::PluginInstallOptions;

using porla::Methods::PluginsUpdate;
using porla::Methods::PluginsUpdateReq;
using porla::Methods::PluginsUpdateRes;

struct FetchHeadEntry
{
    unsigned int is_merge;
    git_oid      oid;
    std::string  ref_name;
    std::string  remote_url;
};

int FetchHeadForEachCallback(
    const char* ref_name,
    const char* remote_url,
    const git_oid *oid,
    unsigned int is_merge,
    void* payload)
{

    git_oid entry_oid;
    git_oid_cpy(&entry_oid, oid);

    auto fetch_head_entries = static_cast<std::vector<FetchHeadEntry>*>(payload);
    fetch_head_entries->emplace_back(FetchHeadEntry{
        .is_merge   = is_merge,
        .oid        = entry_oid,
        .ref_name   = ref_name,
        .remote_url = remote_url
    });

    return 0;
}

struct UpdatePluginOptions
{
    porla::Methods::WriteCb<PluginsUpdateRes>            callback;
    boost::asio::io_context&                             io;
    fs::path                                             path;
    PluginEngine&                                        plugin_engine;
    std::string                                          plugin_name;
    std::map<std::string, std::shared_ptr<std::thread>>& running_updates;
};

static void UpdatePluginEnd(
    const std::shared_ptr<UpdatePluginOptions>& opts,
    std::thread::id update_thread_id,
    bool is_error,
    const std::string& error = "")
{
    auto update_thread = std::find_if(
        opts->running_updates.begin(),
        opts->running_updates.end(),
        [update_thread_id](const auto& pair)
        {
            return pair.second->get_id() == update_thread_id;
        });

    if (update_thread == opts->running_updates.end())
    {
        BOOST_LOG_TRIVIAL(warning) << "Could not find update thread";
    }
    else
    {
        if (update_thread->second->joinable())
        {
            update_thread->second->join();
        }

        opts->running_updates.erase(update_thread);
    }

    if (is_error)
    {
        BOOST_LOG_TRIVIAL(error) << error;
        return opts->callback.Error(-10, error);
    }

    opts->plugin_engine.Reload(opts->plugin_name);

    opts->callback.Ok({});
}

static void UpdatePlugin(const std::shared_ptr<UpdatePluginOptions>& opts)
{
    BOOST_LOG_TRIVIAL(info) << "Updating plugin at path " << opts->path;

    const auto error = [](const std::shared_ptr<UpdatePluginOptions>& opts, const std::string& error)
    {
        boost::asio::post(
            opts->io,
            [o = opts, err = error, tid = std::this_thread::get_id()]()
            {
                UpdatePluginEnd(o, tid, true, err);
            });
    };

    git_repository* repo;
    int res = git_repository_open_ext(
        &repo,
        fs::absolute(opts->path).c_str(),
        GIT_REPOSITORY_OPEN_NO_SEARCH,
        nullptr);

    if (res != GIT_OK)
    {
        return error(opts, "Could not open plugin repository");
    }

    git_strarray remotes = {nullptr};
    res = git_remote_list(&remotes, repo);

    if (res != GIT_OK)
    {
        git_repository_free(repo);
        return error(opts, "Could not list remotes");
    }

    BOOST_LOG_TRIVIAL(info) << "Fetching from " << remotes.count << " remote(s)";

    for (int i = 0; i < remotes.count; i++)
    {
        git_remote* remote;
        res = git_remote_lookup(&remote, repo, remotes.strings[i]);

        if (res != GIT_OK)
        {
            git_repository_free(repo);
            git_strarray_free(&remotes);

            return error(opts, "Could not find remote");
        }

        git_fetch_options options = GIT_FETCH_OPTIONS_INIT;
        res = git_remote_fetch(remote, nullptr, &options, nullptr);

        if (res != GIT_OK)
        {
            git_strarray_free(&remotes);
            git_repository_free(repo);
            git_remote_free(remote);

            return error(opts, "Could not fetch remote");
        }

        git_remote_free(remote);
    }

    std::vector<FetchHeadEntry> fetch_head_entries;
    res = git_repository_fetchhead_foreach(repo, FetchHeadForEachCallback, &fetch_head_entries);

    if (res != GIT_OK)
    {
        git_repository_free(repo);
        return error(opts, "Could not enumerate FETCH_HEAD entries");
    }

    if (fetch_head_entries.empty())
    {
        git_repository_free(repo);
        return error(opts, "Plugin repository does not have any FETCH_HEAD entries");
    }

    git_reference* head;
    res = git_repository_head(&head, repo);

    if (res != GIT_OK)
    {
        git_repository_free(repo);
        return error(opts, "Could not get repository head");
    }

    const auto head_fetch_head = std::find_if(
        fetch_head_entries.begin(),
        fetch_head_entries.end(),
        [head](const auto& entry)
        {
            return entry.ref_name == git_reference_name(head);
        });

    if (head_fetch_head == fetch_head_entries.end())
    {
        git_reference_free(head);
        git_repository_free(repo);

        return error(opts, "Repository head does not have a FETCH_HEAD");
    }

    git_annotated_commit* their_heads[] = {nullptr};
    res = git_annotated_commit_lookup(their_heads, repo, &head_fetch_head->oid);

    if (res != GIT_OK)
    {
        git_reference_free(head);
        git_repository_free(repo);

        return error(opts, "Failed to lookup annotated commit");
    }

    git_merge_analysis_t analysis;
    git_merge_preference_t preference;
    res = git_merge_analysis(
        &analysis,
        &preference,
        repo,
        reinterpret_cast<const git_annotated_commit **>(&their_heads),
        1);

    if (res != GIT_OK)
    {
        git_annotated_commit_free(their_heads[0]);
        git_reference_free(head);
        git_repository_free(repo);

        return error(opts, "Failed to analyze merge");
    }

    const auto is_up_to_date = (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) == GIT_MERGE_ANALYSIS_UP_TO_DATE;
    const auto is_fast_forward = (analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) == GIT_MERGE_ANALYSIS_FASTFORWARD;

    if (!is_up_to_date && is_fast_forward)
    {
        BOOST_LOG_TRIVIAL(info) << "Fast-forwarding repository";

        git_reference* target_ref;
        git_reference_set_target(&target_ref, head, &head_fetch_head->oid, "pull: Fast-forward");
        git_reset_from_annotated(repo, their_heads[0], GIT_RESET_HARD, nullptr);
        git_reference_free(target_ref);
    }

    git_reference_free(head);
    git_annotated_commit_free(their_heads[0]);
    git_repository_state_cleanup(repo);
    git_repository_free(repo);

    boost::asio::post(
        opts->io,
        [o = opts, tid = std::this_thread::get_id()]()
        {
            UpdatePluginEnd(o, tid, false);
        });
}

PluginsUpdate::PluginsUpdate(const PluginsUpdateOptions& options)
    : m_options(options)
{
}

void PluginsUpdate::Invoke(const PluginsUpdateReq& req, WriteCb<PluginsUpdateRes> cb)
{
    const auto plugin_state = m_options.plugin_engine.Plugins().find(req.name);

    if (plugin_state == m_options.plugin_engine.Plugins().end())
    {
        return cb.Error(-1, "Plugin not found");
    }

    const auto is_repository = git_repository_open_ext(
        nullptr,
        fs::absolute(plugin_state->second.path).c_str(),
        GIT_REPOSITORY_OPEN_NO_SEARCH,
        nullptr) == GIT_OK;

    if (!is_repository)
    {
        return cb.Error(-2, "Plugin path is not a Git repository");
    }

    if (m_running_updates.find(req.name) != m_running_updates.end())
    {
        return cb.Error(-3, "An update is already in progress for this plugin");
    }

    auto options = std::make_shared<UpdatePluginOptions>(UpdatePluginOptions{
        .callback        = std::move(cb),
        .io              = m_options.io,
        .path            = plugin_state->second.path,
        .plugin_engine   = m_options.plugin_engine,
        .plugin_name     = req.name,
        .running_updates = m_running_updates
    });

    m_running_updates.insert(
        { req.name, std::make_shared<std::thread>([o = std::move(options)]() { UpdatePlugin(o); })});
}
