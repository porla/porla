#include "pluginsinstall.hpp"

#include <boost/log/trivial.hpp>
#include <utility>
#include <toml++/toml.hpp>

#include "../../lua/plugin.hpp"
#include "../../lua/pluginengine.hpp"

#define MAX_CONCURRENT_INSTALLS 3

using porla::Lua::PluginEngine;
using porla::Lua::PluginInstallOptions;

using porla::Methods::PluginsInstall;
using porla::Methods::PluginsInstallOptions;
using porla::Methods::PluginsInstallReq;
using porla::Methods::PluginsInstallRes;

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

PluginsInstall::PluginsInstall(PluginsInstallOptions options)
    : m_options(std::move(options))
{
}

void PluginsInstall::Invoke(const PluginsInstallReq& req, WriteCb<PluginsInstallRes> cb)
{
    std::string real_path = req.path;

    if (req.source.value_or("dir") == "git")
    {
        return cb.Error(-1, "Installing from Git has been removed");
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
