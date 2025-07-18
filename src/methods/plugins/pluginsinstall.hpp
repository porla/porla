#pragma once

#include <filesystem>
#include <thread>
#include <vector>

#include "../../config.hpp"
#include "../method.hpp"
#include "pluginsinstall_reqres.hpp"

namespace porla::Lua
{
    class PluginEngine;
}

namespace porla::Methods
{
    struct PluginsInstallOptions
    {
        bool                      allow_git;
        fs::path                  install_dir;
        boost::asio::io_context&  io;
        porla::Lua::PluginEngine& plugin_engine;
    };

    template <bool SSL> class PluginsInstall : public Method<PluginsInstallReq, PluginsInstallRes, SSL>
    {
    public:
        explicit PluginsInstall(PluginsInstallOptions options);

    protected:
        void Invoke(const PluginsInstallReq& req, WriteCb<PluginsInstallRes, SSL> cb) override;

    private:
        std::vector<std::shared_ptr<std::thread>> m_git_install_threads;
        PluginsInstallOptions m_options;
    };
}
