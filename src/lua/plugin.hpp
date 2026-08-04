#pragma once

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/asio/io_context.hpp>

namespace porla
{
    class Config;
    class CurlMulti;
    class Sessions;
}

namespace porla::Lua
{
    struct PluginLoadOptions
    {
        // NOTE: curl_multi is now held *by value*. The old `std::shared_ptr<CurlMulti>&`
        // meant the plugin kept a reference to the caller's shared_ptr variable, which
        // dangles the moment the caller's PluginLoadOptions (often a temporary) dies.
        Config&                    config;
        std::shared_ptr<CurlMulti> curl_multi;
        boost::asio::io_context&   io;
        porla::Sessions&           sessions;

        // How often we poll suspended coroutines to see whether they finished.
        std::chrono::milliseconds coroutine_poll_interval = std::chrono::milliseconds(250);

        // How long Unload() waits for outstanding coroutines before giving up on them.
        std::chrono::milliseconds unload_timeout = std::chrono::seconds(30);
    };

    class Plugin
    {
    public:
        struct Meta
        {
            std::optional<std::string> name;
            std::optional<std::string> version;
        };

        // Invoked (via boost::asio::post, so never re-entrantly) once `destroy` and all
        // outstanding coroutines have finished. It is safe to delete the Plugin from here.
        using UnloadCallback = std::function<void()>;

        static std::unique_ptr<Plugin> LoadFromArchive(
            const std::vector<char>& buffer,
            const std::optional<std::string>& config,
            const PluginLoadOptions& opts);

        static std::unique_ptr<Plugin> LoadFromPath(
            const std::filesystem::path& path,
            const std::optional<std::string>& config,
            const PluginLoadOptions& opts);

        Plugin(const Plugin&)            = delete;
        Plugin(Plugin&&)                 = delete;
        Plugin& operator=(const Plugin&) = delete;
        Plugin& operator=(Plugin&&)      = delete;

        ~Plugin();

        [[nodiscard]] std::optional<Meta> GetMeta() const;

        // Diagnostics.
        [[nodiscard]] std::size_t ActiveCoroutines() const;
        [[nodiscard]] bool        IsUnloading() const;
        [[nodiscard]] bool        IsUnloaded() const;

        // Asynchronous shutdown. Spawns `destroy` as a coroutine (so it may sleep(),
        // do http calls, etc), then waits for every outstanding coroutine of this plugin
        // to run to completion before invoking `callback`.
        //
        // Calling this twice is a no-op apart from replacing the callback. Destroying the
        // Plugin without calling Unload() falls back to a *synchronous*, non-yieldable
        // `destroy` call and logs a warning.
        void Unload(UnloadCallback callback = {});

    private:
        struct State;

        explicit Plugin(std::shared_ptr<State> state);

        std::shared_ptr<State> m_state;
    };
}
