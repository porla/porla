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
    class PluginsUpdate : public Method<PluginsUpdateReq, PluginsUpdateRes>
    {
    public:
        explicit PluginsUpdate(porla::Lua::Plugins::PluginEngine& plugin_engine);

    protected:
        void Invoke(const PluginsUpdateReq& req, WriteCb<PluginsUpdateRes> cb) override;

    private:
        porla::Lua::Plugins::PluginEngine& m_plugin_engine;
        std::map<std::string, std::shared_ptr<std::thread>> m_running_updates;
    };
}
