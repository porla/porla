#pragma once

#include <map>
#include <thread>

#include "../method.hpp"
#include "pluginsupdate_reqres.hpp"

namespace porla::Lua::Plugins
{
    class PluginEngine;
}

namespace porla::Methods
{
    struct PluginsUpdateOptions
    {
        boost::asio::io_context&           io;
        porla::Lua::Plugins::PluginEngine& plugin_engine;
    };

    class PluginsUpdate : public Method<PluginsUpdateReq, PluginsUpdateRes>
    {
    public:
        explicit PluginsUpdate(const PluginsUpdateOptions& options);

    protected:
        void Invoke(const PluginsUpdateReq& req, WriteCb<PluginsUpdateRes> cb) override;

    private:
        PluginsUpdateOptions m_options;
        std::map<std::string, std::shared_ptr<std::thread>> m_running_updates;
    };
}
