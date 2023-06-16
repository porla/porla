#include "pluginsinstall.hpp"

#include <boost/log/trivial.hpp>
#include <utility>
#include <git2.h>
#include <toml++/toml.h>

#include "../../lua/plugins/plugin.hpp"
#include "../../lua/plugins/pluginengine.hpp"

#define MAX_CONCURRENT_INSTALLS 3

using porla::Lua::Plugins::PluginEngine;
using porla::Lua::Plugins::PluginInstallOptions;

using porla::Methods::PluginsInstall;
using porla::Methods::PluginsInstallOptions;
using porla::Methods::PluginsInstallReq;
using porla::Methods::PluginsInstallRes;

struct InstallFromGitOptions
{
    porla::Methods::WriteCb<PluginsInstallRes> callback;
    std::optional<std::string>                 config;
    bool                                       enable;
    std::string                                git_repository;
    fs::path                                   install_dir;
    boost::asio::io_context&                   io;
    PluginEngine&                              plugin_engine;
    std::vector<std::shared_ptr<std::thread>>& threads;
};

struct InstallFromDirectoryOptions
{
    porla::Methods::WriteCb<PluginsInstallRes>  callback;
    std::optional<std::string>                  config;
    bool                                        enable;
    fs::path                                    path;
    PluginEngine&                               plugin_engine;
};

static void InstallFromDirectory(InstallFromDirectoryOptions& options)
{
    const PluginInstallOptions install_options{
        .config = options.config,
        .enable = options.enable,
        .path   = options.path
    };

    std::error_code ec;
    options.plugin_engine.Install(install_options, ec);

    if (ec)
    {
        options.callback.Error(-1, "Failed to install plugin");
        return;
    }

    options.callback.Ok({});
}

// This is the last step in the install-from-git flow. When we get here, we
// must be in the main thread again. We'll remove the thread keep on trucking.
static void InstallFromGitEnd(std::thread::id install_thread_id, const fs::path& path, const std::shared_ptr<InstallFromGitOptions>& options)
{
    auto install_thread = std::find_if(
        options->threads.begin(),
        options->threads.end(),
        [&install_thread_id](auto const& t) { return t->get_id() == install_thread_id; });

    if (install_thread == options->threads.end())
    {
        // This really shouldn't happen.
        BOOST_LOG_TRIVIAL(error) << "Where did our install thread go?";
    }
    else
    {
        BOOST_LOG_TRIVIAL(debug) << "Erasing worker thread " << install_thread_id;

        if ((*install_thread)->joinable())
        {
            (*install_thread)->join();
        }

        options->threads.erase(install_thread);
    }

    InstallFromDirectoryOptions dir_options{
        .callback      = std::move(options->callback),
        .config        = options->config,
        .enable        = options->enable,
        .path          = path,
        .plugin_engine = options->plugin_engine
    };

    InstallFromDirectory(dir_options);
}

// This method runs in a separate thread. It will post to InstallFromGitEnd
// when it's done (or has errored).
static void InstallFromGit(const std::shared_ptr<InstallFromGitOptions>& options)
{
    BOOST_LOG_TRIVIAL(info) << "Installing plugin from repository " << options->git_repository;

    std::string tmp_dir = fs::temp_directory_path() / "porla-git-plugin-XXXXXX";
    mkdtemp(tmp_dir.data());

    BOOST_LOG_TRIVIAL(debug) << "Cloning repository to " << tmp_dir;

    git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
    git_repository *repo = nullptr;
    int error = git_clone(&repo, options->git_repository.c_str(), tmp_dir.c_str(), &clone_opts);
    git_repository_free(repo);

    if (error < 0)
    {
        const git_error *e = git_error_last();
        printf("Error %d/%d: %s\n", error, e->klass, e->message);
    }
    else
    {
        const fs::path manifest = fs::path(tmp_dir) / "manifest.toml";

        if (!fs::exists(manifest))
        {
            BOOST_LOG_TRIVIAL(error) << "Plugin does not have a manifest file";
            return;
        }

        try
        {
            const auto tbl = toml::parse_file(manifest.string());
            const auto plugin_name = tbl["plugin"]["name"].as<std::string>();

            BOOST_LOG_TRIVIAL(info) << "Plugin " << *plugin_name << " found";

            const fs::path plugin_dir = options->install_dir / plugin_name->get();

            if (fs::exists(plugin_dir))
            {
                BOOST_LOG_TRIVIAL(error) << "Plugin installation directory already exists - " << plugin_dir;
                return;
            }

            BOOST_LOG_TRIVIAL(info) << "Moving plugin from " << tmp_dir << " to " << plugin_dir;
            fs::copy(tmp_dir, plugin_dir, fs::copy_options::recursive);

            boost::asio::post(
                options->io,
                [o = options, p = plugin_dir, thread_id = std::this_thread::get_id()]()
                {
                    InstallFromGitEnd(thread_id, p, o);
                });
        }
        catch (const std::exception &err)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to install plugin: " << err.what();
        }
    }
}

PluginsInstall::PluginsInstall(PluginsInstallOptions options)
    : m_options(std::move(options))
{
}

void PluginsInstall::Invoke(const PluginsInstallReq& req, WriteCb<PluginsInstallRes> cb)
{
    std::string real_path = req.path;

    if (req.source.value_or("dir") == "git")
    {
        if (!m_options.allow_git)
        {
            return cb.Error(-1, "Installing from Git is not allowed - see configuration");
        }

        if (m_git_install_threads.size() >= MAX_CONCURRENT_INSTALLS)
        {
            return cb.Error(-2, "Max amount of installs running");
        }

        if (!fs::exists(m_options.install_dir))
        {
            fs::create_directories(m_options.install_dir);
        }

        auto options = std::make_shared<InstallFromGitOptions>(InstallFromGitOptions{
            .callback       = std::move(cb),
            .config         = req.config,
            .enable         = req.enable.value_or(false),
            .git_repository = req.path,
            .install_dir    = m_options.install_dir,
            .io             = m_options.io,
            .plugin_engine  = m_options.plugin_engine,
            .threads        = m_git_install_threads
        });

        m_git_install_threads.emplace_back(
            std::make_shared<std::thread>([o = options]() { InstallFromGit(o); }));

        return;
    }

    InstallFromDirectoryOptions options{
        .callback      = std::move(cb),
        .config        = req.config,
        .enable        = req.enable.value_or(false),
        .path          = req.path,
        .plugin_engine = m_options.plugin_engine
    };

    InstallFromDirectory(options);
}
