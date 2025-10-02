#include "plugin.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>
#include <zip.h>

#include "packages.hpp"

namespace fs = std::filesystem;
using porla::Lua::Plugin;
using porla::Lua::PluginLoadOptions;

struct Plugin::State
{
    std::map<std::string, std::vector<char>> files;
    PluginLoadOptions                        load_options;
    sol::state                               lua;
};

static std::vector<char> LoadFile(const fs::path& path)
{
    std::ifstream path_stream;
    path_stream.open(path, std::ios::binary);
    path_stream.seekg(0, std::ios_base::end);
    const std::streamsize path_size = path_stream.tellg();
    path_stream.seekg(0, std::ios_base::beg);

    std::vector<char> path_buffer;
    path_buffer.resize(path_size);

    path_stream.read(&path_buffer[0], path_size);
    path_stream.close();

    return path_buffer;
}

static std::map<std::string, std::vector<char>> LoadArchive(const std::vector<char>& buffer)
{
    std::map<std::string, std::vector<char>> files;

    zip_error_t err;
    zip_source_t* source = zip_source_buffer_create(
        buffer.data(),
        buffer.size(),
        0,
        &err);

    zip_t* archive = zip_open_from_source(source, ZIP_RDONLY, &err);
    zip_int64_t num_entries = zip_get_num_entries(archive, ZIP_FL_UNCHANGED);

    for (int i = 0; i < num_entries; i++)
    {
        zip_stat_t st;
        zip_stat_init(&st);
        zip_stat_index(archive, i, 0, &st);

        if (st.size == 0)
        {
            continue;
        }

        zip_file* file = zip_fopen_index(archive, i, ZIP_FL_UNCHANGED);

        std::vector<char> buffer;
        buffer.resize(st.size);

        zip_fread(file, &buffer[0], st.size);
        zip_fclose(file);

        files.emplace(st.name, buffer);
    }

    zip_close(archive);
    zip_source_close(source);

    return files;
}

std::unique_ptr<Plugin> Plugin::LoadFromArchive(
    const std::vector<char>& buffer,
    const std::optional<std::string>& config,
    const PluginLoadOptions& opts)
{
    return nullptr;
}

std::unique_ptr<Plugin> Plugin::LoadFromPath(
    const std::filesystem::path& path,
    const std::optional<std::string>& config,
    const PluginLoadOptions& opts)
{
    auto state = std::make_unique<State>(State{
        .load_options = opts,
        .lua          = sol::state()
    });

    state->lua.open_libraries(
        sol::lib::base,
        sol::lib::io,
        sol::lib::os,
        sol::lib::package,
        sol::lib::string,
        sol::lib::table);

    state->lua.globals()["__load_opts"] = state->load_options;
    state->lua.globals()["porla"]       = state->lua.create_table();

    Packages::Config::Register(state->lua);
    Packages::Cron::Register(state->lua);
    Packages::Events::Register(state->lua);
    Packages::FileSystem::Register(state->lua);
    Packages::HttpClient::Register(state->lua);
    Packages::Json::Register(state->lua);
    Packages::Log::Register(state->lua);
    Packages::PQL::Register(state->lua);
    Packages::Presets::Register(state->lua);
    Packages::Process::Register(state->lua);
    Packages::Sessions::Register(state->lua);
    Packages::Sqlite::Register(state->lua);
    Packages::Timers::Register(state->lua);
    Packages::Torrents::Register(state->lua);
    Packages::Workflows::Register(state->lua);

    try
    {
        const fs::path plugin_lua = path / "plugin.lua";
        state->lua.script_file(plugin_lua.string());

        if (state->lua.globals()["porla"]["init"].is<sol::function>())
        {
            BOOST_LOG_TRIVIAL(info) << config.value_or("");

            state->lua.globals()["porla"]["init"](
                config.has_value()
                    ? sol::object(state->lua.script(config.value()))
                    : sol::nil);
        }

        return std::unique_ptr<Plugin>(new Plugin(std::move(state)));
    }
    catch (const sol::error& err)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load plugin: " << err.what();
    }

    return nullptr;
}

Plugin::Plugin(std::unique_ptr<State> state)
    : m_state(std::move(state))
{
}

Plugin::~Plugin()
{
    if (m_state->lua.globals()["porla"]["destroy"].is<sol::function>())
    {
        m_state->lua.globals()["porla"]["destroy"]();
    }
}
