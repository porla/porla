#include "pluginsource.hpp"

#include <fstream>

#include <boost/log/trivial.hpp>

#include "../zip.hpp"

using porla::Lua::PluginSource;

namespace
{
    static std::optional<std::vector<char>> ReadFile(const fs::path& path)
    {
        std::ifstream file(path, std::ios::binary);

        if (!file)
        {
            return std::nullopt;
        }

        return std::vector<char>(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());
    }

    // porla only loads plugin.json, any .lua file or all files under ui/
    static bool IsPluginFile(const fs::path& relative)
    {
        return relative == "plugin.json"
            || relative.extension() == ".lua"
            || *relative.begin() == "ui";
    }

    // some directories we know we can skip - this helps local plugin
    // development speed since a reload otherwise takes pretty much
    // forever
    static bool IsSkippedDirectory(const fs::path& relative)
    {
        const auto name = relative.filename().string();
        return name.starts_with(".") || name == "node_modules";
    }
}

std::optional<PluginSource> PluginSource::Load(const fs::path& path)
{
    if (!path.is_absolute())
    {
        BOOST_LOG_TRIVIAL(error) << "Provided path is not absolute: " << path;
        return std::nullopt;
    }

    if (fs::is_regular_file(path) && path.extension() == ".lua")
    {
        const auto file_data = ReadFile(path);

        if (!file_data.has_value())
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to read file " << path;
            return std::nullopt;
        }

        const auto file_name = path.filename();

        return PluginSource{
            .entrypoint = file_name,
            .sources    = {
                {file_name, std::move(file_data.value())}
            }
        };
    }

    if (fs::is_regular_file(path) && path.extension() == ".zip")
    {
        auto archive_buffer = ReadFile(path);

        if (!archive_buffer.has_value())
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to load plugin sources from " << path;
            return std::nullopt;
        }

        return PluginSource{
            .entrypoint = "plugin.lua",
            .sources    = Zip::Load(*archive_buffer)
        };
    }

    if (fs::is_directory(path))
    {
        PluginSource source{
            .entrypoint = "plugin.lua",
            .sources    = {}
        };

        for (auto it = fs::recursive_directory_iterator(path); it != fs::recursive_directory_iterator(); ++it)
        {
            const fs::path relative = it->path().lexically_relative(path);

            if (it->is_directory() && IsSkippedDirectory(relative))
            {
                it.disable_recursion_pending();
                continue;
            }

            if (!it->is_regular_file() || !IsPluginFile(relative))
            {
                continue;
            }

            auto contents = ReadFile(it->path());

            if (!contents.has_value())
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to read file " << it->path();
                continue;
            }

            source.sources.insert_or_assign(relative, std::move(*contents));
        }

        return source;
    }

    return std::nullopt;
}
