#include "pluginsinstall.hpp"

#include <boost/log/trivial.hpp>
#include <utility>
#include <git2.h>
#include <toml++/toml.h>

#include "../../lua/plugins/plugin.hpp"
#include "../../lua/plugins/pluginengine.hpp"

using porla::Lua::Plugins::PluginEngine;
using porla::Lua::Plugins::PluginInstallOptions;

using porla::Methods::PluginsInstall;
using porla::Methods::PluginsInstallReq;
using porla::Methods::PluginsInstallRes;

PluginsInstall::PluginsInstall(PluginEngine& plugin_engine, fs::path plugin_state_dir)
    : m_plugin_engine(plugin_engine)
    , m_plugin_state_dir(std::move(plugin_state_dir))
{
}

void PluginsInstall::Invoke(const PluginsInstallReq& req, WriteCb<PluginsInstallRes> cb)
{
    std::string real_path = req.path;

    if (req.source.value_or("dir") == "git")
    {
        std::string tmp_dir = fs::temp_directory_path() / "porla-git-plugin-XXXXXX";
        mkdtemp(tmp_dir.data());

        BOOST_LOG_TRIVIAL(info) << "Cloning plugin to temporary directory " << tmp_dir;

        git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
        git_repository *repo = nullptr;
        int error = git_clone(&repo, real_path.c_str(), tmp_dir.c_str(), &clone_opts);
        git_repository_free(repo);

        if (error < 0)
        {
            const git_error *e = git_error_last();
            printf("Error %d/%d: %s\n", error, e->klass, e->message);
        }
        else
        {
            const fs::path manifest = fs::path(tmp_dir) / "manifest.toml";

            if (fs::exists(manifest))
            {
                try
                {
                    const auto tbl = toml::parse_file(manifest.string());
                    const auto plugin_name = tbl["plugin"]["name"].as<std::string>();

                    BOOST_LOG_TRIVIAL(info) << "Plugin " << plugin_name << " found";

                    if (!fs::exists(m_plugin_state_dir))
                    {
                        fs::create_directories(m_plugin_state_dir);
                    }

                    const fs::path plugin_dir = m_plugin_state_dir / plugin_name->get();

                    if (fs::exists(plugin_dir))
                    {
                        // toodooo
                    }
                    else
                    {
                        BOOST_LOG_TRIVIAL(info) << "Moving plugin from " << tmp_dir << " to " << plugin_dir;
                        fs::rename(tmp_dir, plugin_dir);

                        // If we get here - adjust the path on the request to the
                        // plugin directory.
                        real_path = plugin_dir;
                    }
                }
                catch (const std::exception& err)
                {
                    BOOST_LOG_TRIVIAL(error) << "Failed to install plugin: " << err.what();
                }
            }

            BOOST_LOG_TRIVIAL(info) << "Plugin cloned";
        }
    }

    const PluginInstallOptions install_options{
        .config = req.config,
        .enable = req.enable.value_or(false),
        .path   = real_path
    };

    std::error_code ec;
    m_plugin_engine.Install(install_options, ec);

    if (ec)
    {
        return cb.Error(-1, "Failed to install plugin");
    }

    cb.Ok({});
}
